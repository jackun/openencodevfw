#include "stdafx.h"
#include "bitstream.h"

//TODO make it in-place instead
int Parser::nal_decode(/* nal_t *nal,*/ void *p_data, int i_data )
{
    uint8 *src = (uint8*)p_data;
    uint8 *end = &src[i_data];
    //uint8 *dst = nal.p_payload;

    nal.i_type    = src[0]&0x1f;
    nal.i_ref_idc = (src[0] >> 5)&0x03;

    src++;

    while( src < end )
    {
        if( src < end - 3 && src[0] == 0x00 && src[1] == 0x00  && src[2] == 0x03 )
        {
            //*dst++ = 0x00;
            //*dst++ = 0x00;

            src += 3;
            continue;
        }
        //*dst++ = *src++;
		src++;
    }

    nal.i_payload = (int)(/*dst*/ src - (uint8*)p_data);
    return 0;
}

void Parser::init()
{
    h264.i_width = 0;
    h264.i_height = 0;
    h264.b_key = 0;
    h264.i_nal_type = -1;
    h264.i_ref_idc = -1;
    h264.i_idr_pic_id = -1;
    h264.i_frame_num = -1;
    h264.i_log2_max_frame_num = 0;
    h264.i_poc = -1;
    h264.i_poc_type = -1;
}

void Parser::parse(int *pb_nal_start)
{
	h264_t *h = &h264;
    bs_t s;
    *pb_nal_start = 0;

    if( nal.i_type == NAL_SPS || nal.i_type == NAL_PPS )
        *pb_nal_start = 1;

    bs_init( &s, nal.p_payload, nal.i_payload );
    if( nal.i_type == NAL_SPS )
    {
        int i_tmp;

        i_tmp = bs_read( &s, 8 );
        bs_skip( &s, 1+1+1 + 5 + 8 );
        /* sps id */
        bs_read_ue( &s );

        if( i_tmp >= 100 )
        {
            b_hiprofile = 1;
			bs_read_ue( &s ); // chroma_format_idc
            bs_read_ue( &s ); // bit_depth_luma_minus8
            bs_read_ue( &s ); // bit_depth_chroma_minus8
            bs_skip( &s, 1 ); // qpprime_y_zero_transform_bypass_flag
            if( bs_read( &s, 1 ) ) // seq_scaling_matrix_present_flag
            {
                int i, j;
                for( i = 0; i < 8; i++ )
                {
                    if( bs_read( &s, 1 ) ) // seq_scaling_list_present_flag[i]
                    {
                        uint8 i_tmp = 8;
                        for( j = 0; j < (i<6?16:64); j++ )
                        {
                            i_tmp += bs_read_se( &s );
                            if( i_tmp == 0 )
                                break;
                        }
                    }
                }
            }
        }

        /* Skip i_log2_max_frame_num */
        h->i_log2_max_frame_num = bs_read_ue( &s ) + 4;
        /* Read poc_type */
        h->i_poc_type = bs_read_ue( &s );
        if( h->i_poc_type == 0 )
        {
            h->i_log2_max_poc_lsb = bs_read_ue( &s ) + 4;
        }
        else if( h->i_poc_type == 1 )
        {
            int i_cycle;
            /* skip b_delta_pic_order_always_zero */
            bs_skip( &s, 1 );
            /* skip i_offset_for_non_ref_pic */
            bs_read_se( &s );
            /* skip i_offset_for_top_to_bottom_field */
            bs_read_se( &s );
            /* read i_num_ref_frames_in_poc_cycle */
            i_cycle = bs_read_ue( &s ); 
            if( i_cycle > 256 ) i_cycle = 256;
            while( i_cycle > 0 )
            {
                /* skip i_offset_for_ref_frame */
                bs_read_se(&s );
            }
        }
        /* i_num_ref_frames */
        bs_read_ue( &s );
        /* b_gaps_in_frame_num_value_allowed */
        bs_skip( &s, 1 );

        /* Read size */
        h->i_width  = 16 * ( bs_read_ue( &s ) + 1 );
        h->i_height = 32 * ( bs_read_ue( &s ) + 1 );

        /* b_frame_mbs_only */
        i_tmp = bs_read( &s, 1 );
        if( i_tmp == 1 )
        {
			// fix: progressive, half height
			h->i_height /= 2;
		}
		else
		{
			// mb_adaptive_frame_field_flag
            bs_skip( &s, 1 );
        }
        /* b_direct8x8_inference */
        bs_skip( &s, 1 );

        /* crop ? */
        i_tmp = bs_read( &s, 1 );
        if( i_tmp )
        {
			// fix: no cropping

            /* left */
            //h->i_width -= 2 * bs_read_ue( &s );
            /* right */
            //h->i_width -= 2 * bs_read_ue( &s );
            /* top */
            //h->i_height -= 2 * bs_read_ue( &s );
            /* bottom */
            //h->i_height -= 2 * bs_read_ue( &s );
        }

        /* vui: ignored */
    }
    else if( nal.i_type >= NAL_SLICE && nal.i_type <= NAL_SLICE_IDR )
    {
        int i_tmp;

        /* i_first_mb */
        bs_read_ue( &s );
        /* picture type */
        switch( bs_read_ue( &s ) )
        {
            case 0: case 5: /* P */
            case 1: case 6: /* B */
            case 3: case 8: /* SP */
                h->b_key = 0;
                break;
            case 2: case 7: /* I */
            case 4: case 9: /* SI */
				// fix: key flags in high profile
                h->b_key = (nal.i_type == NAL_SLICE_IDR || (b_hiprofile && nal.i_type == NAL_SLICE));
                break;
        }
        /* pps id */
        bs_read_ue( &s );

        /* frame num */
        i_tmp = bs_read( &s, h->i_log2_max_frame_num );

        if( i_tmp != h->i_frame_num )
            *pb_nal_start = 1;

        h->i_frame_num = i_tmp;

        if( nal.i_type == NAL_SLICE_IDR )
        {
            i_tmp = bs_read_ue( &s );
            if( h->i_nal_type == NAL_SLICE_IDR && h->i_idr_pic_id != i_tmp )
                *pb_nal_start = 1;

            h->i_idr_pic_id = i_tmp;
        }

        if( h->i_poc_type == 0 )
        {
            i_tmp = bs_read( &s, h->i_log2_max_poc_lsb );
            if( i_tmp != h->i_poc )
                *pb_nal_start = 1;
            h->i_poc = i_tmp;
        }
    }
    h->i_nal_type = nal.i_type;
    h->i_ref_idc = nal.i_ref_idc;
}