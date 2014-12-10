#include "OvEncodeTypedef.h"

//NAL ref idc codes
#define NAL_REF_IDC_PRIORITY_HIGHEST 3
#define NAL_REF_IDC_PRIORITY_HIGH 2
#define NAL_REF_IDC_PRIORITY_LOW 1
#define NAL_REF_IDC_PRIORITY_DISPOSABLE 0
//Table 7-1 NAL unit type codes
#define NAL_UNIT_TYPE_UNSPECIFIED 0 // Unspecified
#define NAL_UNIT_TYPE_CODED_SLICE_NON_IDR 1 // Coded slice of a non-IDR picture
#define NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A 2 // Coded slice data partition A
#define NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B 3 // Coded slice data partition B
#define NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C 4 // Coded slice data partition C
#define NAL_UNIT_TYPE_CODED_SLICE_IDR 5 // Coded slice of an IDR picture
#define NAL_UNIT_TYPE_SEI 6 // Supplemental enhancement information (SEI)
#define NAL_UNIT_TYPE_SPS 7 // Sequence parameter set
#define NAL_UNIT_TYPE_PPS 8 // Picture parameter set
#define NAL_UNIT_TYPE_AUD 9 // Access unit delimiter
#define NAL_UNIT_TYPE_END_OF_SEQUENCE 10 // End of sequence
#define NAL_UNIT_TYPE_END_OF_STREAM 11 // End of stream
#define NAL_UNIT_TYPE_FILLER 12 // Filler data
#define NAL_UNIT_TYPE_SPS_EXT 13 // Sequence parameter set extension
// 14..18 // Reserved
#define NAL_UNIT_TYPE_CODED_SLICE_AUX 19 // Coded slice of an auxiliary coded picture without partitioning
// 20..23 // Reserved
// 24..31 // Unspecified 

//7.4.3 Table 7-6. Name association to slice_type
#define SH_SLICE_TYPE_P 0 // P (P slice)
#define SH_SLICE_TYPE_B 1 // B (B slice)
#define SH_SLICE_TYPE_I 2 // I (I slice)
#define SH_SLICE_TYPE_SP 3 // SP (SP slice)
#define SH_SLICE_TYPE_SI 4 // SI (SI slice)
//as per footnote to Table 7-6, the *_ONLY slice types indicate that all other slices in that picture are of the same type
#define SH_SLICE_TYPE_P_ONLY 5 // P (P slice)
#define SH_SLICE_TYPE_B_ONLY 6 // B (B slice)
#define SH_SLICE_TYPE_I_ONLY 7 // I (I slice)
#define SH_SLICE_TYPE_SP_ONLY 8 // SP (SP slice)
#define SH_SLICE_TYPE_SI_ONLY 9 // SI (SI slice)


//Stolen from avc2avi
#define MAX_DATA 3000000

enum nal_unit_type_e
{
    NAL_UNKNOWN = 0,
    NAL_SLICE   = 1,
    NAL_SLICE_DPA   = 2,
    NAL_SLICE_DPB   = 3,
    NAL_SLICE_DPC   = 4,
    NAL_SLICE_IDR   = 5,    /* ref_idc != 0 */
    NAL_SEI         = 6,    /* ref_idc == 0 */
    NAL_SPS         = 7,
    NAL_PPS         = 8
    /* ref_idc == 0 for 6,9,10,11,12 */
};
enum nal_priority_e
{
    NAL_PRIORITY_DISPOSABLE = 0,
    NAL_PRIORITY_LOW        = 1,
    NAL_PRIORITY_HIGH       = 2,
    NAL_PRIORITY_HIGHEST    = 3,
};

typedef struct
{
    int i_ref_idc;  /* nal_priority_e */
    int i_type;     /* nal_unit_type_e */

    /* This data are raw payload */
    int     i_payload;
    uint8 *p_payload;
} nal_t;

typedef struct
{
    int i_width;
    int i_height;

    int i_nal_type;
    int i_ref_idc;
    int i_idr_pic_id;
    int i_frame_num;
    int i_poc;

    int b_key;
    int i_log2_max_frame_num;
    int i_poc_type;
    int i_log2_max_poc_lsb;
} h264_t;

class Parser
{
public:

    h264_t  h264;

    nal_t nal;
    int i_frame;
    int i_data;
    int b_eof;
    int b_key;
    int b_slice;
	int b_hiprofile;

	Parser(): i_frame(0), i_data(0), 
		b_eof(0), b_key(0), b_slice(0), b_hiprofile(0) 
	{
		nal.p_payload = NULL;//(uint8*) malloc(MAX_DATA);
		init();
	}

	~Parser()
	{
		//free(nal.p_payload);
		nal.p_payload = 0;
	}

	void init();
	void parse( /*h264_t *h, nal_t *n, */ int *pb_nal_start );

	int nal_decode( /*nal_t *nal,*/ void *p_data, int i_data );
};

int add_vui(void *srcPtr, size_t srcSize, void *dstPtr, size_t dstSize, int color);