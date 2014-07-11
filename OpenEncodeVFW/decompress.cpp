#include "stdafx.h"
#include "OpenEncodeVFW.h"

// check if the codec can decompress the given format to the desired format
DWORD CodecInst::DecompressQuery(const LPBITMAPINFOHEADER lpbiIn, const LPBITMAPINFOHEADER lpbiOut){

	if ( lpbiIn->biCompression != FOURCC_H264 ){
		return_badformat();
	}

	return_badformat();//TODO Doesn't support decoding yet

	return (DWORD)ICERR_OK;
}

// return the default decompress format for the given input format 
DWORD CodecInst::DecompressGetFormat(const LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){

	if ( DecompressQuery(lpbiIn, NULL ) != ICERR_OK){
		return_badformat();
	}

	if ( !lpbiOut)
		return sizeof(BITMAPINFOHEADER);

	
	return_badformat();

	return (DWORD)ICERR_OK;
}

DWORD CodecInst::DecompressGetPalette(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut) {
	return_badformat()
}

// initalize the codec for decompression
DWORD CodecInst::DecompressBegin(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){
	if ( started == 0x1337){
		DecompressEnd();
	}
	started = 0;

	if ( int error = DecompressQuery(lpbiIn,lpbiOut) != ICERR_OK ){
		return error;
	}
	
	started = 0x1337;
	return ICERR_OK;
}

// release resources when decompression is done
DWORD CodecInst::DecompressEnd(){
	if ( started == 0x1337 ){

	}
	started=0;
	return ICERR_OK;
}


// Called to decompress a frame, the actual decompression will be
// handed off to other functions based on the frame type.
DWORD CodecInst::Decompress(ICDECOMPRESS* icinfo, DWORD dwSize) {
//	try {

	DWORD return_code=ICERR_OK;
	if ( started != 0x1337 ){
		DecompressBegin(icinfo->lpbiInput,icinfo->lpbiOutput);
	}
	//out = (uint8 *)icinfo->lpOutput;
	//in  = (uint8 *)icinfo->lpInput; 
	icinfo->lpbiOutput->biSizeImage = 0;//length;

	mCompressed_size = icinfo->lpbiInput->biSizeImage;

	return_badformat(); //TODO Doesn't support decoding yet
	return return_code;
//} catch ( ... ){
//	MessageBox (HWND_DESKTOP, "Exception caught in decompress main", "Error", MB_OK | MB_ICONEXCLAMATION);
//	return ICERR_INTERNAL;
//}
}

//MessageBox (HWND_DESKTOP, msg, "Error", MB_OK | MB_ICONEXCLAMATION);