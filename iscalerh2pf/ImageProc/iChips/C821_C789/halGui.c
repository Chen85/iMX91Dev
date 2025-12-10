#include <string.h>

//#include "halC789CtrlAPI.h"
#include "halGui.h"
#include "cJSON.h"
#include "utilDbgMsg.h"
//#include "lwip_fs.h"
//#include "ff.h"
//#include "MemMap.h"
//#include "halScaler.h"      //A70LV_Doulas_0014
#include "dvC821.h"
#include "utilCRCAPI.h"

//#define STANDARD_C_MALLOC
#define CHAR_LINE_BUF_MAX 32

/* Read a cJSON item/entity/structure to embedded. */
char JSON_Get(const cJSON *item, int iReadStatus);

//#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
//pthread_mutex_t             xGuiSemaphore;
//#else
//SemaphoreHandle_t            xGuiSemaphore;
//#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
//    StaticSemaphore_t            xGuiMutexBuffer;
//#endif
//#endif

//char *m_pcBitmapStruct = (char*)BITMAP_JSON_ADDR;
//char *m_pcTextStruct = (char*)TEXT_JSON_ADDR;
//char *m_pcOSDStruct = (char*)OSD_STRUCT_ADDR; //A70LV_Larry_0001


//	static int iBitmapTotalWidth = 0;
//	static int iBitmapTotalHeight = 0;
//static int iBitmapPaletteUsed = 768;
UINT8 aiBitmapPalette[768];


inline BOOL halGui_SemaphoreTake(const char *pcFunc)
{
    BOOL bResult = dvC821_SemaphoreTake(TRUE, pcFunc);

    return bResult;
}


inline BOOL halGui_SemaphoreGive(const char *pcFunc)
{
    BOOL bResult = dvC821_SemaphoreTake(FALSE, pcFunc);

    return bResult;
}

// ---------------------------------------------------------------------------
// color definition data strucutre
// ---------------------------------------------------------------------------


//	RGB m_asPalette[] =	//T100_Sander_0016
//	{
//	{12, 110, 180},		// index 240 Christie Logo background
//	{19, 54, 89},		// index 241 Active Menu Title background
//	{45, 90, 135},		// index 242 Active Menu Item background
//	{61, 84, 108},     	// index  243 Inactive Menu Title background
//	{160, 160, 160},    // index  244 Inactive Menu Item background
//	{33, 100, 168},     // index  245 Focus Bar
//	{255, 0, 0},     	// index  246 Red
//	{0, 255, 0},     	// index  247 Green
//	{0, 0, 255},     	// index  248 Blue
//	{255, 255, 0},     	// index  249 Yellow
//	{255, 255, 255},    // index  250 White
//	{9, 45, 82},    	// index  251 Slider Bar
//	{255, 255, 255},    // index  252 Slider Bar background
//	{252, 252, 252},    // index  253 Transparent Color 1
//	{253, 253, 253},    // index  254 Transparent Color 2
//	{254, 0, 254}     	// index  255 Inhibit color
//	};
//	#define NUM_PALETTE_TABLE_ENTRIES (sizeof(m_asPalette)/sizeof(RGB))


//	INT8 Gui_ColorPalette_Replace()
//	{
//	    INT16 iCount;
//	    INT16 iIdx;
//
//	    LOG_MSG(db_ALWAYS, "(func:%s, line:%d)replace 240~256 colors\n", __FUNCTION__, __LINE__, ulAllMenuCount, ulAllItemCount);
//	    iIdx = 0;
//	    for (iCount=240; iCount<256; iCount++)
//	    {
//	        aiBitmapPalette[iCount*3] = m_asPalette[iIdx].ucRed;
//	        aiBitmapPalette[iCount*3+1] = m_asPalette[iIdx].ucGreen;
//	        aiBitmapPalette[iCount*3+2] = m_asPalette[iIdx].ucBlue;
//	        iIdx++;
//	    }
//
//	    return GUI_PASS;
//	}


//	INT8 Gui_Data_Load(char *filename, UINT32 ulSflAd)
//	{
//	    FIL Fil;	/* File object */
//	    FRESULT rc;     /* Result code */
//
//	    UINT32 br; //bw, br, i;
//	    char *cbuf = NULL;
//
//
//	    rc = f_open(&Fil, filename, FA_READ);
//	    cbuf = (char *) pvPortMalloc(f_size(&Fil));
//
//	    LOG_MSG(db_ALWAYS, "(func:%s, line:%d) after fs_open called len=%d\n", __FUNCTION__, __LINE__, f_size(&Fil));
//
//	    rc = f_read(&Fil, (void *)cbuf, f_size(&Fil), &br);
//
//	    if(rc || !br)
//	    {
//
//	        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)Error %s f_read Fail !\n", __FUNCTION__, __LINE__, filename);
//	        f_close(&Fil);
//	        return ERROR_GUI_NOT_FOUND;
//	        /* Error or end of file */
//	    }
//
//	    f_close(&Fil);
//
//	    //LOG_MSG(db_ALWAYS, "(func:%s, line:%d) Write %s data\n", __FUNCTION__, __LINE__, filename);
//	    //call C734 SPI-flahs write fucntion
//	    dvC734_SerialFlash_Write(ulSflAd, (UINT8 *)cbuf, f_size(&Fil));
//
//	    vPortFree(cbuf);
//
//	    return GUI_PASS;
//	}


INT8 halGui_Paint_SubString( OSD_STRING_INFO sOsdString, START_POINT_INFO sDes_position, INT16 iTextColor, INT16 iStringOffset)
{
    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Des_(x,y)=(%d,%d), StrLen=%d, StringOffset=%d\n", __FUNCTION__, __LINE__, sDes_position.iX, sDes_position.iY, sOsdString.uiStringLength ,iStringOffset);
    INT8 cResult = 0;
    UINT16 uiCount = 0;
    UINT16 uiCharWdith = 0;
    START_POINT sSubDes_position ;
    OSD_STRING sSubString ;

    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        memcpy(&sSubDes_position, &sDes_position, sizeof(START_POINT_INFO));
        memcpy(&sSubString, &sOsdString, sizeof(OSD_STRING_INFO));

        for(uiCount = 0; uiCount < sOsdString.uiStringLength; uiCount = uiCount + CHAR_LINE_BUF_MAX)
        {
            sSubString.uiStringLength = ((sOsdString.uiStringLength - uiCount) > CHAR_LINE_BUF_MAX) ? CHAR_LINE_BUF_MAX : (sOsdString.uiStringLength - uiCount);

            cResult &= dvC821_Paint_String(sSubDes_position, sSubString, iTextColor, GUI_INHITBIT_COLOR, uiCount);

            if((sOsdString.uiStringLength - uiCount) > CHAR_LINE_BUF_MAX)
            {
                for(uiCharWdith = 0; uiCharWdith < sSubString.uiStringLength ; uiCharWdith++)
                {
                    sSubDes_position.iX = sSubDes_position.iX + sOsdString.pucCharWdith[uiCharWdith + uiCount];
                }
                sSubDes_position.iX = sSubDes_position.iX + CHAR_LINE_BUF_MAX;  //G100_Steven_0015 fix osd text overlap  //padding 1 pixel foreach character of iChip font drawing
            }
        }


        halGui_SemaphoreGive(__FUNCTION__);
    }

    return cResult;
}


#if (LOGO_REPLACE == 1)// R70G2_Bruce#0023
INT8 LogoPaletteFlag = -1;
INT8 ServiceLogoPaletteFlag = -1;

void halGui_LogoPaletteFlagInit(void)
{
    LogoPaletteFlag = -1;
}

void halGui_ServiceLogoPaletteFlagInit(void) //A65_OPTOMA_Julie_0076
{
    ServiceLogoPaletteFlag = -1;
}

INT8 halGui_SecondLogoHWInit(void) //T100_Optoma_Coda:
{
    if (LogoPaletteFlag == 1)
        return 0;
	if(halGui_InitSemaphore() != GUI_PASS)	   //A70LV_Doulas_0014
		return ERROR_GUI_SEMAPHORE_FAIL;

	halC789Ctrl_OutputEnableSet(0);

    UINT16 ret = 0;
    UINT8 Palette_toWrite[768] = {0};
    UINT8 Palette_2ndLogo[OSD_2ND_LOGO_PALETTE_SIZE] = {0};
    FILE *pFile = fopen(OSD_2ND_LOGO_PALETTE_FILE, "rb");
    if(pFile == NULL)
    {
        LOG_MSG(db_UPGRADE, "%s() fopen(OSD_2ND_LOGO_PALETTE_FILE) failed!\r\n", __FUNCTION__);// db_ALWAYS
        return (ERROR_GUI_EMPTY_STRING);
    }
    //Get Second Logo Palette 0 ~ 118 total 119
    ret = fread(Palette_2ndLogo, 1, OSD_2ND_LOGO_PALETTE_SIZE, pFile);
    fclose(pFile);
    if (ret != OSD_2ND_LOGO_PALETTE_SIZE)
    {
        LOG_MSG(db_UPGRADE, "%s() fread(%d) failed!\r\n", __FUNCTION__, ret);// db_ALWAYS
        return (ERROR_GUI_EMPTY_STRING);
    }

    // copy aiBitmapPalette[768] to Palette_toWrite[768]
    memcpy(Palette_toWrite, aiBitmapPalette, sizeof(Palette_toWrite));
    // copy Palette_2ndLogo[119*3] to Palette_toWrite[768]
    memcpy(Palette_toWrite, Palette_2ndLogo, OSD_2ND_LOGO_PALETTE_SIZE);

    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
    	dvC821_2ND_LOGO_OSD_Init(256, Palette_toWrite, GUI_INHITBIT_COLOR, eCM_USER_LOGO_2D); //A65_OPTOMA_Julie_0067

        halGui_SemaphoreGive(__FUNCTION__);
        LOG_MSG(db_HAL_GUI, "%s() pass.\n", __FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_GUI, "%s() fail!\n", __FUNCTION__);
    }
    LogoPaletteFlag = 1;
	ServiceLogoPaletteFlag = 0;
	halC789Ctrl_OutputEnableSet(1);
    LOG_MSG(db_UPGRADE, "%s(%d)\r\n", __FUNCTION__, LogoPaletteFlag);
    return GUI_PASS;
}

INT8 halGui_Service_SecondLogoHWInit(void) //A65_OPTOMA_Julie_0076
{
    if (ServiceLogoPaletteFlag == 1)
        return 0;
	if(halGui_InitSemaphore() != GUI_PASS)	   //A70LV_Doulas_0014
		return ERROR_GUI_SEMAPHORE_FAIL;

	halC789Ctrl_OutputEnableSet(0);

    UINT16 ret = 0;
    UINT8 Palette_toWrite[768] = {0};
    UINT8 Palette_2ndLogo[OSD_2ND_LOGO_PALETTE_SIZE] = {0};
    FILE *pFile = fopen(OSD_2ND_LOGO_SERVICE_PALETTE_FILE, "rb");
    if(pFile == NULL)
    {
        LOG_MSG(db_UPGRADE, "(%s,%d) fopen(%s) failed!\r\n", __FUNCTION__, __LINE__, OSD_2ND_LOGO_SERVICE_PALETTE_FILE);// db_ALWAYS
        return (ERROR_GUI_EMPTY_STRING);
    }
    //Get Second Logo Palette 0 ~ 118 total 119
    ret = fread(Palette_2ndLogo, 1, OSD_2ND_LOGO_PALETTE_SIZE, pFile);
    fclose(pFile);
    if (ret != OSD_2ND_LOGO_PALETTE_SIZE)
    {
        LOG_MSG(db_UPGRADE, "%s() fread(%d) failed!\r\n", __FUNCTION__, ret);// db_ALWAYS
        return (ERROR_GUI_EMPTY_STRING);
    }

    // copy aiBitmapPalette[768] to Palette_toWrite[768]
    memcpy(Palette_toWrite, aiBitmapPalette, sizeof(Palette_toWrite));
    // copy Palette_2ndLogo[119*3] to Palette_toWrite[768]
    memcpy(Palette_toWrite, Palette_2ndLogo, OSD_2ND_LOGO_PALETTE_SIZE);

    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
    	dvC821_2ND_LOGO_OSD_Init(256, Palette_toWrite, GUI_INHITBIT_COLOR, eCM_USER_LOGO_SERVICE_2D); //A65_OPTOMA_Julie_0067

        halGui_SemaphoreGive(__FUNCTION__);
        LOG_MSG(db_HAL_GUI, "%s() pass.\n", __FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_GUI, "%s() fail!\n", __FUNCTION__);
    }
    ServiceLogoPaletteFlag = 1;
	LogoPaletteFlag = 0;
	halC789Ctrl_OutputEnableSet(1);
    LOG_MSG(db_UPGRADE, "%s(%d)\r\n", __FUNCTION__, ServiceLogoPaletteFlag);
    return GUI_PASS;
}


INT8 halGui_SecondLogoPaletteStore(char *fileName, UINT8 ucIndex) //A65_OPTOMA_Julie_0076 // R70G2_Bruce#0023
{
    FILE *pFile = NULL;
    UINT8 Palette[512] = {0};
    UINT16 r = 0, w = 0;

    pFile = fopen(fileName, "rb");
    if (pFile == NULL)
    {
        LOG_MSG(db_UPGRADE, "%s() fopen(TMP_2ND_LOGO_PALETTE_FILE) failed!\r\n", __func__);
        return (ERROR_GUI_EMPTY_STRING);
    }
    r = fread(Palette, 1, OSD_2ND_LOGO_PALETTE_SIZE, pFile);
    fclose(pFile);

	if(ucIndex == eCM_USER_LOGO_SERVICE_2D) //A65_OPTOMA_Julie_0076
	{
	    pFile = fopen(OSD_2ND_LOGO_SERVICE_PALETTE_FILE, "wb+");
	    if (pFile == NULL)
	    {
	        LOG_MSG(db_UPGRADE, "%s() fopen(OSD_2ND_LOGO_SERVICE_PALETTE_FILE) failed!\r\n", __func__);
	        return (ERROR_GUI_EMPTY_STRING);
	    }
	}
	else
	{
	    pFile = fopen(OSD_2ND_LOGO_PALETTE_FILE, "wb+");
	    if (pFile == NULL)
	    {
	        LOG_MSG(db_UPGRADE, "%s() fopen(OSD_2ND_LOGO_PALETTE_FILE) failed!\r\n", __func__);
	        return (ERROR_GUI_EMPTY_STRING);
	    }
	}
    w = fwrite(Palette, 1, OSD_2ND_LOGO_PALETTE_SIZE, pFile);
    fclose(pFile);

    if ((r != OSD_2ND_LOGO_PALETTE_SIZE) || (w != OSD_2ND_LOGO_PALETTE_SIZE))
    {
        LOG_MSG(db_UPGRADE, "%s() r = %d, w = %d!\r\n", __func__, r, w);
        return (ERROR_GUI_EMPTY_STRING);
    }
    return GUI_PASS;
}
#endif
INT8 halGui_HWInit(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color)
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_OSD_Init(iPalette_ColorAmount, pucPaletteData, ucInhibit_Color);  //need to get amount of palette color and palette data point

        halGui_SemaphoreGive(__FUNCTION__);

	    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Init complete!\n", __FUNCTION__, __LINE__);
    }
    else
    {
        LOG_MSG(db_HAL_GUI, "%s() fail!\n", __FUNCTION__);
    }

    return GUI_PASS;
}

INT8 halGui_OSDReload(void)
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_OSD_Reload();

        halGui_SemaphoreGive(__FUNCTION__);
    }
    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    return GUI_PASS;
}
#if (LOGO_REPLACE == 1)
INT8 halGui_2ND_LOGO_OSDReload(UINT8 ucIndex) //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0067
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_OSD_2ND_LOGO_Reload(ucIndex);
        halGui_SemaphoreGive(__FUNCTION__);
    }
    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    return GUI_PASS;
}
#endif

INT8 halGui_OSD_Copy(void) //A70LV_Larry_0067
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        OSD_CurrentCopy();

        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

INT8 halGui_OSD_On(COORDINATE sStart_Position, BLOCK_SIZE_INFO sDraw_Size, INT8 cTrap_Enable, OSD_TRANSPARENCY_COLOR_INFO sTrap_Color)
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        START_POINT sStartPoint;
        RECT_SIZE sRectSize;
        OSD_TRANSPARENCY_COLOR sTransparentColor;

        memcpy(&sStartPoint, &sStart_Position, sizeof(START_POINT));
        memcpy(&sRectSize,   &sDraw_Size,      sizeof(RECT_SIZE));
        memcpy(&sTransparentColor, &sTrap_Color, sizeof(OSD_TRANSPARENCY_COLOR));

        dvC821_OSD_On(sStartPoint, sRectSize, GUI_ON, sTransparentColor);  //disable transparency color for  first stage
        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}


INT8 halGui_OSD_Off(void)
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_OSD_Off();
        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}


INT8 halGui_OSD_Set_InhitbitColor( INT8 cEnable, INT16 iInhibit_Color )
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_Inhibit_Color_Set(cEnable, iInhibit_Color);
        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

INT8 halGui_Paint_Bitmap(OSD_BITMAP_INFO sBitmap, START_POINT_INFO sDes_position)
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        OSD_BITMAP sOsdBitmap;
        START_POINT sStartPoint;

        memcpy(&sOsdBitmap,  &sBitmap, sizeof(OSD_BITMAP));
        memcpy(&sStartPoint, &sDes_position, sizeof(START_POINT));

        dvC821_Paint_Bitmap(sOsdBitmap, sStartPoint);  //need to design return type from driver to Application at here

        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}
//ZU860_Clare_0132, add, >>>
#if 1
INT8 halGui_Paint_Specified_Bitmap(OSD_BITMAP_INFO sBitmap, COORDINATE sSrc_Offset_Position, COORDINATE sDraw_Position, BLOCK_SIZE_INFO sDraw_Size)  //H2 wait review
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        OSD_BITMAP sOsdBitmap;
        START_POINT sOffsetPosition;
        RECT_SIZE sSrcSize;
        START_POINT sDesPosition;

        memcpy(&sOsdBitmap, &sBitmap, sizeof(OSD_BITMAP));
        memcpy(&sOffsetPosition, &sSrc_Offset_Position, sizeof(START_POINT));
        memcpy(&sSrcSize, &sDraw_Size, sizeof(RECT_SIZE));
        memcpy(&sDesPosition, &sDraw_Position, sizeof(START_POINT));

        dvC821_Paint_Specified_Bitmap(sOsdBitmap, sOffsetPosition, sSrcSize, sDesPosition);

        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}
#else
INT8 halGui_Paint_Specified_Bitmap(COORDINATE sSrc_Position, BLOCK_SIZE_INFO sDraw_Size, COORDINATE sStart_Position)
{
	START_POINT sBmp_position;
	RECT_SIZE sBmp_Size;
	START_POINT sDes_position;
	memcpy(&sBmp_position, &sSrc_Position, sizeof(START_POINT));
	memcpy(&sBmp_Size, &sDraw_Size, sizeof(BLOCK_SIZE_INFO));
	memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT));

	halGui_SemaphoreTake(__FUNCTION__);
	dvC821_Paint_Specified_Bitmap(sBmp_position, sBmp_Size, sDes_position);  //need to design return type from driver to Application at her
	halGui_SemaphoreGive(__FUNCTION__);

	return GUI_FOUND;
}
#endif
//ZU860_Clare_0132, add, <<<

//T100_Sander_0013
#if 0
//ZU860_Clare_0019, >>>
INT8 halGui_Paint_TextHightLight(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index)      // for OPTOMA IP/PIN Text
{
    START_POINT sDes_position;
    RECT_SIZE sDes_size;
    INT8 iVal = 0;  //ZU860_Doulas_0136

    sDes_position.iX = sStart_Position.iX;
    sDes_position.iY = sStart_Position.iY;
    sDes_size.iWidth = sSize.iWidth;
    sDes_size.iHeight = sSize.iHeight;

    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)       //ZU860_Doulas_0136
    {
#ifdef CUSTOM_OPTOMA_ZU860	//ZU860_Clare_0093
        dvC821_Paint_Rectangle(sDes_position, sDes_size, GUI_C_SLIDER_BAR); 	// EK816U_626U_Energy_0019
#endif
        sDes_size.iHeight = sSize.iHeight;
        iVal = dvC821_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

        halGui_SemaphoreGive(__FUNCTION__);          //ZU860_Doulas_0136
    }

    return iVal;
}
//ZU860_Clare_0019, <<<
#endif

INT8 halGui_Paint_Rectangle(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index)
{
    eRESULT cResult = rcERROR;

    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        START_POINT sStartPoint;
        RECT_SIZE   sRectSize;

        memcpy(&sStartPoint, &sStart_Position, sizeof(START_POINT));
        memcpy(&sRectSize,   &sSize, sizeof(RECT_SIZE));

        cResult = (eRESULT)dvC821_Paint_Rectangle(sStartPoint, sRectSize, iColor_Index);

        halGui_SemaphoreGive(__FUNCTION__);
    }

    return cResult;
}


INT8 halGui_GuiData_EraseAll(void) //A70LV_Larry_0004
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        //call C734 SPI-flash erase fucntion
        dvC821_SerialFlash_Erase_All();

        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)Erase spi-flash complete!\n", __FUNCTION__, __LINE__);

        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

INT8 halGui_GuiData_FlashEraseSector(UINT32 ulAddr, UINT32 ulSize) //A70LV_Larry_0004
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_SerialFlash_Erase_Sector(ulAddr, ulSize);

        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

INT8 halGui_GuiData_FlashWrite(UINT32 ulAddr, UINT8 *pucData, UINT32 ulSize) //A70LV_Larry_0004
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        #ifdef SCALER_C821_C789
        dvC821_SerialFlash_Write(ulAddr, pucData, ulSize);
        #endif
		
        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

#define FLASH_SECTION_SIZE 0x10000

#if 0
UINT32 halGui_GuiData_PartialWrite(char *fileName, UINT32 ulAddr) //A70LV_Larry_0134
{
    UINT32 ulBlock = 0;
    UINT32 ulCount = 0;
    UINT32 ulCheckSum = 0;
    UINT32 ulExtSheckSum = 0;
    UINT32 ulSectorSize = 0; //A70LV_Larry_0158
    UINT32 ulFileSize = 0;
    FILE *pFile = NULL;
    UINT8 *pcExtAddr = NULL;
    UINT8 *pcReadAddr = NULL;

    pFile = fopen(fileName, "rb");

    if(pFile == NULL)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) Open %s failed!\r\n", __FUNCTION__, __LINE__, fileName);
        return 0;
    }
    else //計算.dat szie //A35G2_CDS_Larry_0036
    {
        fseek(pFile, 0, SEEK_END);
        ulSize = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
    }

    if((ulSize % 0x10000 == 0))
    {
        ulBlock = ulSize/0x10000;
    }
    else
    {
        ulBlock = (ulSize/0x10000) + 1;
    }

    ulFileSize = ulBlock*0x10000;

    pcExtAddr = (UINT8 *)malloc(ulFileSize);
    memset(pcExtAddr, 0xFF, ulFileSize);
    fread(pcExtAddr, ulSize, 1, pFile);
    fclose(pFile);
    pcReadAddr = pcExtAddr;

    for(ulCount = 0; ulCount < ulBlock; ulCount++)
    {
        ulCheckSum = 0;
        ulSectorSize = 0; //A70LV_Larry_0158
        ulExtSheckSum = 0;

        if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
        {
            ulCheckSum = dvC821_SerialFlash_CheckSum(ulAddr, 0x10000);

            halGui_SemaphoreGive(__FUNCTION__);
        }

        do
        {
            ulExtSheckSum = ulExtSheckSum + (*(pcReadAddr++) * (ulSectorSize + 1));
            ulSectorSize++; //A70LV_Larry_0158
        }while(ulSectorSize < 0x10000); //64K


        if(ulCheckSum != ulExtSheckSum)
        {
            halGui_GuiData_FlashEraseSector(ulAddr, 0x10000);
            halGui_GuiData_FlashWrite(ulAddr, (pcExtAddr + ulCount * 0x10000), 0x10000);
            MS_SLEEP(10);

            if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
            {
                ulCheckSum = dvC821_SerialFlash_CheckSum(ulAddr, 0x10000);

                halGui_SemaphoreGive(__FUNCTION__);
            }
            MS_SLEEP(10);

            if(ulExtSheckSum != ulCheckSum)
            {
                LOG_MSG(db_UPGRADE, "Flash Copy Fail\n");
            }
            else
            {
                LOG_MSG(db_UPGRADE, "Flash Copy Pass\n");
            }
        }

        ulAddr = ulAddr + 0x10000;
//        ulExtAddr = ulExtAddr + 0x10000;
//        pucData = (UINT8 *)ulExtAddr;

        LOG_MSG(db_UPGRADE, "Copy Address %03d 0x%08x-0x%08x [0x%08x][0x%08x]\n", ulCount, ulAddr,ulAddr+0xFFFF, ulExtSheckSum, ulCheckSum);
        //MS_SLEEP(10);
    }

    free(pcExtAddr);

    return GUI_PASS;
}
#endif /* 0 */

#if 1
UINT32 halGui_GuiData_PartialWrite(char *fileName, UINT32 ulFlashAddr, UINT32 ulRAMAddr, UINT8 ucFocus) //A35G2_CDS_Larry_0050
{
    UINT32 ulBlock = 0;
    UINT32 ulCount = 0;
    UINT32 ulReadCRC = 0, ulReadCRC2 = 0;
    UINT32 ulExtCRC = 0;
    UINT32 ulSectorSize = 0;
    UINT32 ulFileSize = 0;
    FILE *pFile = NULL;
    UINT8 *pcExtAddr = NULL;
    UINT8 *pcReadAddr = NULL;
    UINT32 ulSize;
    UINT8 *pcReadBuffer = NULL;

    pFile = fopen(fileName, "rb");

    if(pFile == NULL)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) Open %s failed!\r\n", __FUNCTION__, __LINE__, fileName);
        return 0;
    }
	else //計算.dat szie
    {
        fseek(pFile, 0, SEEK_END);
        ulSize = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
    }

    if((ulSize % FLASH_SECTION_SIZE == 0))
    {
        ulBlock = ulSize/FLASH_SECTION_SIZE;
    }
    else
    {
        ulBlock = (ulSize/FLASH_SECTION_SIZE) + 1;
    }

    ulFileSize = ulBlock*FLASH_SECTION_SIZE;

    pcExtAddr = (UINT8 *)malloc(ulFileSize);
    memset(pcExtAddr, 0xFF, ulFileSize);
    fread(pcExtAddr, ulSize, 1, pFile);
    fclose(pFile);
    pcReadAddr = pcExtAddr;

    pcReadBuffer = (UINT8 *)malloc(FLASH_SECTION_SIZE + 0x1000);

    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        OSD_DMA_SerialFlash_Set(ulFlashAddr, ulRAMAddr, 0, ulFileSize);
        halGui_SemaphoreGive(__FUNCTION__);
    }

    for(ulCount = 0; ulCount < ulBlock; ulCount++)
    {
        ulReadCRC = 0;
        ulReadCRC2 = 0;
        ulSectorSize = 0;
        ulExtCRC = 0;

        if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
        {
            memset(pcReadBuffer, 0, FLASH_SECTION_SIZE);

            dvC821_RAM_Read(ulRAMAddr, pcReadBuffer, FLASH_SECTION_SIZE);
            ulReadCRC = utilCRC16Calc(pcReadBuffer, FLASH_SECTION_SIZE);

            halGui_SemaphoreGive(__FUNCTION__);
        }

        ulExtCRC = utilCRC16Calc((pcExtAddr + ulCount * FLASH_SECTION_SIZE), FLASH_SECTION_SIZE);

        if((ulReadCRC != ulExtCRC) || ucFocus)
        {
            if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
            {
                dvC821_SerialFlash_Erase_Sector(ulFlashAddr, FLASH_SECTION_SIZE);
                dvC821_RAM_Write(ulRAMAddr, (pcExtAddr + ulCount * FLASH_SECTION_SIZE), FLASH_SECTION_SIZE);

                OSD_DMA_DDR3_WriteTo_Flash(ulRAMAddr, ulFlashAddr, 0, FLASH_SECTION_SIZE);
                OSD_DMA_SerialFlash_Set(ulFlashAddr, ulRAMAddr, 0, FLASH_SECTION_SIZE);

                memset(pcReadBuffer, 0, FLASH_SECTION_SIZE);

                dvC821_RAM_Read(ulRAMAddr, pcReadBuffer, FLASH_SECTION_SIZE);
                ulReadCRC2 = utilCRC16Calc(pcReadBuffer, FLASH_SECTION_SIZE);

                halGui_SemaphoreGive(__FUNCTION__);
            }

            if(ulExtCRC != ulReadCRC2)
            {
                LOG_MSG(db_UPGRADE, "Flash Copy Fail\n");
            }
            else
            {
                LOG_MSG(db_UPGRADE, "Flash Copy Pass\n");
            }
        }

        LOG_MSG(db_UPGRADE, "Copy Address %03d 0x%08x-0x%08x [0x%04x][0x%04x][0x%04x]\n", ulCount, ulFlashAddr, ulFlashAddr + (FLASH_SECTION_SIZE - 1), ulExtCRC, ulReadCRC, ulReadCRC2);

        ulFlashAddr = ulFlashAddr + FLASH_SECTION_SIZE;
        ulRAMAddr = ulRAMAddr + FLASH_SECTION_SIZE;
    }

    free(pcExtAddr);
    free(pcReadBuffer);

    return GUI_PASS;
}
#endif /* 0 */

void halGui_Upgrade(UINT8 ucFocus) //A35G2_CDS_Larry_0050
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_OSD_Memory_Protect(0);

        halGui_SemaphoreGive(__FUNCTION__);
    }

    if(access(OSD_BITMAP_RAWDATA, 0) == 0)
    {
        halGui_GuiData_PartialWrite(OSD_BITMAP_RAWDATA, DEF_SFL_OSD_PSAD_2K, (DEF_BITMAPAD_4K & 0x3fffffff), ucFocus);
    }
    else if(access(CUSTOM_OSD_BITMAP_RAWDATA, 0) == 0) //A35G2_Larry_0067 for RD debug
    {
        halGui_GuiData_PartialWrite(CUSTOM_OSD_BITMAP_RAWDATA, DEF_SFL_OSD_PSAD_2K, (DEF_BITMAPAD_4K & 0x3fffffff), ucFocus);
    }

    if(access(OSD_TEXT_RAWDATA, 0) == 0) //A35G2_Larry_0067 for RD debug
    {
        halGui_GuiData_PartialWrite(OSD_TEXT_RAWDATA, DEF_SFL_OSD_FSAD_2K, (DEF_FONTAD_4K & 0x3fffffff), ucFocus);
    }
    else if(access(CUSTOM_OSD_TEXT_RAWDATA, 0) == 0)
    {
        halGui_GuiData_PartialWrite(CUSTOM_OSD_TEXT_RAWDATA, DEF_SFL_OSD_FSAD_2K, (DEF_FONTAD_4K & 0x3fffffff), ucFocus);
    }
}

#if (LOGO_REPLACE == 1)
void halGui_Upgrade_Second_Logo_Replace(UINT8 ucFocus)  //A35G2_Coda_0067
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_OSD_Memory_Protect(0);

        halGui_SemaphoreGive(__FUNCTION__);
    }

	halGui_SecondLogoPaletteStore(TMP_2ND_LOGO_PALETTE_FILE, eCM_USER_LOGO_2D);
 	halGui_GuiData_PartialWrite(TMP_2ND_LOGO_RAWDATA_FILE, DEF_SFL_OSD_2ND_LOGO, (DEF_BITMAPAD_4K & 0x3fffffff), ucFocus);
 }

void halGui_Upgrade_Service_Second_Logo_Replace(UINT8 ucFocus)  //A35G2_Coda_0067
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_OSD_Memory_Protect(0);

        halGui_SemaphoreGive(__FUNCTION__);
    }

	halGui_SecondLogoPaletteStore(TMP_2ND_LOGO_PALETTE_FILE, eCM_USER_LOGO_SERVICE_2D);
	halGui_GuiData_PartialWrite(TMP_2ND_LOGO_RAWDATA_FILE, DEF_SFL_OSD_SERVICE_2ND_LOGO, (DEF_BITMAPAD_4K & 0x3fffffff), ucFocus);
}
#endif

INT8 halGui_InitSemaphore(void)     //A70LV_Doulas_0014
{
//#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
    //xGuiSemaphore = palImage_Scaler_xSemaphore_Get();
//#else
//    xGuiSemaphore = palImage_Scaler_xSemaphore_Get();
//#endif

//    if(xGuiSemaphore == NULL)
//        return ERROR_GUI_SEMAPHORE_FAIL;

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Init complete!\n", __FUNCTION__, __LINE__);
    return GUI_PASS;
}

INT8 halGui_PanelSet(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0105
{
    dvC821_OSD_PanelSet(ePanelTimingId);
    return GUI_PASS;
}

INT8 halGui_PanelChange(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0105
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_OSD_PanelChange(ePanelTimingId);

        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

INT8 halGui_OSD_MenuTransparencySet(UINT8 ucValue)         //A70LV_Doulas_0122
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_OSD_MenuTransparencySet(ucValue);

        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

INT8 halGui_OSD_MenuTransparencyEnableSet(UINT8 ucEnable)         //A70LV_Doulas_0122
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_OSD_MenuTransparencyEnableSet(ucEnable);

        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

INT8 halGui_DrawRect(UINT16 uiXStart, UINT16 uiXWidth, UINT16 uiYStart, UINT16 uiYWidth, UINT8 ucForeColor, UINT8 ucBackColor) //A70LV_Larry_0137
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_OSD_DrawRect(uiXStart, uiXWidth, uiYStart, uiYWidth, ucForeColor, ucBackColor);

        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

INT8 halGui_Paint_Line(COORDINATE sStart_Position, UINT8 width, INT16 iColor_Index)   //ZU860_Clare_0001, add
{
    START_POINT sDes_position;
    RECT_SIZE sDes_size;
    INT8 iVal = 0;  //ZU860_Doulas_0136

    memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT));

    sDes_position.iX = sStart_Position.iX;
    sDes_position.iY = sStart_Position.iY;
    sDes_size.iWidth = width;
    sDes_size.iHeight = 1;

    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)       //ZU860_Doulas_0136
    {
        iVal = dvC821_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

        halGui_SemaphoreGive(__FUNCTION__);          //ZU860_Doulas_0136
    }

    return iVal;
}

void halGui_UpgradeAccess_Set(BOOL bEnable)	//A65_OPTOMA_Doulas_0126
{
    dvC821_UpgradeAccess_Set(bEnable);
}

BOOL halGui_UpgradeAccess_Get(void)	//A65_OPTOMA_Doulas_0126
{
    return dvC821_UpgradeAccess_Get();
}

//only for debug
void halGui_ShowOsdBitmapData(UINT32 ulStartVertPosition)	//A35G2_Simon_0114
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        dvC821_ShowOsdBitmapData(ulStartVertPosition);

        halGui_SemaphoreGive(__FUNCTION__);
    }
}


INT8 halGui_OSD_Memory_Protect(UINT8 Enable)
{
    if(halGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        Enable = !!Enable;
        dvC821_OSD_Memory_Protect(Enable);

        halGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

