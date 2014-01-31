#include "stdafx.h"
#include "OpenEncodeVFW.h"

using namespace std;

CRITICAL_SECTION ove_CS;
HMODULE hmoduleVFW = 0;

CodecInst::CodecInst() : isVistaOrNewer(false), 
	mCLConvert(0), mRaw(0), mUseCLConv(true), mUseCPU(false), mDialogUpdated(false), buffer2(0),
	fps_den(0), fps_num(0), frame_total(0), mParser(0), mLog(0), prev(0), buffer(0), in(0), out(0),
	clDeviceID(0), mCpuCtx(0), mCpuDev(0) {
/*#ifndef OPENENCODE_RELEASE
	if ( started == 0x1337){
		char msg[128];
		sprintf_s(msg,128,"Attempting to instantiate a codec instance that has not been destroyed");
		MessageBox (HWND_DESKTOP, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
	}
#endif*/
	started=0;
	
	mDevList = getDeviceList();
	prepareConfigMap();
	readRegistry();

	mParser = new Parser();
	mLog = new Logger(mConfigTable[L"Log"] == 1);
	Log(L"Init\n");

	memset(&mDeviceHandle, 0, sizeof(OVDeviceHandle));
	memset(&mEncodeHandle, 0, sizeof(OVEncodeHandle));
	initProfileCnt(&mProfile);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD r, LPVOID) {
	hmoduleVFW = (HMODULE) hinstDLL;

	switch (r)
    {
        case DLL_PROCESS_ATTACH:
            InitializeCriticalSection(&ove_CS);
            break;

        case DLL_PROCESS_DETACH:
            DeleteCriticalSection(&ove_CS);
            break;
    }
	return TRUE;
}

CodecInst* Open(ICOPEN* icinfo) {
	if (icinfo && icinfo->fccType != ICTYPE_VIDEO)
		return NULL;

	CodecInst* pinst = new CodecInst();
	//pinst->Log(L"Open\n");
	/**************************************************************************/
    /* Find the version of Windows                                            */
    /**************************************************************************/
    OSVERSIONINFO vInfo;
    memset(&vInfo, 0, sizeof(vInfo));
    vInfo.dwOSVersionInfoSize = sizeof(vInfo);
    if(!GetVersionEx(&vInfo))
    {
        pinst->Log(L"Error : Unable to get Windows version information\n");
		if (icinfo) icinfo->dwError = ICERR_INTERNAL;
        return pinst; //FIXME Can be NULL ?
    }

    /**************************************************************************/
    /* Tell the user that this only runs on Win7 or Vista                     */
    /**************************************************************************/
    if(vInfo.dwMajorVersion >= 6)
        pinst->isVistaOrNewer = true;

    if(!pinst->isVistaOrNewer)
    {
        pinst->Log(L"Error : Unsupported OS! Vista or newer required.\n");
		if (icinfo) icinfo->dwError = ICERR_INTERNAL;
        return pinst;
    }

	if (icinfo) icinfo->dwError = pinst ? ICERR_OK : ICERR_MEMORY;

	if (icinfo) pinst->Log(L"Error: %d\n", icinfo->dwError);
	return pinst;
}

CodecInst::~CodecInst(){
	try {
		if ( started == 0x1337 ){
			CompressEnd();
		}
		started = 0;
		if(mParser) { delete mParser; mParser = NULL; }
		if(mLog) { delete mLog; mLog = NULL; }

		//Something weird goes on with Dxtory
		memset(&mDeviceHandle, 0, sizeof(OVDeviceHandle));
		memset(&mEncodeHandle, 0, sizeof(OVEncodeHandle));
	} catch ( ... ) {};
}

//Something weird goes on with Dxtory
DWORD Close(CodecInst* pinst) {
	try {
		if ( pinst && !IsBadWritePtr(pinst,sizeof(CodecInst)) ){
			delete pinst;
		}
	} catch ( ... ){};
    return 1;
}

// some programs assume that the codec is not configurable if GetState
// and SetState are not supported.
DWORD CodecInst::GetState(LPVOID pv, DWORD dwSize){
	Log(L"GetState\n");
	if ( pv == NULL ){
		return 1;
	} else if ( dwSize < 1 ){
		return ICERR_BADSIZE;
	}
	memset(pv,0,1);
	return 1;
}

// See GetState comment
DWORD CodecInst::SetState(LPVOID pv, DWORD dwSize) {
	if ( pv ){
		return ICERR_OK;
	} else {
		return 1;
	}
}

// return information about the codec
DWORD CodecInst::GetInfo(ICINFO* icinfo, DWORD dwSize) {
	if (icinfo == NULL)
		return sizeof(ICINFO);

	if (dwSize < sizeof(ICINFO))
		return 0;

	icinfo->dwSize          = sizeof(ICINFO);
	icinfo->fccType         = ICTYPE_VIDEO;
	icinfo->fccHandler		= FOURCC_OPEN;
	icinfo->dwFlags			= VIDCF_FASTTEMPORALC | VIDCF_FASTTEMPORALD;
	icinfo->dwVersion		= 0x00010000;
	icinfo->dwVersionICM	= ICVERSION;
	memcpy(icinfo->szName,L"OpenEncodeVFW",sizeof(L"OpenEncodeVFW"));
	//Well, no decoder part (yet)
	memcpy(icinfo->szDescription,L"OpenEncodeVFW Codec using AMD APP/VCE",sizeof(L"OpenEncodeVFW Codec using AMD APP/VCE"));

	return sizeof(ICINFO);
}

void CodecInst::Log(const wchar_t *psz_fmt, ...)
{
	if(mLog)
	{
		va_list arg;
		va_start(arg, psz_fmt);
		mLog->Log_internal(psz_fmt, arg);
		va_end(arg);
	}
}

/** 
 *******************************************************************************
 *  @fn     yuvToNV12
 *  @brief   Read yuv video file and converts it to NV12 format
 *           
 *  @param[in] fr  : pointer to the input picture data
 *  @param[in] uiHeight : video frame height
 *  @param[in] uiWidth  : video frame width
 *  @param[in] alignedSurfaceWidth  : aligned frame width
 *  @param[out] *pBitstreamData : input surface buffer pointer
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
//Some hypothetical 12 bit YUV format
bool CodecInst::yuvToNV12(const uint8 *inData, uint32 uiHeight, uint32 uiWidth, 
               uint32 alignedSurfaceWidth, int8 *pBitstreamData)
{
    bool results=false;
	const uint8 *fr = inData;
    const uint8  * pUplane = NULL;
    const uint8  * pVplane = NULL;
    uint8  * pUVrow = NULL;
    uint32     uiUVSize = (uiWidth*uiHeight)>>2;
    uint32     uiHalfHeight = uiHeight>>1;
    uint32     uiHalfWidth  = uiWidth>>1;
	
    if (fr)
    {
        /**********************************************************************/
        /* Y plane                                                            */
        /**********************************************************************/
        uint8* pBuf = (uint8 *) pBitstreamData;

        for (uint32 h=0; h<uiHeight; h++)
        {
			memcpy(pBuf, fr, sizeof(uint8) * uiWidth);
			fr += uiWidth;
            pBuf += alignedSurfaceWidth;
        }

        /**********************************************************************/
        /* Align the Y plane before adding the U & V data.                    */
        /* pBuf = (uint8 *)pBitstreamData + alignedSurfaceWidth *            */
        /* alignedSurfaceHeight;                                              */
        /**********************************************************************/
        /**********************************************************************/
        /* UV plane                                                           */
        /**********************************************************************/
        uint32 chromaWidth = uiHalfWidth;
		pUplane = fr;
		pVplane = fr + chromaWidth*uiHalfHeight;

        for (uint32 h=0; h<uiHalfHeight; h++)
        {
			pUVrow = pBuf;
            for (uint32 i = 0; i < chromaWidth; ++i)
            {
                pUVrow[i*2]     = pUplane[chromaWidth * h + i];
                pUVrow[i*2 + 1] = pVplane[chromaWidth * h + i];
            }
            pBuf += alignedSurfaceWidth;
        }

        results = true;
    }
	
	return results;
}

bool CodecInst::yv12ToNV12(const uint8 *inData, uint32 uiHeight, uint32 uiWidth, 
               uint32 alignedSurfaceWidth, int8 *pBitstreamData)
{
    bool results=false;
	const uint8 *fr = inData;
    const uint8  * pUplane = NULL;
    const uint8  * pVplane = NULL;
    uint8  * pUVrow = NULL;
    uint32     uiUVSize = (uiWidth*uiHeight)>>2;
    uint32     uiHalfHeight = uiHeight>>1;
    uint32     uiHalfWidth  = uiWidth>>1;
	
    if (fr)
    {
        /**********************************************************************/
        /* Y plane                                                            */
        /**********************************************************************/
        uint8* pBuf = (uint8 *) pBitstreamData;

        for (uint32 h=0; h<uiHeight; h++)
        {
			memcpy(pBuf, fr, sizeof(uint8) * uiWidth);
			fr += uiWidth;
            pBuf += alignedSurfaceWidth;
        }

        /**********************************************************************/
        /* UV plane                                                           */
        /**********************************************************************/
        uint32 chromaWidth = uiHalfWidth;
		pUplane = fr;
		pVplane = fr + chromaWidth*uiHalfHeight;

        for (uint32 h=0; h<uiHalfHeight; h++)
        {
			pUVrow = pBuf;
            for (uint32 i = 0; i < chromaWidth; ++i)
            {
                pUVrow[i*2]     = pVplane[chromaWidth * h + i]; //V comes first
                pUVrow[i*2 + 1] = pUplane[chromaWidth * h + i];
            }
            pBuf += alignedSurfaceWidth;
        }
        results = true;
    }
	
	return results;
}

bool CodecInst::nv12ToNV12Aligned(const uint8 *inData, uint32 uiHeight, uint32 uiWidth, 
               uint32 alignedSurfaceWidth, int8 *pBitstreamData)
{
    bool results=false;
	const uint8 *fr = inData;
    uint32     uiHalfHeight = uiHeight>>1;
    uint32     uiHalfWidth  = uiWidth>>1;
	
    if (fr)
    {
        // Y plane
        uint8* pBuf = (uint8 *) pBitstreamData;

        for (uint32 h=0; h<uiHeight; h++)
        {
			memcpy(pBuf, fr, sizeof(uint8) * uiWidth);
			fr += uiWidth;
            pBuf += alignedSurfaceWidth;
        }

		// UV plane
        for (uint32 h=0; h<uiHalfHeight; h++)
        {
			memcpy(pBuf, fr, uiWidth);
			fr += uiWidth;
            pBuf += alignedSurfaceWidth;
        }
        results = true;
    }
    return results;
}

// FIXME
bool isH264iFrame(int8 *frame)
{
    if (((int)frame) == 0x000001b6 && frame[4] == 0 && frame[5] == 0)
		return true;
    
    return false;
}

// FIXME SSE2 enabled below don't produce NV12 so kinda useless duh
void ConvertRGB32toYV12_SSE2(const uint8 *src, uint8 *ydest, uint8 *udest, uint8 *vdest, unsigned int w, unsigned int h) {
	const __m128i fraction		= _mm_setr_epi32(0x84000,0x84000,0x84000,0x84000);    //= 0x108000/2 = 0x84000
	const __m128i neg32			= _mm_setr_epi32(-32,-32,-32,-32);
	const __m128i y1y2_mult		= _mm_setr_epi32(0x4A85,0x4A85,0x4A85,0x4A85);
	const __m128i fpix_add		= _mm_setr_epi32(0x808000,0x808000,0x808000,0x808000);
	const __m128i fpix_mul		= _mm_setr_epi32(0x1fb,0x282,0x1fb,0x282);
	const __m128i cybgr_64		= _mm_setr_epi16(0x0c88,0x4087,0x20DE,0,0x0c88,0x4087,0x20DE,0);

	for ( unsigned int y=0;y<h;y+=2){
		uint8 *ydst=ydest+(h-y-1)*w;
		uint8 *udst=udest+(h-y-2)/2*w/2;
		uint8 *vdst=vdest+(h-y-2)/2*w/2;
		for ( unsigned int x=0;x<w;x+=4){
			__m128i rgb0 = _mm_loadl_epi64((__m128i*)&src[y*w*4+x*4]);
			__m128i rgb1 = _mm_loadl_epi64((__m128i*)&src[y*w*4+x*4+8]);
			__m128i rgb2 = _mm_loadl_epi64((__m128i*)&src[y*w*4+x*4+w*4]);
			__m128i rgb3 = _mm_loadl_epi64((__m128i*)&src[y*w*4+x*4+8+w*4]);

			rgb0 = _mm_unpacklo_epi8(rgb0,_mm_setzero_si128());
			rgb1 = _mm_unpacklo_epi8(rgb1,_mm_setzero_si128());
			rgb2 = _mm_unpacklo_epi8(rgb2,_mm_setzero_si128());
			rgb3 = _mm_unpacklo_epi8(rgb3,_mm_setzero_si128());

			__m128i luma0 = _mm_madd_epi16(rgb0,cybgr_64);
			__m128i luma1 = _mm_madd_epi16(rgb1,cybgr_64);
			__m128i luma2 = _mm_madd_epi16(rgb2,cybgr_64);
			__m128i luma3 = _mm_madd_epi16(rgb3,cybgr_64);

			rgb0 = _mm_add_epi16(rgb0,_mm_shuffle_epi32(rgb0,(2<<0)+(3<<2)+(0<<4)+(1<<6)));
			rgb1 = _mm_add_epi16(rgb1,_mm_shuffle_epi32(rgb1,(2<<0)+(3<<2)+(0<<4)+(1<<6)));
			rgb2 = _mm_add_epi16(rgb2,_mm_shuffle_epi32(rgb2,(2<<0)+(3<<2)+(0<<4)+(1<<6)));
			rgb3 = _mm_add_epi16(rgb3,_mm_shuffle_epi32(rgb3,(2<<0)+(3<<2)+(0<<4)+(1<<6)));

			__m128i chroma0 = _mm_unpacklo_epi64(rgb0,rgb1);
			__m128i chroma1 = _mm_unpacklo_epi64(rgb2,rgb3);
			chroma0 = _mm_slli_epi32(chroma0,16); // remove green channel
			chroma1 = _mm_slli_epi32(chroma1,16);

			luma0 = _mm_add_epi32(luma0, _mm_shuffle_epi32(luma0,(1<<0)+(0<<2)+(3<<4)+(2<<6)));
			luma1 = _mm_add_epi32(luma1, _mm_shuffle_epi32(luma1,(1<<0)+(0<<2)+(3<<4)+(2<<6)));
			luma2 = _mm_add_epi32(luma2, _mm_shuffle_epi32(luma2,(1<<0)+(0<<2)+(3<<4)+(2<<6)));
			luma3 = _mm_add_epi32(luma3, _mm_shuffle_epi32(luma3,(1<<0)+(0<<2)+(3<<4)+(2<<6)));
			luma0 = _mm_srli_si128(luma0,4);
			luma1 = _mm_srli_si128(luma1,4);
			luma2 = _mm_srli_si128(luma2,4);
			luma3 = _mm_srli_si128(luma3,4);
			luma0 = _mm_unpacklo_epi64(luma0,luma1);
			luma2 = _mm_unpacklo_epi64(luma2,luma3);

			luma0 = _mm_add_epi32(luma0,fraction);
			luma2 = _mm_add_epi32(luma2,fraction);
			luma0 = _mm_srli_epi32(luma0,15);
			luma2 = _mm_srli_epi32(luma2,15);

			__m128i temp0 = _mm_add_epi32(luma0,_mm_shuffle_epi32(luma0,1+(0<<2)+(3<<4)+(2<<6)));
			__m128i temp1 = _mm_add_epi32(luma2,_mm_shuffle_epi32(luma2,1+(0<<2)+(3<<4)+(2<<6)));
			temp0 = _mm_add_epi32(temp0,neg32);
			temp1 = _mm_add_epi32(temp1,neg32);
			temp0 = _mm_madd_epi16(temp0,y1y2_mult);
			temp1 = _mm_madd_epi16(temp1,y1y2_mult);

			luma0 = _mm_packs_epi32(luma0,luma0);
			luma2 = _mm_packs_epi32(luma2,luma2);
			luma0 = _mm_packus_epi16(luma0,luma0);
			luma2 = _mm_packus_epi16(luma2,luma2);

			//if ( *(int *)&ydst[x]!=_mm_cvtsi128_si32(luma0)||
			//	*(int *)&ydst[x-w]!=_mm_cvtsi128_si32(luma2)){
			//	__asm int 3;
			//}

			*(int *)&ydst[x]=_mm_cvtsi128_si32(luma0);
			*(int *)&ydst[x-w]=_mm_cvtsi128_si32(luma2);

			chroma0 = _mm_srli_epi64(chroma0,2);
			chroma1 = _mm_srli_epi64(chroma1,2);
			chroma0 = _mm_sub_epi32(chroma0,temp0);
			chroma1 = _mm_sub_epi32(chroma1,temp1);
			chroma0 = _mm_srli_epi32(chroma0,9);
			chroma1 = _mm_srli_epi32(chroma1,9);
			chroma0 = _mm_madd_epi16(chroma0,fpix_mul);
			chroma1 = _mm_madd_epi16(chroma1,fpix_mul);
			chroma0 = _mm_add_epi32(chroma0,fpix_add);
			chroma1 = _mm_add_epi32(chroma1,fpix_add);
			chroma0 = _mm_packus_epi16(chroma0,chroma0);
			chroma1 = _mm_packus_epi16(chroma1,chroma1);

			chroma0 = _mm_avg_epu8(chroma0,chroma1);

			chroma0 = _mm_srli_epi16(chroma0,8);
			chroma0 = _mm_shufflelo_epi16(chroma0,0+(2<<2)+(1<<4)+(3<<6));
			chroma0 = _mm_packus_epi16(chroma0,chroma0);
			
			//if ( *(unsigned short *)&udst[x/2]!=_mm_extract_epi16(chroma0,0) ||
 			//	*(unsigned short *)&vdst[x/2]!=_mm_extract_epi16(chroma0,1)){
			//	__asm int 3;
			//}

			*(unsigned short *)&udst[x/2] = _mm_extract_epi16(chroma0,0);
			*(unsigned short *)&vdst[x/2] = _mm_extract_epi16(chroma0,1);
		}
	}
}

void ConvertRGB24toYV12_SSE2(const uint8 *src, uint8 *ydest, uint8 *udest, uint8 *vdest, unsigned int w, unsigned int h) {
	const __m128i fraction		= _mm_setr_epi32(0x84000,0x84000,0x84000,0x84000);    //= 0x108000/2 = 0x84000
	const __m128i neg32			= _mm_setr_epi32(-32,-32,-32,-32);
	const __m128i y1y2_mult		= _mm_setr_epi32(0x4A85,0x4A85,0x4A85,0x4A85);
	const __m128i fpix_add		= _mm_setr_epi32(0x808000,0x808000,0x808000,0x808000);
	const __m128i fpix_mul		= _mm_setr_epi32(0x1fb,0x282,0x1fb,0x282);
	const __m128i cybgr_64		= _mm_setr_epi16(0,0x0c88,0x4087,0x20DE,0x0c88,0x4087,0x20DE,0);

	for ( unsigned int y=0;y<h;y+=2){
		uint8 *ydst=ydest+(h-y-1)*w;
		uint8 *udst=udest+(h-y-2)/2*w/2;
		uint8 *vdst=vdest+(h-y-2)/2*w/2;
		for ( unsigned int x=0;x<w;x+=4){
			__m128i rgb0 = _mm_cvtsi32_si128(*(int*)&src[y*w*3+x*3]);
			__m128i rgb1 = _mm_loadl_epi64((__m128i*)&src[y*w*3+x*3+4]);
			__m128i rgb2 = _mm_cvtsi32_si128(*(int*)&src[y*w*3+x*3+w*3]);
			__m128i rgb3 = _mm_loadl_epi64((__m128i*)&src[y*w*3+x*3+4+w*3]);
			rgb0 = _mm_unpacklo_epi32(rgb0,rgb1);
			rgb0 = _mm_slli_si128(rgb0,1);
			rgb1 = _mm_srli_si128(rgb1,1);

			rgb2 = _mm_unpacklo_epi32(rgb2,rgb3);
			rgb2 = _mm_slli_si128(rgb2,1);
			rgb3 = _mm_srli_si128(rgb3,1);

			rgb0 = _mm_unpacklo_epi8(rgb0,_mm_setzero_si128());
			rgb1 = _mm_unpacklo_epi8(rgb1,_mm_setzero_si128());
			rgb2 = _mm_unpacklo_epi8(rgb2,_mm_setzero_si128());
			rgb3 = _mm_unpacklo_epi8(rgb3,_mm_setzero_si128());

			__m128i luma0 = _mm_madd_epi16(rgb0,cybgr_64);
			__m128i luma1 = _mm_madd_epi16(rgb1,cybgr_64);
			__m128i luma2 = _mm_madd_epi16(rgb2,cybgr_64);
			__m128i luma3 = _mm_madd_epi16(rgb3,cybgr_64);

			rgb0 = _mm_add_epi16(rgb0,_mm_srli_si128(rgb0,6));
			rgb1 = _mm_add_epi16(rgb1,_mm_srli_si128(rgb1,6));
			rgb2 = _mm_add_epi16(rgb2,_mm_srli_si128(rgb2,6));
			rgb3 = _mm_add_epi16(rgb3,_mm_srli_si128(rgb3,6));
			
			__m128i chroma0 = _mm_unpacklo_epi64(rgb0,rgb1);
			__m128i chroma1 = _mm_unpacklo_epi64(rgb2,rgb3);
			chroma0 = _mm_srli_epi32(chroma0,16); // remove green channel
			chroma1 = _mm_srli_epi32(chroma1,16); // remove green channel

			luma0 = _mm_add_epi32(luma0, _mm_shuffle_epi32(luma0,(1<<0)+(0<<2)+(3<<4)+(2<<6)));
			luma1 = _mm_add_epi32(luma1, _mm_shuffle_epi32(luma1,(1<<0)+(0<<2)+(3<<4)+(2<<6)));
			luma2 = _mm_add_epi32(luma2, _mm_shuffle_epi32(luma2,(1<<0)+(0<<2)+(3<<4)+(2<<6)));
			luma3 = _mm_add_epi32(luma3, _mm_shuffle_epi32(luma3,(1<<0)+(0<<2)+(3<<4)+(2<<6)));
			luma0 = _mm_srli_si128(luma0,4);
			luma1 = _mm_srli_si128(luma1,4);
			luma2 = _mm_srli_si128(luma2,4);
			luma3 = _mm_srli_si128(luma3,4);
			luma0 = _mm_unpacklo_epi64(luma0,luma1);
			luma2 = _mm_unpacklo_epi64(luma2,luma3); // luma1, luma3 no longer used

			luma0 = _mm_add_epi32(luma0,fraction);
			luma2 = _mm_add_epi32(luma2,fraction);
			luma0 = _mm_srli_epi32(luma0,15);
			luma2 = _mm_srli_epi32(luma2,15);

			__m128i temp0 = _mm_add_epi32(luma0,_mm_shuffle_epi32(luma0,1+(0<<2)+(3<<4)+(2<<6)));
			__m128i temp1 = _mm_add_epi32(luma2,_mm_shuffle_epi32(luma2,1+(0<<2)+(3<<4)+(2<<6)));
			temp0 = _mm_add_epi32(temp0,neg32);
			temp1 = _mm_add_epi32(temp1,neg32);
			temp0 = _mm_madd_epi16(temp0,y1y2_mult);
			temp1 = _mm_madd_epi16(temp1,y1y2_mult);

			luma0 = _mm_packs_epi32(luma0,luma0);
			luma2 = _mm_packs_epi32(luma2,luma2);
			luma0 = _mm_packus_epi16(luma0,luma0);
			luma2 = _mm_packus_epi16(luma2,luma2);
			unsigned int flipped_pos = (h-y-1)*w+x;

			//if ( *(int *)&ydst[x]!=_mm_cvtsi128_si32(luma0)||
			//	*(int *)&ydst[x-w]!=_mm_cvtsi128_si32(luma2) ){
			//	__asm int 3;
			//}

			*(int *)&ydst[x]=_mm_cvtsi128_si32(luma0);
			*(int *)&ydst[x-w]=_mm_cvtsi128_si32(luma2);


			chroma0 = _mm_slli_epi64(chroma0,14);
			chroma1 = _mm_slli_epi64(chroma1,14);
			chroma0 = _mm_sub_epi32(chroma0,temp0);
			chroma1 = _mm_sub_epi32(chroma1,temp1);
			chroma0 = _mm_srli_epi32(chroma0,9);
			chroma1 = _mm_srli_epi32(chroma1,9);
			chroma0 = _mm_madd_epi16(chroma0,fpix_mul);
			chroma1 = _mm_madd_epi16(chroma1,fpix_mul);
			chroma0 = _mm_add_epi32(chroma0,fpix_add);
			chroma1 = _mm_add_epi32(chroma1,fpix_add);
			chroma0 = _mm_packus_epi16(chroma0,chroma0);
			chroma1 = _mm_packus_epi16(chroma1,chroma1);
			
			chroma0 = _mm_avg_epu8(chroma0,chroma1);

			chroma0 = _mm_srli_epi16(chroma0,8);
			chroma0 = _mm_shufflelo_epi16(chroma0,0+(2<<2)+(1<<4)+(3<<6));
			chroma0 = _mm_packus_epi16(chroma0,chroma0);

			//if ( *(unsigned short *)&udst[x/2]!=_mm_extract_epi16(chroma0,0) ||
			//	*(unsigned short *)&vdst[x/2]!=_mm_extract_epi16(chroma0,1)){
			//	__asm int 3;
			//}

			*(unsigned short *)&udst[x/2]=_mm_extract_epi16(chroma0,0);
			*(unsigned short *)&vdst[x/2]=_mm_extract_epi16(chroma0,1);
		}
	}
}

//AviSynth toyv12
void RGBtoNV12 (const uint8 * rgb,
	uint8 * yuv,
	unsigned rgbIncrement, //bpp in bytes
	uint8 flip,
	int srcFrameWidth, int srcFrameHeight, uint32 yuvPitch)
{

	// Colour conversion from
	// http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html#RTFToC30
	//
	// YCbCr in Rec. 601 format
	// RGB values are in the range [0..255]
	//
	// [ Y  ]   [  16 ]    1    [  65.738    129.057    25.064  ]   [ R ]
	// [ Cb ] = [ 128 ] + --- * [ -37.945    -74.494    112.439 ] * [ G ]
	// [ Cr ]   [ 128 ]   256   [ 112.439    -94.154    -18.285 ]   [ B ]

	int rgbPitch = srcFrameWidth;
	unsigned int planeSize;
	unsigned int halfWidth;
	unsigned int padRGB = 0;
	unsigned int padYUV =  yuvPitch - srcFrameWidth; //Should work for Y (Y sample per pixel == 1 byte) and UV (2x half sample per pixel == 1 byte)

	uint8 * Y;
	uint8 * UV;
	//uint8 * V;
	int x,y;

	planeSize = yuvPitch * srcFrameHeight;
	halfWidth = yuvPitch >> 1;

	// get pointers to the data
	Y = yuv;
	UV = yuv + planeSize;
	const uint8 * src = rgb;

	long RtoYCoeff = long (65.738  * 256 + 0.5);
	long GtoYCoeff = long (129.057 * 256 + 0.5);
	long BtoYCoeff = long (25.064  * 256 + 0.5);

	long RtoUCoeff = long (-37.945 * 256 + 0.5);
	long GtoUCoeff = long (-74.494 * 256 + 0.5);
	long BtoUCoeff = long (112.439 * 256 + 0.5);

	long RtoVCoeff = long (112.439 * 256 + 0.5);
	long GtoVCoeff = long (-94.154 * 256 + 0.5);
	long BtoVCoeff = long (-18.285 * 256 + 0.5);

	uint8 U00, U01, U10, U11;
	uint8 V00, V01, V10, V11;

#define rgbTo(s, x, y) s += x + y * rgbPitch * rgbIncrement
#define rgbAt(s, x, y) (*(s + x + y * rgbPitch * rgbIncrement))
#define yuvTo(s, x, y) s += x + y * yuvPitch
#define yuvTo2(s, x, y) s += x + y * halfWidth

	//#pragma omp parallel
	{
	////#pragma omp section
	{
		//Y plane
		//#pragma omp parallel for 
		for ( y = srcFrameHeight - 1; y >= 0; y--)
		{
			uint8 *lY;
			if(!!flip)
				lY = Y + yuvPitch * (srcFrameHeight - y);
			else
				lY = Y + yuvPitch * y;//, src += padRGB
			const uint8 *lsrc = rgb + (srcFrameWidth*(y)*rgbIncrement);
			for ( x = srcFrameWidth ; x > 0; x--)
			{
				lsrc += rgbIncrement;
				// No need to saturate between 16 and 235
				*(lY++) = 16 + ((32768 + RtoYCoeff * lsrc[0] + GtoYCoeff * lsrc[1] + BtoYCoeff * lsrc[2]) >> 16);
			}
		}
	}
	
	//rgbTo(src, 0,-1); //rgbTo(src, 0, -srcFrameHeight); //.to(0, -src.height); //we return to the beginning of the plane.

	//U and V planes
	
	//#pragma omp for 
	for (y = 0; y < (srcFrameHeight>>1); y++)
	{
		//UV += padYUV, src+=rgbIncrement*rgbPitch
		uint8 *lUV;
		if(!!flip)
			lUV = UV + yuvPitch * ((srcFrameHeight>>1) - y - 1);
		else
			lUV = UV + yuvPitch * y;

		const uint8 *lsrc = rgb + (srcFrameWidth*(y*2)*rgbIncrement);
		
		for (x = 0; x < (srcFrameWidth>>1); x++)
		{
			yuvTo2(lUV, 2, 0);
			lsrc += rgbIncrement*2;

			// No need to saturate between 16 and 240
			U00 = 128 + ((32768 + RtoUCoeff * lsrc[0] + GtoUCoeff * lsrc[1] + BtoUCoeff * lsrc[2]) >> 16);
			U01 = 128 + ((32768 + RtoUCoeff * lsrc[0+rgbIncrement] + GtoUCoeff * lsrc[1+rgbIncrement] + BtoUCoeff * lsrc[2+rgbIncrement]) >> 16);
			U10 = 128 + ((32768 + RtoUCoeff * rgbAt(lsrc, 0, 1) + GtoUCoeff * rgbAt(lsrc, 1, 1) + BtoUCoeff * rgbAt(lsrc, 2, 1)) >> 16);
			U11 = 128 + ((32768 + RtoUCoeff * rgbAt(lsrc, 0+rgbIncrement, 1) + GtoUCoeff * (rgbAt(lsrc, 1+rgbIncrement, 1)) + BtoUCoeff * rgbAt(lsrc, 2+rgbIncrement, 1)) >> 16);
			lUV[1] = (2 + U00 + U01 + U10 + U11) >> 2;

			V00 = 128 + ((32768 + RtoVCoeff * lsrc[0] + GtoVCoeff * lsrc[1] + BtoVCoeff * lsrc[2]) >> 16);
			V01 = 128 + ((32768 + RtoVCoeff * lsrc[0+rgbIncrement] + GtoVCoeff * lsrc[1+rgbIncrement] + BtoVCoeff * lsrc[2+rgbIncrement]) >> 16);
			V10 = 128 + ((32768 + RtoVCoeff * rgbAt(lsrc, 0, 1) + GtoVCoeff * rgbAt(lsrc, 1, 1) + BtoVCoeff * rgbAt(lsrc, 2, 1)) >> 16);
			V11 = 128 + ((32768 + RtoVCoeff * rgbAt(lsrc, 0+rgbIncrement, 1) + GtoVCoeff * rgbAt(lsrc, 1+rgbIncrement, 1) + BtoVCoeff * rgbAt(lsrc, 2+rgbIncrement, 1)) >> 16);
			lUV[0] = (2 + V00 + V01 + V10 + V11) >> 2; //UV[0] ... wat?
            
			//UV[0] = -0.14713f * src[0] - 0.28886f * src[1] + 0.436f * src[2] + 128;
			//UV[1] = 0.615f * src[0] - 0.51499f * src[1] - 0.10001f * src[2] + 128;
		}
	}
	}

}
