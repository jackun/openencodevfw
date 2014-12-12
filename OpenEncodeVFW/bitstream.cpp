#include "stdafx.h"
#include "bitstream.h"
#include "colorspace.h"

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

#define SAR_Extended      255        // Extended_SAR

/**
Network Abstraction Layer (NAL) unit
@see 7.3.1 NAL unit syntax
@see read_nal_unit
@see write_nal_unit
@see debug_nal
*/
//typedef struct
//{
//	int forbidden_zero_bit;
//	int nal_ref_idc;
//	int nal_unit_type;
//	void* parsed; // FIXME
//	int sizeof_parsed;
//
//	//uint8_t* rbsp_buf;
//	//int rbsp_size;
//} nal_t;

/**
Sequence Parameter Set
@see 7.3.2.1 Sequence parameter set RBSP syntax
@see read_seq_parameter_set_rbsp
@see write_seq_parameter_set_rbsp
@see debug_sps
*/
typedef struct
{
	int profile_idc;
	int constraint_set0_flag;
	int constraint_set1_flag;
	int constraint_set2_flag;
	int constraint_set3_flag;
	int constraint_set4_flag;
	int constraint_set5_flag;
	int reserved_zero_2bits;
	int level_idc;
	int seq_parameter_set_id;
	int chroma_format_idc;
	int residual_colour_transform_flag;
	int bit_depth_luma_minus8;
	int bit_depth_chroma_minus8;
	int qpprime_y_zero_transform_bypass_flag;
	int seq_scaling_matrix_present_flag;
	int seq_scaling_list_present_flag[8];
	int* ScalingList4x4[6];
	int UseDefaultScalingMatrix4x4Flag[6];
	int* ScalingList8x8[2];
	int UseDefaultScalingMatrix8x8Flag[2];
	int log2_max_frame_num_minus4;
	int pic_order_cnt_type;
	int log2_max_pic_order_cnt_lsb_minus4;
	int delta_pic_order_always_zero_flag;
	int offset_for_non_ref_pic;
	int offset_for_top_to_bottom_field;
	int num_ref_frames_in_pic_order_cnt_cycle;
	int offset_for_ref_frame[256];
	int num_ref_frames;
	int gaps_in_frame_num_value_allowed_flag;
	int pic_width_in_mbs_minus1;
	int pic_height_in_map_units_minus1;
	int frame_mbs_only_flag;
	int mb_adaptive_frame_field_flag;
	int direct_8x8_inference_flag;
	int frame_cropping_flag;
	int frame_crop_left_offset;
	int frame_crop_right_offset;
	int frame_crop_top_offset;
	int frame_crop_bottom_offset;
	int vui_parameters_present_flag;

	struct
	{
		int aspect_ratio_info_present_flag;
		int aspect_ratio_idc;
		int sar_width;
		int sar_height;
		int overscan_info_present_flag;
		int overscan_appropriate_flag;
		int video_signal_type_present_flag;
		int video_format;
		int video_full_range_flag;
		int colour_description_present_flag;
		int colour_primaries;
		int transfer_characteristics;
		int matrix_coefficients;
		int chroma_loc_info_present_flag;
		int chroma_sample_loc_type_top_field;
		int chroma_sample_loc_type_bottom_field;
		int timing_info_present_flag;
		int num_units_in_tick;
		int time_scale;
		int fixed_frame_rate_flag;
		int nal_hrd_parameters_present_flag;
		int vcl_hrd_parameters_present_flag;
		int low_delay_hrd_flag;
		int pic_struct_present_flag;
		int bitstream_restriction_flag;
		int motion_vectors_over_pic_boundaries_flag;
		int max_bytes_per_pic_denom;
		int max_bits_per_mb_denom;
		int log2_max_mv_length_horizontal;
		int log2_max_mv_length_vertical;
		int num_reorder_frames;
		int max_dec_frame_buffering;
	} vui;

	struct
	{
		int cpb_cnt_minus1;
		int bit_rate_scale;
		int cpb_size_scale;
		int bit_rate_value_minus1[32]; // up to cpb_cnt_minus1, which is <= 31
		int cpb_size_value_minus1[32];
		int cbr_flag[32];
		int initial_cpb_removal_delay_length_minus1;
		int cpb_removal_delay_length_minus1;
		int dpb_output_delay_length_minus1;
		int time_offset_length;
	} hrd;

} sps_t;

//7.3.2.1.1 Scaling list syntax
void read_scaling_list(bs_t* b, int* scalingList, int sizeOfScalingList, int useDefaultScalingMatrixFlag)
{
	int j;
	if (scalingList == NULL)
	{
		return;
	}

	int lastScale = 8;
	int nextScale = 8;
	for (j = 0; j < sizeOfScalingList; j++)
	{
		if (nextScale != 0)
		{
			int delta_scale = bs_read_se(b);
			nextScale = (lastScale + delta_scale + 256) % 256;
			useDefaultScalingMatrixFlag = (j == 0 && nextScale == 0);
		}
		scalingList[j] = (nextScale == 0) ? lastScale : nextScale;
		lastScale = scalingList[j];
	}
}

//7.3.2.1.1 Scaling list syntax
void write_scaling_list(bs_t* b, int* scalingList, int sizeOfScalingList, int useDefaultScalingMatrixFlag)
{
	int j;

	int lastScale = 8;
	int nextScale = 8;

	for (j = 0; j < sizeOfScalingList; j++)
	{
		int delta_scale;

		if (nextScale != 0)
		{
			// FIXME will not write in most compact way - could truncate list if all remaining elements are equal
			nextScale = scalingList[j];

			if (useDefaultScalingMatrixFlag)
			{
				nextScale = 0;
			}

			delta_scale = (nextScale - lastScale) % 256;
			bs_write_se(b, delta_scale);
		}

		lastScale = scalingList[j];
	}
}

//7.3.2.11 RBSP trailing bits syntax
void write_rbsp_trailing_bits(bs_t* b)
{
	int rbsp_stop_one_bit = 1;
	int rbsp_alignment_zero_bit = 0;

	bs_write(b, 1, rbsp_stop_one_bit); // equal to 1
	while (!bs_byte_aligned(b))
	{
		bs_write(b, 1, rbsp_alignment_zero_bit); // equal to 0
	}
}

//Appendix E.1.2 HRD parameters syntax
void
write_hrd_parameters(sps_t *sps, bs_t* b)
{
	int SchedSelIdx;

	bs_write_ue(b, sps->hrd.cpb_cnt_minus1);
	bs_write(b, 4, sps->hrd.bit_rate_scale);
	bs_write(b, 4, sps->hrd.cpb_size_scale);
	for (SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++)
	{
		bs_write_ue(b, sps->hrd.bit_rate_value_minus1[SchedSelIdx]);
		bs_write_ue(b, sps->hrd.cpb_size_value_minus1[SchedSelIdx]);
		bs_write(b, 1, sps->hrd.cbr_flag[SchedSelIdx]);
	}
	bs_write(b, 5, sps->hrd.initial_cpb_removal_delay_length_minus1);
	bs_write(b, 5, sps->hrd.cpb_removal_delay_length_minus1);
	bs_write(b, 5, sps->hrd.dpb_output_delay_length_minus1);
	bs_write(b, 5, sps->hrd.time_offset_length);
}

//Appendix E.1.1 VUI parameters syntax
void write_vui_parameters(sps_t *sps, bs_t* b)
{
	bs_write(b, 1, sps->vui.aspect_ratio_info_present_flag);
	if (sps->vui.aspect_ratio_info_present_flag)
	{
		bs_write(b, 8, sps->vui.aspect_ratio_idc);
		if (sps->vui.aspect_ratio_idc == SAR_Extended)
		{
			bs_write(b, 16, sps->vui.sar_width);
			bs_write(b, 16, sps->vui.sar_height);
		}
	}
	bs_write(b, 1, sps->vui.overscan_info_present_flag);
	if (sps->vui.overscan_info_present_flag)
	{
		bs_write(b, 1, sps->vui.overscan_appropriate_flag);
	}
	bs_write(b, 1, sps->vui.video_signal_type_present_flag);
	if (sps->vui.video_signal_type_present_flag)
	{
		bs_write(b, 3, sps->vui.video_format);
		bs_write(b, 1, sps->vui.video_full_range_flag);
		bs_write(b, 1, sps->vui.colour_description_present_flag);
		if (sps->vui.colour_description_present_flag)
		{
			bs_write(b, 8, sps->vui.colour_primaries);
			bs_write(b, 8, sps->vui.transfer_characteristics);
			bs_write(b, 8, sps->vui.matrix_coefficients);
		}
	}
	bs_write(b, 1, sps->vui.chroma_loc_info_present_flag);
	if (sps->vui.chroma_loc_info_present_flag)
	{
		bs_write_ue(b, sps->vui.chroma_sample_loc_type_top_field);
		bs_write_ue(b, sps->vui.chroma_sample_loc_type_bottom_field);
	}
	bs_write(b, 1, sps->vui.timing_info_present_flag);
	if (sps->vui.timing_info_present_flag)
	{
		bs_write(b, 32, sps->vui.num_units_in_tick);
		bs_write(b, 32, sps->vui.time_scale);
		bs_write(b, 1, sps->vui.fixed_frame_rate_flag);
	}
	bs_write(b, 1, sps->vui.nal_hrd_parameters_present_flag);
	if (sps->vui.nal_hrd_parameters_present_flag)
	{
		write_hrd_parameters(sps, b);
	}
	bs_write(b, 1, sps->vui.vcl_hrd_parameters_present_flag);
	if (sps->vui.vcl_hrd_parameters_present_flag)
	{
		write_hrd_parameters(sps, b);
	}
	if (sps->vui.nal_hrd_parameters_present_flag || sps->vui.vcl_hrd_parameters_present_flag)
	{
		bs_write(b, 1, sps->vui.low_delay_hrd_flag);
	}
	bs_write(b, 1, sps->vui.pic_struct_present_flag);
	bs_write(b, 1, sps->vui.bitstream_restriction_flag);
	if (sps->vui.bitstream_restriction_flag)
	{
		bs_write(b, 1, sps->vui.motion_vectors_over_pic_boundaries_flag);
		bs_write_ue(b, sps->vui.max_bytes_per_pic_denom);
		bs_write_ue(b, sps->vui.max_bits_per_mb_denom);
		bs_write_ue(b, sps->vui.log2_max_mv_length_horizontal);
		bs_write_ue(b, sps->vui.log2_max_mv_length_vertical);
		bs_write_ue(b, sps->vui.num_reorder_frames);
		bs_write_ue(b, sps->vui.max_dec_frame_buffering);
	}
}

//7.3.2.1 Sequence parameter set RBSP syntax
void write_seq_parameter_set_rbsp(sps_t *sps, bs_t* b)
{
	int i;

	bs_write(b, 8, sps->profile_idc);
	bs_write(b, 1, sps->constraint_set0_flag);
	bs_write(b, 1, sps->constraint_set1_flag);
	bs_write(b, 1, sps->constraint_set2_flag);
	bs_write(b, 1, sps->constraint_set3_flag);
	bs_write(b, 1, sps->constraint_set4_flag);
	bs_write(b, 1, sps->constraint_set5_flag);
	bs_write(b, 2, 0);  /* reserved_zero_2bits */
	bs_write(b, 8, sps->level_idc);
	bs_write_ue(b, sps->seq_parameter_set_id);
	if (sps->profile_idc == 100 || sps->profile_idc == 110 ||
		sps->profile_idc == 122 || sps->profile_idc == 144)
	{
		bs_write_ue(b, sps->chroma_format_idc);
		if (sps->chroma_format_idc == 3)
		{
			bs_write(b, 1, sps->residual_colour_transform_flag);
		}
		bs_write_ue(b, sps->bit_depth_luma_minus8);
		bs_write_ue(b, sps->bit_depth_chroma_minus8);
		bs_write(b, 1, sps->qpprime_y_zero_transform_bypass_flag);
		bs_write(b, 1, sps->seq_scaling_matrix_present_flag);
		if (sps->seq_scaling_matrix_present_flag)
		{
			for (i = 0; i < 8; i++)
			{
				bs_write(b, 1, sps->seq_scaling_list_present_flag[i]);
				if (sps->seq_scaling_list_present_flag[i])
				{
					if (i < 6)
					{
						write_scaling_list(b, sps->ScalingList4x4[i], 16,
							sps->UseDefaultScalingMatrix4x4Flag[i]);
					}
					else
					{
						write_scaling_list(b, sps->ScalingList8x8[i - 6], 64,
							sps->UseDefaultScalingMatrix8x8Flag[i - 6]);
					}
				}
			}
		}
	}
	bs_write_ue(b, sps->log2_max_frame_num_minus4);
	bs_write_ue(b, sps->pic_order_cnt_type);
	if (sps->pic_order_cnt_type == 0)
	{
		bs_write_ue(b, sps->log2_max_pic_order_cnt_lsb_minus4);
	}
	else if (sps->pic_order_cnt_type == 1)
	{
		bs_write(b, 1, sps->delta_pic_order_always_zero_flag);
		bs_write_se(b, sps->offset_for_non_ref_pic);
		bs_write_se(b, sps->offset_for_top_to_bottom_field);
		bs_write_ue(b, sps->num_ref_frames_in_pic_order_cnt_cycle);
		for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
		{
			bs_write_se(b, sps->offset_for_ref_frame[i]);
		}
	}
	bs_write_ue(b, sps->num_ref_frames);
	bs_write(b, 1, sps->gaps_in_frame_num_value_allowed_flag);
	bs_write_ue(b, sps->pic_width_in_mbs_minus1);
	bs_write_ue(b, sps->pic_height_in_map_units_minus1);
	bs_write(b, 1, sps->frame_mbs_only_flag);
	if (!sps->frame_mbs_only_flag)
	{
		bs_write(b, 1, sps->mb_adaptive_frame_field_flag);
	}
	bs_write(b, 1, sps->direct_8x8_inference_flag);
	bs_write(b, 1, sps->frame_cropping_flag);
	if (sps->frame_cropping_flag)
	{
		bs_write_ue(b, sps->frame_crop_left_offset);
		bs_write_ue(b, sps->frame_crop_right_offset);
		bs_write_ue(b, sps->frame_crop_top_offset);
		bs_write_ue(b, sps->frame_crop_bottom_offset);
	}
	bs_write(b, 1, sps->vui_parameters_present_flag);
	if (sps->vui_parameters_present_flag)
	{
		write_vui_parameters(sps, b);
	}
	write_rbsp_trailing_bits(b);
}

//Appendix E.1.2 HRD parameters syntax
void read_hrd_parameters(sps_t *sps, bs_t* b)
{
	int SchedSelIdx;

	sps->hrd.cpb_cnt_minus1 = bs_read_ue(b);
	sps->hrd.bit_rate_scale = bs_read(b, 4);
	sps->hrd.cpb_size_scale = bs_read(b, 4);
	for (SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++)
	{
		sps->hrd.bit_rate_value_minus1[SchedSelIdx] = bs_read_ue(b);
		sps->hrd.cpb_size_value_minus1[SchedSelIdx] = bs_read_ue(b);
		sps->hrd.cbr_flag[SchedSelIdx] = bs_read(b, 1);
	}
	sps->hrd.initial_cpb_removal_delay_length_minus1 = bs_read(b, 5);
	sps->hrd.cpb_removal_delay_length_minus1 = bs_read(b, 5);
	sps->hrd.dpb_output_delay_length_minus1 = bs_read(b, 5);
	sps->hrd.time_offset_length = bs_read(b, 5);
}

//Appendix E.1.1 VUI parameters syntax
void read_vui_parameters(sps_t *sps, bs_t* b)
{
	sps->vui.aspect_ratio_info_present_flag = bs_read(b, 1);
	if (sps->vui.aspect_ratio_info_present_flag)
	{
		sps->vui.aspect_ratio_idc = bs_read(b, 8);
		if (sps->vui.aspect_ratio_idc == SAR_Extended)
		{
			sps->vui.sar_width = bs_read(b, 16);
			sps->vui.sar_height = bs_read(b, 16);
		}
	}
	sps->vui.overscan_info_present_flag = bs_read(b, 1);
	if (sps->vui.overscan_info_present_flag)
	{
		sps->vui.overscan_appropriate_flag = bs_read(b, 1);
	}
	sps->vui.video_signal_type_present_flag = bs_read(b, 1);
	if (sps->vui.video_signal_type_present_flag)
	{
		sps->vui.video_format = bs_read(b, 3);
		sps->vui.video_full_range_flag = bs_read(b, 1);
		sps->vui.colour_description_present_flag = bs_read(b, 1);
		if (sps->vui.colour_description_present_flag)
		{
			sps->vui.colour_primaries = bs_read(b, 8);
			sps->vui.transfer_characteristics = bs_read(b, 8);
			sps->vui.matrix_coefficients = bs_read(b, 8);
		}
	}
	sps->vui.chroma_loc_info_present_flag = bs_read(b, 1);
	if (sps->vui.chroma_loc_info_present_flag)
	{
		sps->vui.chroma_sample_loc_type_top_field = bs_read_ue(b);
		sps->vui.chroma_sample_loc_type_bottom_field = bs_read_ue(b);
	}
	sps->vui.timing_info_present_flag = bs_read(b, 1);
	if (sps->vui.timing_info_present_flag)
	{
		sps->vui.num_units_in_tick = bs_read(b, 32);
		sps->vui.time_scale = bs_read(b, 32);
		sps->vui.fixed_frame_rate_flag = bs_read(b, 1);
	}
	sps->vui.nal_hrd_parameters_present_flag = bs_read(b, 1);
	if (sps->vui.nal_hrd_parameters_present_flag)
	{
		read_hrd_parameters(sps, b);
	}
	sps->vui.vcl_hrd_parameters_present_flag = bs_read(b, 1);
	if (sps->vui.vcl_hrd_parameters_present_flag)
	{
		read_hrd_parameters(sps, b);
	}
	if (sps->vui.nal_hrd_parameters_present_flag || sps->vui.vcl_hrd_parameters_present_flag)
	{
		sps->vui.low_delay_hrd_flag = bs_read(b, 1);
	}
	sps->vui.pic_struct_present_flag = bs_read(b, 1);
	sps->vui.bitstream_restriction_flag = bs_read(b, 1);
	if (sps->vui.bitstream_restriction_flag)
	{
		sps->vui.motion_vectors_over_pic_boundaries_flag = bs_read(b, 1);
		sps->vui.max_bytes_per_pic_denom = bs_read_ue(b);
		sps->vui.max_bits_per_mb_denom = bs_read_ue(b);
		sps->vui.log2_max_mv_length_horizontal = bs_read_ue(b);
		sps->vui.log2_max_mv_length_vertical = bs_read_ue(b);
		sps->vui.num_reorder_frames = bs_read_ue(b);
		sps->vui.max_dec_frame_buffering = bs_read_ue(b);
	}
}

//7.3.2.1 Sequence parameter set RBSP syntax
void read_seq_parameter_set_rbsp(sps_t *sps, bs_t* b)
{
	int i;

	// NOTE can't read directly into sps because seq_parameter_set_id not yet known and so sps is not selected

	int profile_idc = bs_read(b, 8);
	int constraint_set0_flag = bs_read(b, 1);
	int constraint_set1_flag = bs_read(b, 1);
	int constraint_set2_flag = bs_read(b, 1);
	int constraint_set3_flag = bs_read(b, 1);
	int constraint_set4_flag = bs_read(b, 1);
	int constraint_set5_flag = bs_read(b, 1);
	int reserved_zero_2bits = bs_read(b, 2);  /* all 0's */
	int level_idc = bs_read(b, 8);
	int seq_parameter_set_id = bs_read_ue(b);

	memset(sps, 0, sizeof(sps_t));

	sps->chroma_format_idc = 1;

	sps->profile_idc = profile_idc; // bs_read(b, 8);
	sps->constraint_set0_flag = constraint_set0_flag;//bs_read(b, 1);
	sps->constraint_set1_flag = constraint_set1_flag;//bs_read(b, 1);
	sps->constraint_set2_flag = constraint_set2_flag;//bs_read(b, 1);
	sps->constraint_set3_flag = constraint_set3_flag;//bs_read(b, 1);
	sps->constraint_set4_flag = constraint_set4_flag;//bs_read(b, 1);
	sps->constraint_set5_flag = constraint_set5_flag;//bs_read(b, 1);
	sps->reserved_zero_2bits = reserved_zero_2bits;//bs_read(b,2);
	sps->level_idc = level_idc; //bs_read(b, 8);
	sps->seq_parameter_set_id = seq_parameter_set_id; // bs_read_ue(b);
	if (sps->profile_idc == 100 || sps->profile_idc == 110 ||
		sps->profile_idc == 122 || sps->profile_idc == 144)
	{
		sps->chroma_format_idc = bs_read_ue(b);
		if (sps->chroma_format_idc == 3)
		{
			sps->residual_colour_transform_flag = bs_read(b, 1);
		}
		sps->bit_depth_luma_minus8 = bs_read_ue(b);
		sps->bit_depth_chroma_minus8 = bs_read_ue(b);
		sps->qpprime_y_zero_transform_bypass_flag = bs_read(b, 1);
		sps->seq_scaling_matrix_present_flag = bs_read(b, 1);
		if (sps->seq_scaling_matrix_present_flag)
		{
			for (i = 0; i < 8; i++)
			{
				sps->seq_scaling_list_present_flag[i] = bs_read(b, 1);
				if (sps->seq_scaling_list_present_flag[i])
				{
					if (i < 6)
					{
						read_scaling_list(b, sps->ScalingList4x4[i], 16,
							sps->UseDefaultScalingMatrix4x4Flag[i]);
					}
					else
					{
						read_scaling_list(b, sps->ScalingList8x8[i - 6], 64,
							sps->UseDefaultScalingMatrix8x8Flag[i - 6]);
					}
				}
			}
		}
	}
	sps->log2_max_frame_num_minus4 = bs_read_ue(b);
	sps->pic_order_cnt_type = bs_read_ue(b);
	if (sps->pic_order_cnt_type == 0)
	{
		sps->log2_max_pic_order_cnt_lsb_minus4 = bs_read_ue(b);
	}
	else if (sps->pic_order_cnt_type == 1)
	{
		sps->delta_pic_order_always_zero_flag = bs_read(b, 1);
		sps->offset_for_non_ref_pic = bs_read_se(b);
		sps->offset_for_top_to_bottom_field = bs_read_se(b);
		sps->num_ref_frames_in_pic_order_cnt_cycle = bs_read_ue(b);
		for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
		{
			sps->offset_for_ref_frame[i] = bs_read_se(b);
		}
	}
	sps->num_ref_frames = bs_read_ue(b);
	sps->gaps_in_frame_num_value_allowed_flag = bs_read(b, 1);
	sps->pic_width_in_mbs_minus1 = bs_read_ue(b);
	sps->pic_height_in_map_units_minus1 = bs_read_ue(b);
	sps->frame_mbs_only_flag = bs_read(b, 1);
	if (!sps->frame_mbs_only_flag)
	{
		sps->mb_adaptive_frame_field_flag = bs_read(b, 1);
	}
	sps->direct_8x8_inference_flag = bs_read(b, 1);
	sps->frame_cropping_flag = bs_read(b, 1);
	if (sps->frame_cropping_flag)
	{
		sps->frame_crop_left_offset = bs_read_ue(b);
		sps->frame_crop_right_offset = bs_read_ue(b);
		sps->frame_crop_top_offset = bs_read_ue(b);
		sps->frame_crop_bottom_offset = bs_read_ue(b);
	}
	sps->vui_parameters_present_flag = bs_read(b, 1);
	if (sps->vui_parameters_present_flag)
	{
		read_vui_parameters(sps, b);
	}
	//read_rbsp_trailing_bits(h, b);
}

int add_vui(void *srcPtr, size_t srcSize, void *dstPtr, size_t dstSize, int color)
{
	sps_t sps;
	bs_t b;
	memset(&b, 0, sizeof(bs_t));

	bs_init(&b, srcPtr, srcSize);
	read_seq_parameter_set_rbsp(&sps, &b);

	sps.vui_parameters_present_flag = 1;

	{
		sps.vui.video_signal_type_present_flag = 1;
		{
			sps.vui.video_format = 5;
			sps.vui.video_full_range_flag = 1;
		}
		sps.vui.colour_description_present_flag = 1;
		{
			sps.vui.colour_primaries = 1;
		}
		sps.vui.transfer_characteristics = 1;
		sps.vui.matrix_coefficients = 1;
	}

	switch (color)
	{
	case BT601_LIMITED:
		sps.vui.colour_primaries = 5;
		sps.vui.video_full_range_flag = 0;
		sps.vui.transfer_characteristics = 6;
		sps.vui.matrix_coefficients = 5;
		break;
	case BT601_FULL:
		sps.vui.colour_primaries = 5;
		sps.vui.transfer_characteristics = 6;
		sps.vui.matrix_coefficients = 5;
		break;
	case BT601_FULL_YCbCr:
		sps.vui.colour_primaries = 5;
		sps.vui.transfer_characteristics = 8;
		sps.vui.matrix_coefficients = 5;
		break;
	case BT709_LIMITED:
		sps.vui.video_full_range_flag = 0;
		break;
	case BT709_FULL:
		break;
	case BT709_ALT1_LIMITED:
		sps.vui.video_full_range_flag = 0;
		break;
	default:
		sps.vui_parameters_present_flag = 0;
		break;
	}

	bs_init(&b, dstPtr, dstSize);
	write_seq_parameter_set_rbsp(&sps, &b);
	return bs_pos_byte(&b);
}