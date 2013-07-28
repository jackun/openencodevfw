#include "stdafx.h"
#include "OpenEncodeVFW.h"
#include <CommCtrl.h>

extern HMODULE hmoduleVFW;

//FIXME Proper maximum, 100 Mbps with 4.1+ level, probably
#define MAX_BITRATE 50000 //kbit/s
#define MAX_QUANT 51 //0...51 for I/P, (B frames are unsupported)

void CodecInst::prepareConfigMap()
{
	//Custom app settings
	mConfigTable.insert(pair<wstring,int32>(L"sendFPS", 0)); //Send proper video fps to encoder. Not sending allows video conversion with weird framerates
	mConfigTable.insert(pair<wstring,int32>(L"blend", 0)); // Blend to frames, output at half framerate (ok, but how to? :D)(you may need to fix avi header)

	/**************************************************************************/
	/* EncodeSpecifications                                                   */  
	/**************************************************************************/
	mConfigTable.insert(pair<wstring,int32>(L"pictureHeight", 144)); 
	mConfigTable.insert(pair<wstring,int32>(L"pictureWidth", 176)); 
	mConfigTable.insert(pair<wstring,int32>(L"EncodeMode", 1)); 
	mConfigTable.insert(pair<wstring,int32>(L"level", 41)); 
	mConfigTable.insert(pair<wstring,int32>(L"profile", 77)); // 66 -base, 77 - main and maybe 100 - high
	mConfigTable.insert(pair<wstring,int32>(L"pictureFormat", 1)); // 1 - NV12
	mConfigTable.insert(pair<wstring,int32>(L"requestedPriority", 1)); 
	
	/**************************************************************************/
	/* ConfigPicCtl                                                           */ 
	/**************************************************************************/
	mConfigTable.insert(pair<wstring,int32>(L"useConstrainedIntraPred", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"CABACEnable", 1)); 
	mConfigTable.insert(pair<wstring,int32>(L"CABACIDC", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"loopFilterDisable", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encLFBetaOffset", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encLFAlphaC0Offset", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encIDRPeriod", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encIPicPeriod", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encHeaderInsertionSpacing", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encCropLeftOffset", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encCropRightOffset", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encCropTopOffset", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encCropBottomOffset", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encNumMBsPerSlice", 99)); 
	mConfigTable.insert(pair<wstring,int32>(L"encNumSlicesPerFrame", 1)); 
	mConfigTable.insert(pair<wstring,int32>(L"encForceIntraRefresh", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encForceIMBPeriod", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encInsertVUIParam", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encInsertSEIMsg", 0)); 
	
	/**************************************************************************/
	/* ConfigRateCtl                                                          */
	/**************************************************************************/
	mConfigTable.insert(pair<wstring,int32>(L"encRateControlMethod", 4)); 
	mConfigTable.insert(pair<wstring,int32>(L"encRateControlTargetBitRate", 8000000)); 
	mConfigTable.insert(pair<wstring,int32>(L"encRateControlPeakBitRate", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encRateControlFrameRateNumerator", 30)); 
	mConfigTable.insert(pair<wstring,int32>(L"encRateControlFrameRateDenominator", 1)); 
	mConfigTable.insert(pair<wstring,int32>(L"encGOPSize", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encRCOptions", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encQP_I", 22)); 
	mConfigTable.insert(pair<wstring,int32>(L"encQP_P", 22)); 
	mConfigTable.insert(pair<wstring,int32>(L"encQP_B", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encVBVBufferSize", mConfigTable[L"encRateControlTargetBitRate"] / 2)); //4000000));

	/**************************************************************************/
	/* ConfigMotionEstimation				                                  */
	/**************************************************************************/
	mConfigTable.insert(pair<wstring,int32>(L"IMEDecimationSearch", 1)); 
	mConfigTable.insert(pair<wstring,int32>(L"motionEstHalfPixel", 1)); 
	mConfigTable.insert(pair<wstring,int32>(L"motionEstQuarterPixel", 1)); 
	mConfigTable.insert(pair<wstring,int32>(L"disableFavorPMVPoint", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"forceZeroPointCenter", 1)); 
	mConfigTable.insert(pair<wstring,int32>(L"LSMVert", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encSearchRangeX", 16)); 
	mConfigTable.insert(pair<wstring,int32>(L"encSearchRangeY", 16)); 
	mConfigTable.insert(pair<wstring,int32>(L"encSearch1RangeX", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encSearch1RangeY", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"disable16x16Frame1", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"disableSATD", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"enableAMD", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encDisableSubMode", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encIMESkipX", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encIMESkipY", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encEnImeOverwDisSubm", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encImeOverwDisSubmNo", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encIME2SearchRangeX", 4)); 
	mConfigTable.insert(pair<wstring,int32>(L"encIME2SearchRangeY", 4)); 

	/**************************************************************************/
	/* ConfigRDO                                                              */
	/**************************************************************************/
	mConfigTable.insert(pair<wstring,int32>(L"encDisableTbePredIFrame", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encDisableTbePredPFrame", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"useFmeInterpolY", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"useFmeInterpolUV", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"enc16x16CostAdj", 0)); 
	mConfigTable.insert(pair<wstring,int32>(L"encSkipCostAdj", 0)); 
    mConfigTable.insert(pair<wstring,int32>(L"encForce16x16skip", 0)); 

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
	std::map<std::wstring, int32>::iterator it = mConfigTable.begin();
    for(; it != mConfigTable.end(); it++)
    {
        i_size = sizeof(int32);
		if (RegQueryValueEx(hKey, it->first.c_str(), 0, 0, (LPBYTE)&i, &i_size) == ERROR_SUCCESS)
			it->second = i;
    }

	i_size = sizeof(int32);
	if (RegQueryValueEx(hKey, L"useCLConversion", 0, 0, (LPBYTE)&i, &i_size) == ERROR_SUCCESS)
		mUseCLConv = (i == 1);

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
    std::map<std::wstring, int32>::iterator it = mConfigTable.begin();
    for(; it != mConfigTable.end(); it++)
		s = RegSetValueEx(hKey, it->first.c_str(), 0, REG_DWORD, (LPBYTE)&it->second, sizeof(int32));

	i = mUseCLConv ? 1 : 0;
	RegSetValueEx(hKey, L"useCLConversion", 0, REG_DWORD, (LPBYTE)&i, sizeof(int));

    //RegSetValueEx(hKey, L"StringKey", 0, REG_SZ, (LPBYTE)L"Value", wcslen(L"Value") + 1);
    LeaveCriticalSection(&ove_CS);
    RegCloseKey(hKey);
	return true;
}

bool CodecInst::readConfigFile(int8 *fileName, OvConfigCtrl *pConfig,
                    std::map<std::wstring,int32>* pConfigTable)
{
    wchar_t name[1000];
    int32 index;
    int32 value;

    std::wifstream file;
    file.open(fileName);
	
    if (!file)
    {
        printf("Error in reading the configuration file: %s\n", fileName);
		wchar_t msg[MAX_PATH + 128];
		swprintf_s(msg, MAX_PATH + 128, L"Error in reading the configuration file: %s", fileName);
		MessageBox (HWND_DESKTOP, msg, L"Error", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    std::wstring line;
	map<wstring, int32>::iterator itMap;
	
    while (std::getline(file, line))
    {
        std::wstring temp = line;
		index = 0;
   	    swscanf_s(line.c_str(), L"%s %d", name, &value);
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
void CodecInst::encodeSetParam(OvConfigCtrl *pConfig, map<wstring,int32>* pConfigTable)
{

	/**************************************************************************/
    /* fill-in the general configuration structures                           */
	/**************************************************************************/
	map<wstring,int32> configTable = (map<wstring,int32>)*pConfigTable;
	pConfig->height									= configTable[L"pictureHeight"];
	pConfig->width									= configTable[L"pictureWidth"];
	pConfig->encodeMode								= (OVE_ENCODE_MODE) configTable[L"EncodeMode"];

	/**************************************************************************/
	/* fill-in the profile and level                                          */
	/**************************************************************************/
	pConfig->profileLevel.level						= configTable[L"level"];
	pConfig->profileLevel.profile					= configTable[L"profile"];

	pConfig->pictFormat								= (OVE_PICTURE_FORMAT)configTable[L"pictureFormat"];
	pConfig->priority								= (OVE_ENCODE_TASK_PRIORITY)configTable[L"requestedPriority"];

	/**************************************************************************/
	/* fill-in the picture control structures                                 */
	/**************************************************************************/
	pConfig->pictControl.size						= sizeof(OVE_CONFIG_PICTURE_CONTROL);
	pConfig->pictControl.useConstrainedIntraPred	= configTable[L"useConstrainedIntraPred"];
	pConfig->pictControl.cabacEnable				= configTable[L"CABACEnable"];
	pConfig->pictControl.cabacIDC					= configTable[L"CABACIDC"];
	pConfig->pictControl.loopFilterDisable			= configTable[L"loopFilterDisable"];
	pConfig->pictControl.encLFBetaOffset			= configTable[L"encLFBetaOffset"];
	pConfig->pictControl.encLFAlphaC0Offset			= configTable[L"encLFAlphaC0Offset"];
	pConfig->pictControl.encIDRPeriod				= configTable[L"encIDRPeriod"];
	pConfig->pictControl.encIPicPeriod				= configTable[L"encIPicPeriod"];
	pConfig->pictControl.encHeaderInsertionSpacing	= configTable[L"encHeaderInsertionSpacing"];
	pConfig->pictControl.encCropLeftOffset			= configTable[L"encCropLeftOffset"];
	pConfig->pictControl.encCropRightOffset			= configTable[L"encCropRightOffset"];
	pConfig->pictControl.encCropTopOffset			= configTable[L"encCropTopOffset"];
	pConfig->pictControl.encCropBottomOffset		= configTable[L"encCropBottomOffset"];
	pConfig->pictControl.encNumMBsPerSlice			= configTable[L"encNumMBsPerSlice"];
	pConfig->pictControl.encNumSlicesPerFrame		= configTable[L"encNumSlicesPerFrame"];
	pConfig->pictControl.encForceIntraRefresh		= configTable[L"encForceIntraRefresh"];
	pConfig->pictControl.encForceIMBPeriod			= configTable[L"encForceIMBPeriod"];
	pConfig->pictControl.encInsertVUIParam			= configTable[L"encInsertVUIParam"];
	pConfig->pictControl.encInsertSEIMsg			= configTable[L"encInsertSEIMsg"];

	/**************************************************************************/
	/* fill-in the rate control structures                                    */
	/**************************************************************************/
	pConfig->rateControl.size							= sizeof(OVE_CONFIG_RATE_CONTROL);
	pConfig->rateControl.encRateControlMethod			= configTable[L"encRateControlMethod"];
	pConfig->rateControl.encRateControlTargetBitRate	= configTable[L"encRateControlTargetBitRate"];
	pConfig->rateControl.encRateControlPeakBitRate		= configTable[L"encRateControlPeakBitRate"];
	pConfig->rateControl.encRateControlFrameRateNumerator = configTable[L"encRateControlFrameRateNumerator"];
	pConfig->rateControl.encGOPSize						= configTable[L"encGOPSize"];
	pConfig->rateControl.encRCOptions					= configTable[L"encRCOptions"];
	pConfig->rateControl.encQP_I						= configTable[L"encQP_I"];
	pConfig->rateControl.encQP_P						= configTable[L"encQP_P"];
	pConfig->rateControl.encQP_B						= configTable[L"encQP_B"];
	pConfig->rateControl.encVBVBufferSize				= configTable[L"encVBVBufferSize"];
	pConfig->rateControl.encRateControlFrameRateDenominator = configTable[L"encRateControlFrameRateDenominator"];

	/**************************************************************************/
	/* fill-in the motion estimation control structures                       */
	/**************************************************************************/
	pConfig->meControl.size							= sizeof(OVE_CONFIG_MOTION_ESTIMATION);
	pConfig->meControl.imeDecimationSearch			= configTable[L"IMEDecimationSearch"];
	pConfig->meControl.motionEstHalfPixel			= configTable[L"motionEstHalfPixel"];
	pConfig->meControl.motionEstQuarterPixel		= configTable[L"motionEstQuarterPixel"];
	pConfig->meControl.disableFavorPMVPoint			= configTable[L"disableFavorPMVPoint"];
	pConfig->meControl.forceZeroPointCenter			= configTable[L"forceZeroPointCenter"];
	pConfig->meControl.lsmVert						= configTable[L"LSMVert"];
	pConfig->meControl.encSearchRangeX				= configTable[L"encSearchRangeX"];
	pConfig->meControl.encSearchRangeY				= configTable[L"encSearchRangeY"];
	pConfig->meControl.encSearch1RangeX				= configTable[L"encSearch1RangeX"];
	pConfig->meControl.encSearch1RangeY				= configTable[L"encSearch1RangeY"];
	pConfig->meControl.disable16x16Frame1			= configTable[L"disable16x16Frame1"];
	pConfig->meControl.disableSATD					= configTable[L"disableSATD"];
	pConfig->meControl.enableAMD					= configTable[L"enableAMD"];
	pConfig->meControl.encDisableSubMode			= configTable[L"encDisableSubMode"];
	pConfig->meControl.encIMESkipX					= configTable[L"encIMESkipX"];
	pConfig->meControl.encIMESkipY					= configTable[L"encIMESkipY"];
	pConfig->meControl.encEnImeOverwDisSubm			= configTable[L"encEnImeOverwDisSubm"];
	pConfig->meControl.encImeOverwDisSubmNo			= configTable[L"encImeOverwDisSubmNo"];
	pConfig->meControl.encIME2SearchRangeX			= configTable[L"encIME2SearchRangeX"];
	pConfig->meControl.encIME2SearchRangeY			= configTable[L"encIME2SearchRangeY"];

	/**************************************************************************/
	/* fill-in the RDO control structures                                     */
	/**************************************************************************/
	pConfig->rdoControl.size                        = sizeof(OVE_CONFIG_RDO);
	pConfig->rdoControl.encDisableTbePredIFrame		= configTable[L"encDisableTbePredIFrame"];
	pConfig->rdoControl.encDisableTbePredPFrame		= configTable[L"encDisableTbePredPFrame"];
	pConfig->rdoControl.useFmeInterpolY				= configTable[L"useFmeInterpolY"];
	pConfig->rdoControl.useFmeInterpolUV			= configTable[L"useFmeInterpolUV"];
	pConfig->rdoControl.enc16x16CostAdj				= configTable[L"enc16x16CostAdj"];
	pConfig->rdoControl.encSkipCostAdj				= configTable[L"encSkipCostAdj"];
	pConfig->rdoControl.encForce16x16skip			= (uint8)configTable[L"encForce16x16skip"];
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
        Log(L"OVEncodeSendConfig returned error\n");
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
        case 0:\
          rc = 0;\
          break;\
        case 1:\
          rc = 3;\
          break;\
        case 2:\
          rc = 4;\
          break;\
        default:\
          assert(0);\
          break;\
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
		for(; it!=ite; it++)
			SendDlgItemMessage(hwndDlg, IDC_DEVICE_CB, CB_ADDSTRING, 0, (LPARAM)it->second.c_str());

		SendDlgItemMessage(hwndDlg, IDC_DEVICE_CB, CB_SETCURSEL, 0, 0);
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
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_ADDSTRING, 0, (LPARAM)L"5.0 (probably not)");
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_ADDSTRING, 0, (LPARAM)L"5.1 (probably not)");
		uint32 level = pinst->mConfigTable[L"level"];
		LevelToIdx(level);
		SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_SETCURSEL, level, 0);
	}

	/// H264 Profiles
	CheckDlgButton(hwndDlg, IDC_PROF_BASE, 0);
	CheckDlgButton(hwndDlg, IDC_PROF_MAIN, 0);
	CheckDlgButton(hwndDlg, IDC_PROF_HIGH, 0);

	if(pinst->mConfigTable[L"profile"] == 77)
		CheckDlgButton(hwndDlg, IDC_PROF_MAIN, 1);
	else if(pinst->mConfigTable[L"profile"] == 100)
		CheckDlgButton(hwndDlg, IDC_PROF_HIGH, 1); //Might not be supported by VCE
	else
		CheckDlgButton(hwndDlg, IDC_PROF_BASE, 1);
	
	switch(pinst->mConfigTable[L"encRateControlMethod"])
	{
	case 0: //Fixed QP
        SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_SETCURSEL, 0, 0);
        SetDlgItemText(hwndDlg, IDC_RC_LABEL, L"Quantizer");
        SetDlgItemText(hwndDlg, IDC_RC_LOW_LABEL, L"0 (High quality)");
        swprintf(temp, 1023, L"(Low quality) %d", MAX_QUANT);
        SetDlgItemText(hwndDlg, IDC_RC_HIGH_LABEL, temp);
        SetDlgItemInt(hwndDlg, IDC_RC_VAL, pinst->mConfigTable[L"encQP_I"], FALSE);
        SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMIN, TRUE, 0);
        SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMAX, TRUE, MAX_QUANT);
        SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, pinst->mConfigTable[L"encQP_I"]);
        break;
	case 3:// 3: CBR
		SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_SETCURSEL, 1, 0);
		SetDlgItemText(hwndDlg, IDC_RC_LABEL, L"Constant bitrate (kbit/s)");
        SetDlgItemText(hwndDlg, IDC_RC_LOW_LABEL, L"1");
        swprintf(temp, 1023, L"%d", MAX_BITRATE);
        SetDlgItemText(hwndDlg, IDC_RC_HIGH_LABEL, temp);
        SetDlgItemInt(hwndDlg, IDC_RC_VAL, pinst->mConfigTable[L"encRateControlTargetBitRate"] / 1000, FALSE);
        SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMIN, TRUE, 1);
        //SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMAX, TRUE, scale2pos(MAX_BITRATE));
		SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMAX, TRUE, MAX_BITRATE);
        //SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, scale2pos(pinst->mConfigTable[L"encRateControlTargetBitRate"] / 1000));
		SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, pinst->mConfigTable[L"encRateControlTargetBitRate"] / 1000);
		break;

	case 4: // VBR
		SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_SETCURSEL, 2, 0);
		SetDlgItemText(hwndDlg, IDC_RC_LABEL, L"Variable bitrate (kbit/s)");
        SetDlgItemText(hwndDlg, IDC_RC_LOW_LABEL, L"1");
        wsprintf(temp, L"%d", MAX_BITRATE);
        SetDlgItemText(hwndDlg, IDC_RC_HIGH_LABEL, temp);
        SetDlgItemInt(hwndDlg, IDC_RC_VAL, pinst->mConfigTable[L"encRateControlTargetBitRate"] / 1000, FALSE);
        SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMIN, TRUE, 1);
        SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETRANGEMAX, TRUE, MAX_BITRATE);
        SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, pinst->mConfigTable[L"encRateControlTargetBitRate"] / 1000);
		break;
	}

	swprintf(temp, 1023, L"%d", pinst->mConfigTable[L"encSearchRangeX"]);
	SetDlgItemText(hwndDlg, IDC_SEARCHRX, temp);

	swprintf(temp, 1023, L"%d", pinst->mConfigTable[L"encSearchRangeY"]);
	SetDlgItemText(hwndDlg, IDC_SEARCHRY, temp);

	CheckDlgButton(hwndDlg, IDC_CABAC, pinst->mConfigTable[L"CABACEnable"]);
	CheckDlgButton(hwndDlg, IDC_USE_OPENCL, pinst->mUseCLConv ? 1 : 0);
	CheckDlgButton(hwndDlg, IDC_USE_ME_AMD, pinst->mConfigTable[L"enableAMD"]);
	CheckDlgButton(hwndDlg, IDC_SKIP_MV16, pinst->mConfigTable[L"encForce16x16skip"]);
	CheckDlgButton(hwndDlg, IDC_FRAMERATE, pinst->mConfigTable[L"sendFPS"]);
	CheckDlgButton(hwndDlg, IDC_BLEND, pinst->mConfigTable[L"blend"]);

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
						pinst->mConfigTable[L"encRateControlMethod"] = SendDlgItemMessage(hwndDlg, IDC_RC_MODE, CB_GETCURSEL, 0, 0);
                        RCFromCB(pinst->mConfigTable[L"encRateControlMethod"]);
                        
						pinst->mDialogUpdated = false;
						DialogUpdate(hwndDlg, pinst);
						pinst->mDialogUpdated = true;

                        /* Ugly hack for fixing visualization bug of IDC_RC_VAL_SLIDER */
                        //ShowWindow(GetDlgItem(hwndDlg, IDC_RC_VAL_SLIDER), FALSE);
                        //ShowWindow(GetDlgItem(hwndDlg, IDC_RC_VAL_SLIDER), 1);

						break;

					case IDC_LEVEL_CB:
						pinst->mConfigTable[L"level"] = SendDlgItemMessage(hwndDlg, IDC_LEVEL_CB, CB_GETCURSEL, 0, 0);
                        IdxToLevel(pinst->mConfigTable[L"level"]);

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
					pinst->mConfigTable[L"CABACEnable"] = IsDlgButtonChecked(hwndDlg, IDC_CABAC);
					break;
				case IDC_BLEND:
					pinst->mConfigTable[L"blend"] = IsDlgButtonChecked(hwndDlg, IDC_BLEND);
					break;
				case IDC_USE_OPENCL:
					pinst->mUseCLConv = IsDlgButtonChecked(hwndDlg, IDC_USE_OPENCL) == 1;
					break;
				case IDC_USE_ME_AMD:
					pinst->mConfigTable[L"enableAMD"] = IsDlgButtonChecked(hwndDlg, IDC_USE_ME_AMD);
					break;
				case IDC_SKIP_MV16:
					pinst->mConfigTable[L"encForce16x16skip"] = IsDlgButtonChecked(hwndDlg, IDC_SKIP_MV16);
					break;
				case IDC_FRAMERATE:
					pinst->mConfigTable[L"sendFPS"] = IsDlgButtonChecked(hwndDlg, IDC_FRAMERATE);
					break;
				case IDC_PROF_BASE:
					//TODO fix radio buttons in group control, how do to do in plain win32?
					pinst->mConfigTable[L"profile"] = (IsDlgButtonChecked(hwndDlg, IDC_PROF_BASE) == 1 ? 66 : 77);
					pinst->mDialogUpdated = false;
					CheckDlgButton(hwndDlg, IDC_PROF_BASE, 1);
					CheckDlgButton(hwndDlg, IDC_PROF_MAIN, 0);
					CheckDlgButton(hwndDlg, IDC_PROF_HIGH, 0);
					pinst->mDialogUpdated = true;
					break;
				case IDC_PROF_MAIN:
					pinst->mConfigTable[L"profile"] = (IsDlgButtonChecked(hwndDlg, IDC_PROF_MAIN) == 1 ? 77 : 66);
					pinst->mDialogUpdated = false;
					CheckDlgButton(hwndDlg, IDC_PROF_BASE, 0);
					CheckDlgButton(hwndDlg, IDC_PROF_MAIN, 1);
					CheckDlgButton(hwndDlg, IDC_PROF_HIGH, 0);
					pinst->mDialogUpdated = true;
					break;
				case IDC_PROF_HIGH:
					pinst->mConfigTable[L"profile"] = (IsDlgButtonChecked(hwndDlg, IDC_PROF_HIGH) == 1 ? 100 : 66);
					pinst->mDialogUpdated = false;
					CheckDlgButton(hwndDlg, IDC_PROF_BASE, 0);
					CheckDlgButton(hwndDlg, IDC_PROF_MAIN, 0);
					CheckDlgButton(hwndDlg, IDC_PROF_HIGH, 1);
					pinst->mDialogUpdated = true;
					break;
				}
				break;

			case EN_CHANGE:
                switch (LOWORD(wParam))
                {
					case IDC_SEARCHRX:
						CHECKED_SET_MAX_INT(rate, hwndDlg, IDC_SEARCHRX, FALSE, 0, 36);
						pinst->mConfigTable[L"encSearchRangeX"] = rate;
						break;
					case IDC_SEARCHRY:
						CHECKED_SET_MAX_INT(rate, hwndDlg, IDC_SEARCHRY, FALSE, 0, 36);
						pinst->mConfigTable[L"encSearchRangeY"] = rate;
						break;
                    case IDC_RC_VAL:
						switch(pinst->mConfigTable[L"encRateControlMethod"])
						{
						case 0:
                            CHECKED_SET_MAX_INT(qp, hwndDlg, IDC_RC_VAL, FALSE, 0, MAX_QUANT);
                            SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, qp);
							pinst->mConfigTable[L"encQP_I"] = pinst->mConfigTable[L"encQP_P"] = qp;
							break;
						case 3:
						case 4:
                            CHECKED_SET_MAX_INT(rate, hwndDlg, IDC_RC_VAL, FALSE, 1, MAX_BITRATE);
                            SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_SETPOS, TRUE, rate);
							pinst->mConfigTable[L"encRateControlTargetBitRate"] = rate * 1000;
                            break;
						}
						break;
				}
				break;
		}
		case WM_HSCROLL:
            if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_RC_VAL_SLIDER))
            {
                switch (pinst->mConfigTable[L"encRateControlMethod"])
                {
                    case 0:
                        pinst->mConfigTable[L"encQP_I"] = pinst->mConfigTable[L"encQP_P"] = 
							SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_GETPOS, 0, 0);
                        SetDlgItemInt(hwndDlg, IDC_RC_VAL, pinst->mConfigTable[L"encQP_I"], FALSE);
                        break;

                    case 3:
                    case 4:
                        //rate = pos2scale(SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_GETPOS, 0, 0));
						rate = SendDlgItemMessage(hwndDlg, IDC_RC_VAL_SLIDER, TBM_GETPOS, 0, 0);
                        rate = CLIP(rate, 1, MAX_BITRATE);
						pinst->mConfigTable[L"encRateControlTargetBitRate"] = rate * 1000;
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