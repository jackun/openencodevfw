#include "stdafx.h"
#include "OpenEncodeVFW.h"
#include <CommCtrl.h>

extern HMODULE hmoduleVFW;
static bool firstInit = true;
static HWND hwndToolTip = NULL;
static TOOLINFO ti;

#define SETTOOLTIP(ctrl,str)\
	do{\
		ti.lpszText = (LPWSTR)L ##str; \
		ti.uId = (UINT_PTR)GetDlgItem(hwndDlg, ctrl);\
		SendMessageW(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);\
	}while(0)

//FIXME Proper maximum, 100 Mbps with 4.1+ level, probably
#define MAX_BITRATE 50000 //kbit/s
#define MAX_QUANT 51 //0...51 for I/P, (B frames are unsupported, but new driver (and hawaii+) does?)

void CodecInst::quickSet(int qs)
{
	prepareConfigMap(true); //reset
	//Shared settings
	mConfigTable["encEnImeOverwDisSubm"] = 0;
	mConfigTable["encImeOverwDisSubmNo"] = 0;
	mConfigTable["forceZeroPointCenter"] = 0;
	mConfigTable["enableAMD"] = 0;
	mConfigTable["encForce16x16skip"] = 0;

	switch(qs)
	{
	case 0: //speed
		mConfigTable["encSearchRangeX"] = 16;
		mConfigTable["encSearchRangeY"] = 16;
		mConfigTable["encDisableSubMode"] = 254;
		mConfigTable["encForce16x16skip"] = 1;
		break;
	case 1: //balanced
		mConfigTable["encSearchRangeX"] = 24;//balanced.cfg has it on 16
		mConfigTable["encSearchRangeY"] = 24;
		mConfigTable["encDisableSubMode"] = 120;
		mConfigTable["encEnImeOverwDisSubm"] = 1;
		mConfigTable["encImeOverwDisSubmNo"] = 1;
		break;
	case 2: //quality
		mConfigTable["forceZeroPointCenter"] = 1;
		mConfigTable["encSearchRangeX"] = 36;
		mConfigTable["encSearchRangeY"] = 36;
		mConfigTable["enableAMD"] = 1;
		mConfigTable["encDisableSubMode"] = 0;
		break;
	default:
		break;
	}
}

void CodecInst::prepareConfigMap(bool quickset)
{
	/**************************************************************************/
	/* ConfigPicCtl                                                           */
	/**************************************************************************/
	mConfigTable["useConstrainedIntraPred"] = 0; 
	mConfigTable["CABACEnable"] = 1; 
	mConfigTable["CABACIDC"] = 0; 
	mConfigTable["loopFilterDisable"] = 0; 
	mConfigTable["encLFBetaOffset"] = 0; 
	mConfigTable["encLFAlphaC0Offset"] = 0; 
	mConfigTable["encIDRPeriod"] = 0; 
	mConfigTable["encIPicPeriod"] = 0; 
	mConfigTable["encHeaderInsertionSpacing"] = 0; 
	mConfigTable["encCropLeftOffset"] = 0; 
	mConfigTable["encCropRightOffset"] = 0; 
	mConfigTable["encCropTopOffset"] = 0; 
	mConfigTable["encCropBottomOffset"] = 0; 
	mConfigTable["encNumMBsPerSlice"] = 99; 
	mConfigTable["encNumSlicesPerFrame"] = 1; 
	mConfigTable["encForceIntraRefresh"] = 0; 
	mConfigTable["encForceIMBPeriod"] = 0; 
	mConfigTable["encInsertVUIParam"] = 0; 
	mConfigTable["encInsertSEIMsg"] = 0; 

	/**************************************************************************/
	/* ConfigMotionEstimation				                                  */
	/**************************************************************************/
	mConfigTable["IMEDecimationSearch"] = 1; 
	mConfigTable["motionEstHalfPixel"] = 1; 
	mConfigTable["motionEstQuarterPixel"] = 1; 
	mConfigTable["disableFavorPMVPoint"] = 0; 
	mConfigTable["forceZeroPointCenter"] = 0; 
	mConfigTable["LSMVert"] = 0; 
	mConfigTable["encSearchRangeX"] = 16; 
	mConfigTable["encSearchRangeY"] = 16; 
	mConfigTable["encSearch1RangeX"] = 0; 
	mConfigTable["encSearch1RangeY"] = 0; 
	mConfigTable["disable16x16Frame1"] = 0; 
	mConfigTable["disableSATD"] = 0; 
	mConfigTable["enableAMD"] = 0; 
	mConfigTable["encDisableSubMode"] = 0; 
	mConfigTable["encIMESkipX"] = 0; 
	mConfigTable["encIMESkipY"] = 0; 
	mConfigTable["encEnImeOverwDisSubm"] = 0; 
	mConfigTable["encImeOverwDisSubmNo"] = 0; 
	mConfigTable["encIME2SearchRangeX"] = 4; 
	mConfigTable["encIME2SearchRangeY"] = 4; 

	/**************************************************************************/
	/* ConfigRDO                                                              */
	/**************************************************************************/
	mConfigTable["encDisableTbePredIFrame"] = 0; 
	mConfigTable["encDisableTbePredPFrame"] = 0; 
	mConfigTable["useFmeInterpolY"] = 0; 
	mConfigTable["useFmeInterpolUV"] = 0; 
	mConfigTable["enc16x16CostAdj"] = 0; 
	mConfigTable["encSkipCostAdj"] = 0; 
	mConfigTable["encForce16x16skip"] = 0; 

	if(quickset) return;

	/**************************************************************************/
	/* EncodeSpecifications                                                   */
	/**************************************************************************/
	mConfigTable["pictureHeight"] = 144; 
	mConfigTable["pictureWidth"] = 176; 
	mConfigTable["EncodeMode"] = 1; 
	mConfigTable["level"] = 41; 
	mConfigTable["profile"] = 77; // 66 -base, 77 - main, 100 - high
	mConfigTable["pictureFormat"] = 1; // 1 - NV12
	mConfigTable["requestedPriority"] = 1; 

	/**************************************************************************/
	/* ConfigRateCtl                                                          */
	/**************************************************************************/
	mConfigTable["encRateControlMethod"] = 4; 
	mConfigTable["encRateControlTargetBitRate"] = 8000000; 
	mConfigTable["encRateControlPeakBitRate"] = 0; 
	mConfigTable["encRateControlFrameRateNumerator"] = 30; 
	mConfigTable["encRateControlFrameRateDenominator"] = 1; 
	mConfigTable["encGOPSize"] = 0; 
	mConfigTable["encRCOptions"] = 0; 
	mConfigTable["encQP_I"] = 22; 
	mConfigTable["encQP_P"] = 22; 
	mConfigTable["encQP_B"] = 0; 
	mConfigTable["encVBVBufferSize"] = mConfigTable["encRateControlTargetBitRate"] / 2; //4000000;

	//Custom app settings
	mConfigTable["sendFPS"] = 0; //Send proper video fps to encoder. Not sending allows video conversion with weird framerates
	mConfigTable["YV12AsNV12"] = 0;//YUV or YVU or UYV or...
	mConfigTable["SpeedyMath"] = 1; //Enable some OpenCL optimizations
	mConfigTable["ColorspaceLimit"] = 0; //Limit rgb between 16..239
	mConfigTable["Log"] = 0;
	mConfigTable["LogMsgBox"] = 0;
	mConfigTable["IDRframes"] = 250; //encIDRPeriod?
	mConfigTable["ProfileKernels"] = 0;
	mConfigTable["UseDevice"] = 0;
	mConfigTable["SwitchByteOrder"] = 0; //BGR(A) or RGB(A), MSDN says BMP uses BGR
	mConfigTable["crop"] = 1;
}

bool CodecInst::readRegistry()
{
	HKEY    hKey;
	DWORD   i_size;
	int32     i;

	if (RegOpenKeyEx(OVE_REG_KEY, OVE_REG_PARENT L"\\" OVE_REG_CHILD, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	EnterCriticalSection(&ove_CS);

	/* Read all named params */
	auto it = mConfigTable.begin();
	for(; it != mConfigTable.end(); it++)
	{
		i_size = sizeof(int32);
		if (RegQueryValueExA(hKey, it->first.c_str(), 0, 0, (LPBYTE)&i, &i_size) == ERROR_SUCCESS)
			it->second = i;
	}

	i_size = sizeof(int32);
	if (RegQueryValueEx(hKey, L"useCLConversion", 0, 0, (LPBYTE)&i, &i_size) == ERROR_SUCCESS)
		mUseCLConv = (i == 1);
	if (RegQueryValueEx(hKey, L"useCLonCPU", 0, 0, (LPBYTE)&i, &i_size) == ERROR_SUCCESS)
		mUseCPU = (i == 1);

	LeaveCriticalSection(&ove_CS);
	RegCloseKey(hKey);

	return true;
}

bool CodecInst::saveRegistry()
{
	HKEY    hKey;
	DWORD   dwDisposition;
	int32     i;
	LSTATUS s;

	if (RegCreateKeyEx(OVE_REG_KEY, OVE_REG_PARENT L"\\" OVE_REG_CHILD, 0, OVE_REG_CLASS, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hKey, &dwDisposition) != ERROR_SUCCESS)
		return false;

	EnterCriticalSection(&ove_CS);

	/* Save all integers */
	auto it = mConfigTable.begin();
	for(; it != mConfigTable.end(); it++)
		s = RegSetValueExA(hKey, it->first.c_str(), 0, REG_DWORD, (LPBYTE)&it->second, sizeof(int32));

	i = mUseCLConv ? 1 : 0;
	RegSetValueEx(hKey, L"useCLConversion", 0, REG_DWORD, (LPBYTE)&i, sizeof(int));
	i = mUseCPU ? 1 : 0;
	RegSetValueEx(hKey, L"useCLonCPU", 0, REG_DWORD, (LPBYTE)&i, sizeof(int));

	//RegSetValueEx(hKey, L"StringKey", 0, REG_SZ, (LPBYTE)L"Value", wcslen(L"Value") + 1);
	LeaveCriticalSection(&ove_CS);
	RegCloseKey(hKey);
	return true;
}

bool CodecInst::readConfigFile(int8 *fileName, OvConfigCtrl *pConfig,
					std::map<std::string,int32>* pConfigTable)
{
	char name[1000];
	int32 index;
	int32 value;

	std::ifstream file;
	file.open(fileName);
	
	if (!file)
	{
		printf("Error in reading the configuration file: %s\n", fileName);
		wchar_t msg[MAX_PATH + 128];
		swprintf_s(msg, MAX_PATH + 128, L"Error in reading the configuration file: %S", fileName);
		MessageBox (HWND_DESKTOP, msg, L"Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	std::string line;
	map<string, int32>::iterator itMap;
	
	while (std::getline(file, line))
	{
		std::string temp = line;
		index = 0;
		sscanf_s(line.c_str(), "%s %d", name, &value);
		itMap = pConfigTable->find(name);
		if(itMap != pConfigTable->end())
		{
			itMap->second = value;
		}
	
	}

	/**************************************************************************/
	/* Set user specified configuratin                                        */
	/**************************************************************************/
	encodeSetParam(pConfig, pConfigTable);

	file.close();
	return true;
}

/** 
 *******************************************************************************
 *  @fn     encodeSetParam
 *  @brief  Setting up configuration parameters
 *           
 *  @param[in/out] pConfig   : Pointer to the configuration structure 
 *  @param[in] pConfigTable  : Pointer to the configuration map table
 *          
 *  @return void
 *******************************************************************************
 */
void CodecInst::encodeSetParam(OvConfigCtrl *pConfig, map<string,int32>* pConfigTable)
{

	/**************************************************************************/
	/* fill-in the general configuration structures                           */
	/**************************************************************************/
	map<string,int32> configTable = (map<string,int32>)*pConfigTable;
	pConfig->height									= configTable["pictureHeight"];
	pConfig->width									= configTable["pictureWidth"];
	pConfig->encodeMode								= (OVE_ENCODE_MODE) configTable["EncodeMode"];

	/**************************************************************************/
	/* fill-in the profile and level                                          */
	/**************************************************************************/
	pConfig->profileLevel.level						= configTable["level"];
	pConfig->profileLevel.profile					= configTable["profile"];

	pConfig->pictFormat								= (OVE_PICTURE_FORMAT)configTable["pictureFormat"];
	pConfig->priority								= (OVE_ENCODE_TASK_PRIORITY)configTable["requestedPriority"];

	/**************************************************************************/
	/* fill-in the picture control structures                                 */
	/**************************************************************************/
	pConfig->pictControl.size						= sizeof(OVE_CONFIG_PICTURE_CONTROL);
	pConfig->pictControl.useConstrainedIntraPred	= configTable["useConstrainedIntraPred"];
	pConfig->pictControl.cabacEnable				= configTable["CABACEnable"];
	pConfig->pictControl.cabacIDC					= configTable["CABACIDC"];
	pConfig->pictControl.loopFilterDisable			= configTable["loopFilterDisable"];
	pConfig->pictControl.encLFBetaOffset			= configTable["encLFBetaOffset"];
	pConfig->pictControl.encLFAlphaC0Offset			= configTable["encLFAlphaC0Offset"];
	pConfig->pictControl.encIDRPeriod				= configTable["encIDRPeriod"];
	pConfig->pictControl.encIPicPeriod				= configTable["encIPicPeriod"];
	pConfig->pictControl.encHeaderInsertionSpacing	= configTable["encHeaderInsertionSpacing"];
	pConfig->pictControl.encCropLeftOffset			= configTable["encCropLeftOffset"];
	pConfig->pictControl.encCropRightOffset			= configTable["encCropRightOffset"];
	pConfig->pictControl.encCropTopOffset			= configTable["encCropTopOffset"];
	pConfig->pictControl.encCropBottomOffset		= configTable["encCropBottomOffset"];
	pConfig->pictControl.encNumMBsPerSlice			= configTable["encNumMBsPerSlice"];
	pConfig->pictControl.encNumSlicesPerFrame		= configTable["encNumSlicesPerFrame"];
	pConfig->pictControl.encForceIntraRefresh		= configTable["encForceIntraRefresh"];
	pConfig->pictControl.encForceIMBPeriod			= configTable["encForceIMBPeriod"];
	pConfig->pictControl.encInsertVUIParam			= configTable["encInsertVUIParam"];
	pConfig->pictControl.encInsertSEIMsg			= configTable["encInsertSEIMsg"];

	/**************************************************************************/
	/* fill-in the rate control structures                                    */
	/**************************************************************************/
	pConfig->rateControl.size							= sizeof(OVE_CONFIG_RATE_CONTROL);
	pConfig->rateControl.encRateControlMethod			= configTable["encRateControlMethod"];
	pConfig->rateControl.encRateControlTargetBitRate	= configTable["encRateControlTargetBitRate"];
	pConfig->rateControl.encRateControlPeakBitRate		= configTable["encRateControlPeakBitRate"];
	pConfig->rateControl.encRateControlFrameRateNumerator = configTable["encRateControlFrameRateNumerator"];
	pConfig->rateControl.encGOPSize						= configTable["encGOPSize"];
	pConfig->rateControl.encRCOptions					= configTable["encRCOptions"];
	pConfig->rateControl.encQP_I						= configTable["encQP_I"];
	pConfig->rateControl.encQP_P						= configTable["encQP_P"];
	pConfig->rateControl.encQP_B						= configTable["encQP_B"];
	pConfig->rateControl.encVBVBufferSize				= configTable["encVBVBufferSize"];
	pConfig->rateControl.encRateControlFrameRateDenominator = configTable["encRateControlFrameRateDenominator"];

	/**************************************************************************/
	/* fill-in the motion estimation control structures                       */
	/**************************************************************************/
	pConfig->meControl.size							= sizeof(OVE_CONFIG_MOTION_ESTIMATION);
	pConfig->meControl.imeDecimationSearch			= configTable["IMEDecimationSearch"];
	pConfig->meControl.motionEstHalfPixel			= configTable["motionEstHalfPixel"];
	pConfig->meControl.motionEstQuarterPixel		= configTable["motionEstQuarterPixel"];
	pConfig->meControl.disableFavorPMVPoint			= configTable["disableFavorPMVPoint"];
	pConfig->meControl.forceZeroPointCenter			= configTable["forceZeroPointCenter"];
	pConfig->meControl.lsmVert						= configTable["LSMVert"];
	pConfig->meControl.encSearchRangeX				= configTable["encSearchRangeX"];
	pConfig->meControl.encSearchRangeY				= configTable["encSearchRangeY"];
	pConfig->meControl.encSearch1RangeX				= configTable["encSearch1RangeX"];
	pConfig->meControl.encSearch1RangeY				= configTable["encSearch1RangeY"];
	pConfig->meControl.disable16x16Frame1			= configTable["disable16x16Frame1"];
	pConfig->meControl.disableSATD					= configTable["disableSATD"];
	pConfig->meControl.enableAMD					= configTable["enableAMD"];
	pConfig->meControl.encDisableSubMode			= configTable["encDisableSubMode"];
	pConfig->meControl.encIMESkipX					= configTable["encIMESkipX"];
	pConfig->meControl.encIMESkipY					= configTable["encIMESkipY"];
	pConfig->meControl.encEnImeOverwDisSubm			= configTable["encEnImeOverwDisSubm"];
	pConfig->meControl.encImeOverwDisSubmNo			= configTable["encImeOverwDisSubmNo"];
	pConfig->meControl.encIME2SearchRangeX			= configTable["encIME2SearchRangeX"];
	pConfig->meControl.encIME2SearchRangeY			= configTable["encIME2SearchRangeY"];

	/**************************************************************************/
	/* fill-in the RDO control structures                                     */
	/**************************************************************************/
	pConfig->rdoControl.size                        = sizeof(OVE_CONFIG_RDO);
	pConfig->rdoControl.encDisableTbePredIFrame		= configTable["encDisableTbePredIFrame"];
	pConfig->rdoControl.encDisableTbePredPFrame		= configTable["encDisableTbePredPFrame"];
	pConfig->rdoControl.useFmeInterpolY				= configTable["useFmeInterpolY"];
	pConfig->rdoControl.useFmeInterpolUV			= configTable["useFmeInterpolUV"];
	pConfig->rdoControl.enc16x16CostAdj				= configTable["enc16x16CostAdj"];
	pConfig->rdoControl.encSkipCostAdj				= configTable["encSkipCostAdj"];
	pConfig->rdoControl.encForce16x16skip			= (uint8)configTable["encForce16x16skip"];
}

/** 
 *******************************************************************************
 *  @fn     setEncodeConfig
 *  @brief  This function sets the encoder configuration by using user 
 *          supplied configuration information from .cfg file
 *           
 *  @param[in] session : Encoder session for which encoder configuration to be
 *                       set
 *  @param[in] pConfig : pointer to the user configuration from .cfg file
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool CodecInst::setEncodeConfig(ove_session session, OvConfigCtrl *pConfig)
{
	uint32                    numOfConfigBuffers = 4;
	OVE_CONFIG                      configBuffers[4];
	OVresult res = 0;

	/**************************************************************************/
	/* send configuration values for this session                             */
	/**************************************************************************/
	configBuffers[0].config.pPictureControl     = &(pConfig->pictControl);
	configBuffers[0].configType                 = OVE_CONFIG_TYPE_PICTURE_CONTROL;
	configBuffers[1].config.pRateControl        = &(pConfig->rateControl);
	configBuffers[1].configType                 = OVE_CONFIG_TYPE_RATE_CONTROL;
	configBuffers[2].config.pMotionEstimation   = &(pConfig->meControl);
	configBuffers[2].configType                 = OVE_CONFIG_TYPE_MOTION_ESTIMATION;
	configBuffers[3].config.pRDO                = &(pConfig->rdoControl);
	configBuffers[3].configType                 = OVE_CONFIG_TYPE_RDO;
	res = OVEncodeSendConfig (session, numOfConfigBuffers, configBuffers);
	if (!res)
	{
		LogMsg(mMsgBox, L"OVEncodeSendConfig returned error\n");
		return false;
	}

	/**************************************************************************/
	/* Just verifying that the values have been set in the                    */
	/* encoding engine.                                                       */
	/**************************************************************************/
	OVE_CONFIG_PICTURE_CONTROL      pictureControlConfig;
	OVE_CONFIG_RATE_CONTROL         rateControlConfig;
	OVE_CONFIG_MOTION_ESTIMATION    meControlConfig;
	OVE_CONFIG_RDO                  rdoControlConfig;

	/**************************************************************************/
	/* get the picture control configuration.                                 */
	/**************************************************************************/
	memset(&pictureControlConfig, 0, sizeof(OVE_CONFIG_PICTURE_CONTROL));
	pictureControlConfig.size = sizeof(OVE_CONFIG_PICTURE_CONTROL);
	res = OVEncodeGetPictureControlConfig(session, &pictureControlConfig);

	/**************************************************************************/
	/* get the rate control configuration                                     */
	/**************************************************************************/
	memset(&rateControlConfig, 0, sizeof(OVE_CONFIG_RATE_CONTROL));
	rateControlConfig.size = sizeof(OVE_CONFIG_RATE_CONTROL);
	res = OVEncodeGetRateControlConfig(session, &rateControlConfig); 

	/**************************************************************************/
	/* get the MotionEstimation configuration                                 */
	/**************************************************************************/
	memset(&meControlConfig, 0, sizeof(OVE_CONFIG_MOTION_ESTIMATION));
	meControlConfig.size = sizeof(OVE_CONFIG_MOTION_ESTIMATION);
	res = OVEncodeGetMotionEstimationConfig(session, &meControlConfig); 

	/**************************************************************************/
	/* get the RDO configuration                                              */
	/**************************************************************************/
	memset(&rdoControlConfig, 0, sizeof(OVE_CONFIG_RDO));
	rdoControlConfig.size = sizeof(OVE_CONFIG_RDO);
	res = OVEncodeGetRDOControlConfig(session, &rdoControlConfig); 
	
	return(res);
}

static double GetDlgItemDouble(HWND hDlg, int nIDDlgItem)
{
	wchar_t temp[1024];

	if (GetDlgItemText(hDlg, nIDDlgItem, temp, 1024) == 0)
		wcscpy_s(temp, 1, L"");
	return _wtof(temp);
}

static void SetDlgItemDouble(HWND hDlg, int nIDDlgItem, double dblValue, const wchar_t *format)
{
	wchar_t temp[1024];

	swprintf(temp, 1023, format, dblValue);
	SetDlgItemText(hDlg, nIDDlgItem, temp);
}

static int pos2scale(int i_pos)
{
	int res;
	if (i_pos <= 100)
		res = i_pos;
	else
	{
		int i = 1;
		i_pos -= 10;
		while (i_pos > 90)
		{
			i_pos -= 90;
			i *= 10;
		}
		res = (i_pos + 10) * i;
	}
	return res;
}

static int scale2pos(int i_scale)
{
	int res;
	if (i_scale <= 100)
		res = i_scale;
	else
	{
		int i = 900;
		res = 100;
		i_scale -= 100;
		while (i_scale > i)
		{
			res += 90;
			i_scale -= i;
			i *= 10;
		}
		i /= 90;
		res += (i_scale + (i >> 1)) / i;
	}
	return res;
}

#define RCFromCB(rc)\
do {\
	switch (rc)\
	{\
		case 0: rc = 0; break;\
		case 1: rc = 3; break;\
		case 2: rc = 4; break;\
		default: assert(0); break;\
	}\
} while (0)

//x264vfw
static void CheckControlTextIsNumber(HWND hDlgItem, int bSigned, int iDecimalPlacesNum)
{
	wchar_t text_old[MAX_PATH];
	wchar_t text_new[MAX_PATH];
	wchar_t *src, *dest;
	DWORD start, end, num, pos;
	int bChanged = FALSE;
	int bCopy = FALSE;
	int q = !bSigned;

	SendMessage(hDlgItem, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
	num = SendMessage(hDlgItem, WM_GETTEXT, MAX_PATH, (LPARAM)text_old);
	src = text_old;
	dest = text_new;
	pos = 0;
	while (num > 0)
	{
		bCopy = TRUE;
		if (q == 0 && *src == '-')
		{
			q = 1;
		}
		else if ((q == 0 || q == 1) && *src >= '0' && *src <= '9')
		{
			q = 2;
		}
		else if (q == 2 && *src >= '0' && *src <= '9')
		{
		}
		else if (q == 2 && iDecimalPlacesNum > 0 && *src == '.')
		{
			q = 3;
		}
		else if (q == 3 && iDecimalPlacesNum > 0 && *src >= '0' && *src <= '9')
		{
			iDecimalPlacesNum--;
		}
		else
			bCopy = FALSE;
		if (bCopy)
		{
			*dest = *src;
			dest++;
			pos++;
		}
		else
		{
			bChanged = TRUE;
			if (pos < start)
				start--;
			if (pos < end)
				end--;
		}
		src++;
		num--;
	}
	*dest = 0;
	if (bChanged)
	{
		SendMessage(hDlgItem, WM_SETTEXT, 0, (LPARAM)text_new);
		SendMessage(hDlgItem, EM_SETSEL, start, end);
	}
}

#define CHECKED_SET_INT(var, hDlg, nIDDlgItem, bSigned, min, max)\
do {\
	CheckControlTextIsNumber(GetDlgItem(hDlg, nIDDlgItem), bSigned, 0);\
	var = GetDlgItemInt(hDlg, nIDDlgItem, NULL, bSigned);\
	if (var < min)\
	{\
		var = min;\
		SetDlgItemInt(hDlg, nIDDlgItem, var, bSigned);\
		SendMessage(GetDlgItem(hDlg, nIDDlgItem), EM_SETSEL, -2, -2);\
	}\
	else if (var > max)\
	{\
		var = max;\
		SetDlgItemInt(hDlg, nIDDlgItem, var, bSigned);\
		SendMessage(GetDlgItem(hDlg, nIDDlgItem), EM_SETSEL, -2, -2);\
	}\
} while (0)

#define CHECKED_SET_MIN_INT(var, hDlg, nIDDlgItem, bSigned, min, max)\
do {\
	CheckControlTextIsNumber(GetDlgItem(hDlg, nIDDlgItem), bSigned, 0);\
	var = GetDlgItemInt(hDlg, nIDDlgItem, NULL, bSigned);\
	if (var < min)\
	{\
		var = min;\
		SetDlgItemInt(hDlg, nIDDlgItem, var, bSigned);\
		SendMessage(GetDlgItem(hDlg, nIDDlgItem), EM_SETSEL, -2, -2);\
	}\
	else if (var > max)\
		var = max;\
} while (0)

#define CHECKED_SET_MAX_INT(var, hDlg, nIDDlgItem, bSigned, min, max)\
do {\
	CheckControlTextIsNumber(GetDlgItem(hDlg, nIDDlgItem), bSigned, 0);\
	var = GetDlgItemInt(hDlg, nIDDlgItem, NULL, bSigned);\
	if (var < min)\
		var = min;\
	else if (var > max)\
	{\
		var = max;\
		SetDlgItemInt(hDlg, nIDDlgItem, var, bSigned);\
		SendMessage(GetDlgItem(hDlg, nIDDlgItem), EM_SETSEL, -2, -2);\
	}\
} while (0)

#define CHECKED_SET_SHOW_INT(var, hDlg, nIDDlgItem, bSigned, min, max)\
do {\
	CheckControlTextIsNumber(GetDlgItem(hDlg, nIDDlgItem), bSigned, 0);\
	var = GetDlgItemInt(hDlg, nIDDlgItem, NULL, bSigned);\
	if (var < min)\
		var = min;\
	else if (var > max)\
		var = max;\
	SetDlgItemInt(hDlg, nIDDlgItem, var, bSigned);\
} while (0)

#define LevelToIdx(l)\
	do {\
	switch (l)\
	{\
		case 30: l = 0; break;\
		case 31: l = 1; break;\
		case 32: l = 2; break;\
		case 40: l = 3; break;\
		case 41: l = 4; break;\
		case 42: l = 5; break;\
		case 50: l = 6; break;\
		case 51: l = 7; break;\
		default:\
		  l = 3;\
		  break;\
	}\
} while (0)

#define IdxToLevel(l)\
	do {\
	switch (l)\
	{\
		case 0: l = 30; break;\
		case 1: l = 31; break;\
		case 2: l = 32; break;\
		case 3: l = 40; break;\
		case 4: l = 41; break;\
		case 5: l = 42; break;\
		case 6: l = 50; break;\
		case 7: l = 51; break;\
		default:\
		  l = 40;\
		  break;\
	}\
} while (0)

static void DialogUpdate(HWND hwndDlg, CodecInst* pinst)
{
	if(firstInit)
	{
		firstInit = false;
		hwndToolTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			hwndDlg, NULL, hmoduleVFW, NULL);

		
		ZeroMemory(&ti, sizeof(ti));
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_SUBCLASS|TTF_IDISHWND;
		ti.hwnd = hwndDlg;

		SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0, 500);
		SendMessage(hwndToolTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 8000);
	}

	wchar_t temp[1024];
	if (SendMessage(GetDlgItem(hwndDlg, IDC_RC_MODE), CB_GETCOUNT, 0, 0) == 0)
	{
		SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_ADDSTRING, 0, (LPARAM)L"Mode 0: Fixed QP");
		//SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_ADDSTRING, 0, (LPARAM)L"1: Something 1");
		//SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_ADDSTRING, 0, (LPARAM)L"2: Something 2");
		SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_ADDSTRING, 0, (LPARAM)L"Mode 3: CBR");
		SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_ADDSTRING, 0, (LPARAM)L"Mode 4: VBR");
		//SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_ADDSTRING, 0, (LPARAM)"5: Reserved");
	}

	if (pinst->mDevList.size()>0 && SendMessage(GetDlgItem(hwndDlg, IDC_DEVICE_CB), CB_GETCOUNT, 0, 0) == 0)
	{
		DeviceMap::iterator it = pinst->mDevList.begin();
		DeviceMap::iterator ite = pinst->mDevList.end();
		for(; it!=ite; it++) {
			swprintf_s(temp, L"%d. %s", std::distance(pinst->mDevList.begin(), it), it->second.c_str());
			SendDlgItemMessageW(hwndDlg, IDC_DEVICE_CB, CB_ADDSTRING, 0, (LPARAM)temp);
		}

		pinst->mConfigTable["UseDevice"] = MIN(pinst->mConfigTable["UseDevice"], pinst->mDevList.size());
		SendDlgItemMessage(hwndDlg, IDC_DEVICE_CB, CB_SETCURSEL, pinst->mConfigTable["UseDevice"], 0);
	}

	//FIXME
	if (SendMessage(GetDlgItem(hwndDlg, IDC_LEVEL_CB), CB_GETCOUNT, 0, 0) == 0)
	{
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_ADDSTRING, 0, (LPARAM)L"3.0");
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_ADDSTRING, 0, (LPARAM)L"3.1");
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_ADDSTRING, 0, (LPARAM)L"3.2");
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_ADDSTRING, 0, (LPARAM)L"4.0");
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_ADDSTRING, 0, (LPARAM)L"4.1");
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_ADDSTRING, 0, (LPARAM)L"4.2");
		//TODO Either new drivers support it or just hawaii or newer cards
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_ADDSTRING, 0, (LPARAM)L"5.0 (hawaii+)");
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_ADDSTRING, 0, (LPARAM)L"5.1 (hawaii+)");
		uint32 level = pinst->mConfigTable["level"];
		LevelToIdx(level);
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_SETCURSEL, level, 0);
	}

	/// H264 Profiles
	CheckDlgButton(hwndDlg, IDC_PROF_BASE, 0);
	CheckDlgButton(hwndDlg, IDC_PROF_MAIN, 0);
	CheckDlgButton(hwndDlg, IDC_PROF_HIGH, 0);

	if(pinst->mConfigTable["profile"] == 77)
		CheckDlgButton(hwndDlg, IDC_PROF_MAIN, 1);
	else if(pinst->mConfigTable["profile"] == 100)
		CheckDlgButton(hwndDlg, IDC_PROF_HIGH, 1); //Might not be supported by VCE
	else
		CheckDlgButton(hwndDlg, IDC_PROF_BASE, 1);
	
	switch(pinst->mConfigTable["encRateControlMethod"])
	{
	case 0: //Fixed QP
		SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_SETCURSEL, 0, 0);
		SetDlgItemText(hwndDlg, IDC_RC_LABEL, L"Quantizer");
		SetDlgItemText(hwndDlg, IDC_RC_LOW_LABEL, L"0 (High quality)");
		swprintf(temp, 1023, L"(Low quality) %d", MAX_QUANT);
		SetDlgItemText(hwndDlg, IDC_RC_HIGH_LABEL, temp);
		SetDlgItemInt(hwndDlg, IDC_RC_VAL, pinst->mConfigTable["encQP_I"], FALSE);
		SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMIN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMAX, TRUE, MAX_QUANT);
		SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, pinst->mConfigTable["encQP_I"]);
		break;
	case 3:// 3: CBR
		SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_SETCURSEL, 1, 0);
		SetDlgItemText(hwndDlg, IDC_RC_LABEL, L"Constant bitrate (kbit/s)");
		SetDlgItemText(hwndDlg, IDC_RC_LOW_LABEL, L"1");
		swprintf(temp, 1023, L"%d", MAX_BITRATE);
		SetDlgItemText(hwndDlg, IDC_RC_HIGH_LABEL, temp);
		SetDlgItemInt(hwndDlg, IDC_RC_VAL, pinst->mConfigTable["encRateControlTargetBitRate"] / 1000, FALSE);
		SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMIN, TRUE, 1);
		//SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMAX, TRUE, scale2pos(MAX_BITRATE));
		SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMAX, TRUE, MAX_BITRATE);
		//SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, scale2pos(pinst->mConfigTable["encRateControlTargetBitRate"] / 1000));
		SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, pinst->mConfigTable["encRateControlTargetBitRate"] / 1000);
		break;

	case 4: // VBR
		SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_SETCURSEL, 2, 0);
		SetDlgItemText(hwndDlg, IDC_RC_LABEL, L"Variable bitrate (kbit/s)");
		SetDlgItemText(hwndDlg, IDC_RC_LOW_LABEL, L"1");
		wsprintf(temp, L"%d", MAX_BITRATE);
		SetDlgItemText(hwndDlg, IDC_RC_HIGH_LABEL, temp);
		SetDlgItemInt(hwndDlg, IDC_RC_VAL, pinst->mConfigTable["encRateControlTargetBitRate"] / 1000, FALSE);
		SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMIN, TRUE, 1);
		SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMAX, TRUE, MAX_BITRATE);
		SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, pinst->mConfigTable["encRateControlTargetBitRate"] / 1000);
		break;
	}

	swprintf(temp, 1023, L"%d", pinst->mConfigTable["encSearchRangeX"]);
	SetDlgItemText(hwndDlg, IDC_SEARCHRX, temp);

	swprintf(temp, 1023, L"%d", pinst->mConfigTable["encSearchRangeY"]);
	SetDlgItemText(hwndDlg, IDC_SEARCHRY, temp);

	swprintf(temp, 1023, L"%d", pinst->mConfigTable["encGOPSize"]);
	SetDlgItemText(hwndDlg, IDC_GOP, temp);
	SETTOOLTIP(IDC_GOP, "Set GOP size. 0 for automatic.");

	CheckDlgButton(hwndDlg, IDC_CABAC, pinst->mConfigTable["CABACEnable"]);
	CheckDlgButton(hwndDlg, IDC_USE_OPENCL, pinst->mUseCLConv ? 1 : 0);
	CheckDlgButton(hwndDlg, IDC_USE_OPENCL2, pinst->mUseCPU ? 1 : 0);
	CheckDlgButton(hwndDlg, IDC_USE_OPENCL3, pinst->mConfigTable["ProfileKernels"] ? 1 : 0);
	CheckDlgButton(hwndDlg, IDC_USE_ME_AMD, pinst->mConfigTable["enableAMD"]);
	CheckDlgButton(hwndDlg, IDC_SKIP_MV16, pinst->mConfigTable["encForce16x16skip"]);
	CheckDlgButton(hwndDlg, IDC_FRAMERATE, pinst->mConfigTable["sendFPS"]);
	CheckDlgButton(hwndDlg, IDC_YV12ASNV12, pinst->mConfigTable["YV12AsNV12"]);
	CheckDlgButton(hwndDlg, IDC_SPEEDY_MATH, pinst->mConfigTable["SpeedyMath"]);
	SETTOOLTIP(IDC_SPEEDY_MATH, "Use less precise floating-point math.");
	CheckDlgButton(hwndDlg, IDC_CS_RGBA, pinst->mConfigTable["SwitchByteOrder"]);
	CheckDlgButton(hwndDlg, IDC_LOG, pinst->mConfigTable["Log"]);
	CheckDlgButton(hwndDlg, IDC_CROPH, pinst->mConfigTable["crop"]);
	CheckDlgButton(hwndDlg, IDC_HDRINSERTION, pinst->mConfigTable["encHeaderInsertionSpacing"]);
	swprintf(temp, 1023, L"%d", pinst->mConfigTable["IDRframes"]);
	SetDlgItemText(hwndDlg, IDC_IDR, temp);

	swprintf(temp, 1023, L"Build date: %S %S", __DATE__, __TIME__);
	SetDlgItemText(hwndDlg, IDC_BUILD_DATE, temp);
}

static BOOL CALLBACK ConfigureDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	CodecInst *pinst = (CodecInst *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	if (uMsg == WM_INITDIALOG) {
		CodecInst *pinst = (CodecInst *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		pinst->mDialogUpdated = false;
		DialogUpdate(hwndDlg, (CodecInst*)lParam);
		pinst->mDialogUpdated = true;
		return TRUE;
	}  else if ( uMsg == WM_CLOSE ){
		EndDialog(hwndDlg, 0);
	}
	
	if(!(pinst && pinst->mDialogUpdated)) return FALSE;
	
	uint32 rate = 0, qp = 0;

	switch (uMsg){
	
	case WM_COMMAND:
		
		switch (HIWORD(wParam))
		{
			case LBN_SELCHANGE:
				switch (LOWORD(wParam))
				{
					case IDC_RC_MODE:
						pinst->mConfigTable["encRateControlMethod"] = SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_GETCURSEL, 0, 0);
						RCFromCB(pinst->mConfigTable["encRateControlMethod"]);
						
						pinst->mDialogUpdated = false;
						DialogUpdate(hwndDlg, pinst);
						pinst->mDialogUpdated = true;

						/* Ugly hack for fixing visualization bug of IDC_RC_VAL_SLIDER */
						//ShowWindow(GetDlgItem(hwndDlg, IDC_RC_VAL_SLIDER), FALSE);
						//ShowWindow(GetDlgItem(hwndDlg, IDC_RC_VAL_SLIDER), 1);

						break;

					case IDC_LEVEL_CB:
						pinst->mConfigTable["level"] = SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_GETCURSEL, 0, 0);
						IdxToLevel(pinst->mConfigTable["level"]);

						break;
					case IDC_DEVICE_CB:
						pinst->mConfigTable["UseDevice"] = SendDlgItemMessage(hwndDlg, IDC_DEVICE_CB, CB_GETCURSEL, 0, 0);

						break;

					default:
						return FALSE;
				}
				break;

			case BN_CLICKED:
				switch (LOWORD(wParam))
				{
				case IDC_OK:
					pinst->saveRegistry();
					EndDialog(hwndDlg, LOWORD(wParam));
					break;
				case IDC_CANCEL:
					pinst->readRegistry();
					EndDialog(hwndDlg, LOWORD(wParam));
					break;
				case IDC_CABAC:
					pinst->mConfigTable["CABACEnable"] = IsDlgButtonChecked(hwndDlg, IDC_CABAC);
					break;
				case IDC_USE_OPENCL:
					pinst->mUseCLConv = IsDlgButtonChecked(hwndDlg, IDC_USE_OPENCL) == 1;
					break;
				case IDC_USE_OPENCL2:
					pinst->mUseCPU = IsDlgButtonChecked(hwndDlg, IDC_USE_OPENCL2) == 1;
					break;
				case IDC_USE_OPENCL3:
					pinst->mConfigTable["ProfileKernels"] = IsDlgButtonChecked(hwndDlg, IDC_USE_OPENCL3);
					break;
				case IDC_USE_ME_AMD:
					pinst->mConfigTable["enableAMD"] = IsDlgButtonChecked(hwndDlg, IDC_USE_ME_AMD);
					break;
				case IDC_SKIP_MV16:
					pinst->mConfigTable["encForce16x16skip"] = IsDlgButtonChecked(hwndDlg, IDC_SKIP_MV16);
					break;
				case IDC_FRAMERATE:
					pinst->mConfigTable["sendFPS"] = IsDlgButtonChecked(hwndDlg, IDC_FRAMERATE);
					break;
				case IDC_YV12ASNV12:
					pinst->mConfigTable["YV12AsNV12"] = IsDlgButtonChecked(hwndDlg, IDC_YV12ASNV12);
					break;
				case IDC_SPEEDY_MATH:
					pinst->mConfigTable["SpeedyMath"] = IsDlgButtonChecked(hwndDlg, IDC_SPEEDY_MATH);
					break;
				case IDC_CS_RGBA:
					pinst->mConfigTable["SwitchByteOrder"] = IsDlgButtonChecked(hwndDlg, IDC_CS_RGBA);
					break;
				case IDC_LOG:
					pinst->mConfigTable["Log"] = IsDlgButtonChecked(hwndDlg, IDC_LOG);
					break;
				case IDC_CROPH:
					pinst->mConfigTable["crop"] = IsDlgButtonChecked(hwndDlg, IDC_CROPH);
					break;
				case IDC_HDRINSERTION:
					pinst->mConfigTable["encHeaderInsertionSpacing"] = IsDlgButtonChecked(hwndDlg, IDC_HDRINSERTION) == BST_CHECKED ? 1 : 0;
					break;
				case IDC_LOG2:
					pinst->mConfigTable["LogMsgBox"] = IsDlgButtonChecked(hwndDlg, IDC_LOG2);
					break;
				case IDC_PROF_BASE:
					//TODO fix radio buttons in group control, how do to do in plain win32?
					pinst->mConfigTable["profile"] = (IsDlgButtonChecked(hwndDlg, IDC_PROF_BASE) == 1 ? 66 : 77);
					pinst->mDialogUpdated = false;
					CheckDlgButton(hwndDlg, IDC_PROF_BASE, 1);
					CheckDlgButton(hwndDlg, IDC_PROF_MAIN, 0);
					CheckDlgButton(hwndDlg, IDC_PROF_HIGH, 0);
					pinst->mDialogUpdated = true;
					break;
				case IDC_PROF_MAIN:
					pinst->mConfigTable["profile"] = (IsDlgButtonChecked(hwndDlg, IDC_PROF_MAIN) == 1 ? 77 : 66);
					pinst->mDialogUpdated = false;
					CheckDlgButton(hwndDlg, IDC_PROF_BASE, 0);
					CheckDlgButton(hwndDlg, IDC_PROF_MAIN, 1);
					CheckDlgButton(hwndDlg, IDC_PROF_HIGH, 0);
					pinst->mDialogUpdated = true;
					break;
				case IDC_PROF_HIGH:
					pinst->mConfigTable["profile"] = (IsDlgButtonChecked(hwndDlg, IDC_PROF_HIGH) == 1 ? 100 : 66);
					pinst->mDialogUpdated = false;
					CheckDlgButton(hwndDlg, IDC_PROF_BASE, 0);
					CheckDlgButton(hwndDlg, IDC_PROF_MAIN, 0);
					CheckDlgButton(hwndDlg, IDC_PROF_HIGH, 1);
					pinst->mDialogUpdated = true;
					break;
				case IDC_QS_SPEED:
					pinst->quickSet(0);
					DialogUpdate(hwndDlg, pinst);
					break;
				case IDC_QS_BALANCED:
					pinst->quickSet(1);
					DialogUpdate(hwndDlg, pinst);
					break;
				case IDC_QS_QUALITY:
					pinst->quickSet(2);
					DialogUpdate(hwndDlg, pinst);
					break;
				}
				break;

			case EN_CHANGE:
				switch (LOWORD(wParam))
				{
					case IDC_IDR:
						CHECKED_SET_MAX_INT(rate, hwndDlg, IDC_IDR, FALSE, 1, 0xFFFFFFFF);//TODO max
						pinst->mConfigTable["IDRframes"] = rate;
						break;
					case IDC_GOP:
						CHECKED_SET_MAX_INT(rate, hwndDlg, IDC_GOP, FALSE, 0, 0xFFFFFFFF);//TODO max
						pinst->mConfigTable["encGOPSize"] = rate;
						break;
					case IDC_SEARCHRX:
						CHECKED_SET_MAX_INT(rate, hwndDlg, IDC_SEARCHRX, FALSE, 0, 36);
						pinst->mConfigTable["encSearchRangeX"] = rate;
						break;
					case IDC_SEARCHRY:
						CHECKED_SET_MAX_INT(rate, hwndDlg, IDC_SEARCHRY, FALSE, 0, 36);
						pinst->mConfigTable["encSearchRangeY"] = rate;
						break;
					case IDC_RC_VAL:
						switch(pinst->mConfigTable["encRateControlMethod"])
						{
						case 0:
							CHECKED_SET_MAX_INT(qp, hwndDlg, IDC_RC_VAL, FALSE, 0, MAX_QUANT);
							SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, qp);
							pinst->mConfigTable["encQP_I"] = pinst->mConfigTable["encQP_P"] = qp;
							break;
						case 3:
						case 4:
							CHECKED_SET_MAX_INT(rate, hwndDlg, IDC_RC_VAL, FALSE, 1, MAX_BITRATE);
							SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, rate);
							pinst->mConfigTable["encRateControlTargetBitRate"] = rate * 1000;
							break;
						}
						break;
				}
				break;
		}
		case WM_HSCROLL:
			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_RC_VAL_SLIDER))
			{
				switch (pinst->mConfigTable["encRateControlMethod"])
				{
					case 0:
						pinst->mConfigTable["encQP_I"] = pinst->mConfigTable["encQP_P"] = 
							SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_GETPOS, 0, 0);
						SetDlgItemInt(hwndDlg, IDC_RC_VAL, pinst->mConfigTable["encQP_I"], FALSE);
						break;

					case 3:
					case 4:
						//rate = pos2scale(SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_GETPOS, 0, 0));
						rate = SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_GETPOS, 0, 0);
						rate = CLIP(rate, 1, MAX_BITRATE);
						pinst->mConfigTable["encRateControlTargetBitRate"] = rate * 1000;
						SetDlgItemInt(hwndDlg, IDC_RC_VAL, rate, FALSE);
						break;

					default:
						assert(0);
						break;
				}
			}
			else
				return FALSE;
			break;
	}
	return 0;
}

BOOL CodecInst::QueryConfigure() {
	return TRUE; 
}

DWORD CodecInst::Configure(HWND hwnd) {
	DialogBoxParam(hmoduleVFW, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, (DLGPROC)ConfigureDialogProc, (LPARAM)this);
	return ICERR_OK;
}
