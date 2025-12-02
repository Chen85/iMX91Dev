#include <math.h>
#include "dvProAV_Hdr.h"

//#define DEF_TONE_MEPPING_CAL_PRINT
#define DEF_TONE_BT2390

uint32_t m_arHdrGammaTable[GAMMA_TBL_NUM];
static const double m_arHdrCscTable_2020to709[]=
{
    1.66015625   , -0.587890625, -0.072265625,
    -0.125       , 1.1328125   , -0.0078125,
    -0.017578125 , -0.1015625  , 1.119140625
};

static const double m_arHdrCscTable_2020toAdobe[]=
{
    1.1520  , -0.0975   , -0.0545,
    -0.1246 , 1.1329    , -0.0083,
    -0.0225 , -0.0498   , 1.0723
};

static const double m_arHdrCscTable_2020toP3[]=
{
    1.3436  , -0.2822   , -0.0614,
    -0.0653 , 1.0758    , -0.0105,
    0.0028  , -0.0196   , 1.0168
};

static const double m_arHdrCscTable_P3to709[]=
{
    1.22491   , -0.224915   , 0         ,
    -0.0420532, 1.04205     , 0         ,
    -0.0195923, -0.0786133  , 1.09827
};

static const double m_arHdrCscTable_Adobeto709[]=
{
    1.39832 , -0.398315 , 0         ,
    0       , 1         , 0         ,
    0       , -0.0429077, 1.04291
};

//static QTextStream m_txtStream;
//static QTextStream m_txtStream32;

//-------------------------------------------------------------------------------------------------
// SDR Gamma Calculate
//-------------------------------------------------------------------------------------------------
#define SDR_GAMMA 2.2
double SDR_EOTF(double input)
{
    double output = pow(input, SDR_GAMMA);      // EOTF from ITU-R BT.1886
    return output;
}

double SDR_OETF(double input)
{
    double output = pow(input, 1 / SDR_GAMMA);  //inver EOTF from ITU-R BT.1886
    return output;
}

//-------------------------------------------------------------------------------------------------
// HDR Gamma Calculate
//-------------------------------------------------------------------------------------------------
double  LW   = 10000;   // Signal max(Luminances White), 1000 From Rx HDR info-frame Max Master Lum. (Astro Pattern Generate) => use clip mode ?
double  LB   = 0;       // Signal min(Luminances Black),   50 From Rx HDR info-frame Max Master Lum. (Astro Pattern Generate) => (TBD)
double  Lmax = 100;	    // Display maximum Luminances
double  Lmin = 0.005;   // Display minimum Luminances

uint16 m_u16MaxLevel = (GAMMA_TBL_NUM - 1);

bool clip_enabled = false;
double clip_point = 0;

int trans_method = 3; // 1: method I, 2: method II, 3:method III
double high_pivot_y = 0.9;
double m_gamma = 1.5;
double low_pivot_x = 0.03125;
double low_pivot_y = 0.01;

const double c1 = 3424. / 4096;
const double c2 = (2413. / 4096) * 32;
const double c3 = (2392. / 4096) * 32;

const double m1 = 2610. / 16384;
const double m2 = (2523. / 4096) * 128;

double LB_OETF;
double LW_OETF;
double Lmax_OETF;
double Lmin_OETF;

double m_min_lum;
double m_max_lum;

double m_Ks;        // KneeStart, 最高亮度轉折點
double b;           // 最低亮度Offset

double E3_1_min, E3_1_max;

double PQ_norm_OETF(double display_luminance)	// input 0~1, output 0~1
{
    double Y = display_luminance;
    double numerator, denominator, output;

    numerator = c1 + c2 * pow(Y, m1);
    denominator = 1 + c3 * pow(Y, m1);
    output = pow(numerator / denominator, m2);

    return output;
}

double PQ_norm_EOTF(double input)   // input 0~1, output 0~1
{
    double Y;
    double numerator, denominator, output;

    numerator = (pow(input, 1 / m2) - c1) > 0 ? (pow(input, 1 / m2) - c1) : 0;
    denominator = c2 - c3 * pow(input, 1 / m2);
    Y = pow(numerator / denominator, 1 / m1);
    output = Y;

    return output;
}


double PQ_OETF(double display_luminance)     // input 0~10000, output 0~1
{
    double Y = display_luminance / 10000;

    double output = PQ_norm_OETF(Y);

    return output;
}

double PQ_EOTF(double input)    //input 0~1,  output 0~10000
{
    double Y = PQ_norm_EOTF(input);
    double output = 10000 * Y;
    return output;
}

double Hermite_Spline_Equation(double input, double Ks, double maxLum)
{
    double T = (input - Ks) / (1 - Ks);
    double output;

    output = (2 * pow(T, 3) - 3 * pow(T, 2) + 1) * Ks + (pow(T, 3) - 2 * pow(T, 2) + T)*(1 - Ks) + (-2 * pow(T, 3) + 3 * pow(T, 2)) * maxLum;

    return output;
}

double PQ_get_E3(double input)
{
    //double E1 = (input - LB_OETF) / (LW_OETF - LB_OETF);    // BT-2390
    double E1 = input ;

    if(input == 0)
        return 0;
    double E2, E3, E3_1;
    // step 3.1
    if (!clip_enabled)
    {
        if (E1 <= m_Ks)
        {
            E2 = E1;
        }
        else
        {
            E2 = Hermite_Spline_Equation(E1, m_Ks, m_max_lum);
        }
    }
    else // clip mode
    {
        if (E1 < clip_point)
        {
            E2 = E1;
        }
        else
        {
            E2 = clip_point;
        }
    }
    // step 3.2
#ifdef DEF_TONE_BT2390
    E3 = E2 + b * pow((1 - E2), 4);
    E3_1 = E3 * (LW_OETF - LB_OETF) + LB_OETF;  //E4
#else
    E3 = E2;
    E3_1 = E3;
#endif

    return E3_1;
}

void PQ_INITIAL(PQ_HDR_PARAM hdrParam)
{
    m_u16MaxLevel = hdrParam.u16MaxLevel;

    // signal parameters
    LW = hdrParam.signalMax;        // Signal max
    LB = hdrParam.signalMin;        // Signal min

    // display parameters
    Lmax = hdrParam.displayMax;     // Display max
    Lmin = hdrParam.displayMin;     // Display min

    // clip mode parameters
    clip_enabled = hdrParam.clipEnabled;
    clip_point = hdrParam.clipPoint;

    // trans method parameters
    trans_method = hdrParam.transMethod;
    high_pivot_y = hdrParam.transHighPivotY;
    m_gamma = hdrParam.transGamma;  // trans method gamma
    low_pivot_x = hdrParam.transLowPivotX;
    low_pivot_y = hdrParam.transLowPivotY;

    LB_OETF = PQ_OETF(LB);
    LW_OETF = PQ_OETF(LW);
    Lmax_OETF = PQ_OETF(Lmax);
    Lmin_OETF = PQ_OETF(Lmin);

    m_min_lum = (Lmin_OETF - LB_OETF) / (LW_OETF - LB_OETF);
    m_max_lum = (Lmax_OETF - LB_OETF) / (LW_OETF - LB_OETF);

    m_Ks = 1.5 * m_max_lum - 0.5;

    if (hdrParam.clipEnabled)
    {
        clip_point = (hdrParam.clipPoint == 0)? Lmax_OETF : hdrParam.clipPoint;
    }
    else
    {
        clip_point = hdrParam.clipPoint;
    }

    b = m_min_lum;
#if 0 // todo
    cout << "LB_OETF=" << LB_OETF << endl;
    cout << "LW_OETF=" << LW_OETF << endl;
    cout << "Lmax_OETF=" << Lmax_OETF << endl;
    cout << "Lmin_OETF=" << Lmin_OETF << endl;
    cout << "m_max_lum=" << m_max_lum << endl;
    cout << "m_min_lum=" << m_min_lum << endl;
    cout << "m_Ks=" << m_Ks << endl;
    cout << "b=" << b << endl;
#endif

    E3_1_min = PQ_get_E3(0);
    E3_1_max = PQ_get_E3(1);
#if 0 // todo
    cout << "E3_1_min=" << E3_1_min << endl;
    cout << "E3_1_max=" << E3_1_max << endl;
#endif
}

double G_normalize(double input, double max, double min, double gamma)
{
    double output;
    double norm_in = (input - min)/(max - min);
    output = pow(norm_in, gamma);

    return output;
}

double piecewise_G_normalizeM2(double input, double pivot, double gamma)
{
    double output;
    if (input < pivot)
    {
        output = pivot * pow(input / pivot, gamma);
    }
    else
    {
        output = input;
    }

    return output;
}


double  piecewise_G_normalizeM3(double input_x, double input_y, double high_pivot_y, double gamma, double low_pivot_x, double low_pivot_y, double original_low_pivot_y)
{
    double output;

    if(input_y > high_pivot_y)
    {
        output = input_y;
    }
    else if (input_x <= low_pivot_x)
    {
        output = (input_x * low_pivot_y) / low_pivot_x;
    }
    else
    {
        output = low_pivot_y + (high_pivot_y - low_pivot_y) * pow((input_y - original_low_pivot_y) / (high_pivot_y- original_low_pivot_y), gamma);
    }

    return output;

}

double normalize(double input, double max, double min)
{
    double output;

    output = (input - min) / (max - min);

    return output;
}

double PQ_EETF_TRANSFER(double input)
{
    double PQ_E5;
    //double high_pivot_y;
    //double gamma ;
    //double low_pivot_x;
    //double low_pivot_y;
    double input_x = input / (GAMMA_TBL_NUM - 1);

    double E4 = PQ_get_E3(input / m_u16MaxLevel);

    if (trans_method == 1)
    {
        //########## Method I ###############
        //gamma = 1.5;
        PQ_E5 = G_normalize(E4, E3_1_max, 0, m_gamma);  // on PQ panel
    }
    else if (trans_method == 2)
    {
        //########## Method II ###############
        E4 = normalize(E4, E3_1_max, 0);

        //high_pivot_y = 0.9;
        //gamma = 1.5;
        PQ_E5 = piecewise_G_normalizeM2(E4, high_pivot_y, m_gamma);
    }
    else // Method = 3
    {
        //########## Method III ###############
        E4 = normalize(E4, E3_1_max, 0);

        //high_pivot_y = 0.9;
        //gamma = 1.5;
        //low_pivot_x = 0.03125;
        //low_pivot_y = 0.01;
        double original_low_pivot_y = PQ_get_E3(low_pivot_x);

        PQ_E5 = piecewise_G_normalizeM3(input_x, E4, high_pivot_y, m_gamma, low_pivot_x, low_pivot_y, original_low_pivot_y);
    }

    return PQ_E5;
}

double PQ_SDR_EETF_TRANSFER(double input)
{
    double E3_1 = PQ_get_E3(input);
    double E4 = PQ_EOTF(E3_1);
    double SDR_E5 = SDR_OETF((E4 - Lmin) / (Lmax - Lmin));	// on SDR panel
    //cout << SDR_E5 << " " << E4 << endl;

    return SDR_E5;
}

double S_normalize(double input, double max, double min, double curve_control)
{
    double output;
    double norm_in_mid = (max - min) / 2;
    double norm_in = ((input - min) - norm_in_mid ) / ((max - min)/(2 * 5));
    double norm_min = 1 / (1 + exp(curve_control * 5));

    output = 1 / (1 + exp(- curve_control * norm_in));
    output = output - norm_min;

    return output;
}

// set Gamma4 table for PQ input
double PQ_2_SDR_ToneMap(uint16_t index)
{
    double temp;
    temp = PQ_EETF_TRANSFER((double)index);  // EETF from PQ to SDR
    temp = (GAMMA_TBL_NUM - 1) * temp;          // 0~1 to 0~1023
    temp = floor(temp * pow(2, FRACTION_BITS)); // 10bit int + 16bit float to int
    return temp;
}

//-------------------------------------------------------------------------------------------------
// Hdr Tone Mapping (Gamma4)
//-------------------------------------------------------------------------------------------------
uint08 *dvProAV_HdrPq2SdrGammaCalculate(PQ_HDR_PARAM hdrParam)
{
    PQ_INITIAL(hdrParam);

    // HDR Gamma Calaulate
    for (uint16_t i= 0 ;i <GAMMA_TBL_NUM; i++)
    {
        double tone = PQ_2_SDR_ToneMap(i);
        m_arHdrGammaTable[i] = (uint32_t)tone;
    }

#ifdef DEF_TONE_MEPPING_CAL_PRINT
    // HDR Gamma Print
    FILE *fptr;
    fptr = fopen("HdrGamma4_4byte.txt","w");
    char *pAsciiArray = (char *)malloc(GAMMA_TBL_NUM * 12);
    uint16 u16Cnt = 0;
    for (uint16_t i= 0 ;i <GAMMA_TBL_NUM; i++)
    {
        pAsciiArray[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 4 ) & 0x0f);
        pAsciiArray[u16Cnt++] = hextoascii(m_arHdrGammaTable[i] & 0x0f);
        pAsciiArray[u16Cnt++] = ' ';
        pAsciiArray[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 12 ) & 0x0f);
        pAsciiArray[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 8 ) & 0x0f);
        pAsciiArray[u16Cnt++] = ' ';
        pAsciiArray[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 20 ) & 0x0f);
        pAsciiArray[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 16 ) & 0x0f);
        pAsciiArray[u16Cnt++] = ' ';
        pAsciiArray[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 28 ) & 0x0f);
        pAsciiArray[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 24 ) & 0x0f);
        pAsciiArray[u16Cnt++] = '\n';
    }
    fwrite(&pAsciiArray[0], 1, (GAMMA_TBL_NUM * 12), fptr);
    fclose(fptr);
    free(pAsciiArray);

    FILE *fptr1;
    fptr1 = fopen("HdrGamma4_32bits.txt","w");
    char *pAsciiArray1 = (char *)malloc(GAMMA_TBL_NUM * 10);
    u16Cnt = 0;
    for (uint16_t i= 0 ;i <GAMMA_TBL_NUM; i++)
    {
        pAsciiArray1[u16Cnt++] = '\'';
        pAsciiArray1[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 28) & 0x0f);
        pAsciiArray1[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 24) & 0x0f);
        pAsciiArray1[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 20) & 0x0f);
        pAsciiArray1[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 16) & 0x0f);
        pAsciiArray1[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 12) & 0x0f);
        pAsciiArray1[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 8) & 0x0f);
        pAsciiArray1[u16Cnt++] = hextoascii((m_arHdrGammaTable[i] >> 4) & 0x0f);
        pAsciiArray1[u16Cnt++] = hextoascii(m_arHdrGammaTable[i] & 0x0f);
        pAsciiArray1[u16Cnt++] = '\n';
    }
    fwrite(&pAsciiArray1[0], 1, GAMMA_TBL_NUM * 10, fptr1);
    fclose(fptr1);
    free(pAsciiArray1);
#endif

    return (uint8*)m_arHdrGammaTable;
}

//-------------------------------------------------------------------------------------------------
// HDR
//-------------------------------------------------------------------------------------------------
/**
 * @brief Download the HDR CSC Table to Chip
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] cscTable: HDR CSC Table (3 bytes * 9)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @note
 * if(input>0)
 *    output =DEC2HEX(FLOOR(N32*POWER(2,16),1),6) ;
 * else
 *    output =DEC2HEX(FLOOR(ABS(N32)*POWER(2,16),1)+POWER(2,17),6)) ;
 */
int dvProAV_SclVopHdrCscSet(SclEntity eEntity, uint08 *cscTable)
{
    int status = rcSUCCESS;

    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 coefAddr[9] = {eVopmHdrR1CoefAdr, eVopmHdrG1CoefAdr, eVopmHdrB1CoefAdr,
                          eVopmHdrR2CoefAdr, eVopmHdrG2CoefAdr, eVopmHdrB2CoefAdr,
                          eVopmHdrR3CoefAdr, eVopmHdrG3CoefAdr, eVopmHdrB3CoefAdr};
    uint32 u32data, offset =  (eEntity == eSclEntity_Main) ? 0 : (eVopsHdrR1CoefAdr - eVopmHdrR1CoefAdr);

    for (int i =0; i < 9; i++)
    {
        u32data  = cscTable[i * 3 + 2];
        u32data <<= 8;
        u32data |= cscTable[i * 3 + 1];
        u32data <<= 8;
        u32data |= cscTable[i * 3 + 0];
        status &= dvProAV_AccessWrite(coefAddr[i] + offset, (uint32)u32data);
    }

    return status;
}

/**
 * @brief dvProAV_SclVopHdrCscEn
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] bHdrCscEn : HDR CSC enable
 *          -false : HDR CSC disable
 *          -true : HDR CSC enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopHdrCscEn(SclEntity eEntity, bool bHdrCscEn)
{
    return dvProAV_AccessWrite((eEntity == eSclEntity_Main)? eVopmHdrCsc2Enable : eVopsHdrCsc2Enable, bHdrCscEn);
}

/**
 * @brief Download the HDR Gamma Table to Chip
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] gammaType: HDR gamma type
 *                 - eHdrGamma1
 *                 - eHdrGamma3
 *                 - eHdrGamma4
 * @param [in] gamma: gamma data
 *
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopHdrGammaDl2Chip(SclEntity eEntity, uint08 u8GammaType, const uint08 *gamma)
{
    int status = rcSUCCESS;
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    const uint8 ucType[3] = {eUmbGammaType_Hdr1, eUmbGammaType_Hdr30, eUmbGammaType_Hdr4};
    const uint32 ulDram[3] = {DRAM_HDRPQ1_ADDR, DRAM_HDRPQ3_ADDR, DRAM_HDRPQ4_ADDR};
    const uint16 GammaCount = 1024;
    uint16 u16Length = GammaCount * 4;
    uint8 *pcData = (uint8 *)malloc(u16Length);
    if(pcData == NULL)
    {
        return rcERROR;
    }

    u8GammaType %= sizeof(ucType);

    memcpy(pcData, gamma, u16Length);

    do
    {
        //status &= dvProAV_UmbGammaWrite(eUmbFunc_SpiToGamma, 0, ucType[u8GammaType], u16Length, pcData);
        status &= dvProAV_LineBufWrite(eDramBank0, ulDram[u8GammaType], u16Length, pcData);
        status &= dvProAV_UmbGammaWrite(eUmbFunc_DramToGamma, ulDram[u8GammaType], ucType[u8GammaType], u16Length, pcData);
    }while(status != rcSUCCESS);

    free(pcData);

    return status;
}

/**
 * @brief Set the HDR csc by index
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] cscType: HDR csc type index
 *          -eHdrCsc2020to709
 *          -eHdrCsc2020toAdobe
 *          -eHdrCsc2020toP3
 *          -eHdrCscP3to709
 *          -eHdrCscAdobeto709
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopHdrCscByIndexSet(SclEntity eEntity, HdrCscType eCscType)
{
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    const double *table;
    uint08 u8data[27];

    if (eCscType == eHdrCsc2020to709)
    {
        table = m_arHdrCscTable_2020to709;
    }
    else if (eCscType == eHdrCsc2020toAdobe)
    {
        table = m_arHdrCscTable_2020toAdobe;
    }
    else if (eCscType == eHdrCsc2020toP3)
    {
        table = m_arHdrCscTable_2020toP3;
    }
    else if (eCscType == eHdrCscP3to709)
    {
        table = m_arHdrCscTable_P3to709;
    }
    else if (eCscType == eHdrCscAdobeto709)
    {
        table = m_arHdrCscTable_Adobeto709;
    }
    else
    {
        table = m_arHdrCscTable_2020to709;
    }

    for (int i=0 ; i<9 ; i++)
    {
        double coeff = table[i];
        uint32 u32data;
        if (coeff >= 0)
        {
            coeff *= pow(2.0, 16);
        }
        else
        {
            coeff *= pow(2.0, 16) * (-1);
            coeff += pow(2.0, 18); // 18 for 2bit(18-16) integer
        }
        u32data = (uint32)(coeff);

        u8data[i * 3 + 2] = (u32data >> 16) & 0xFF;
        u8data[i * 3 + 1] = (u32data >>  8) & 0xFF;
        u8data[i * 3 + 0] = (u32data      ) & 0xFF;
    }

    return dvProAV_SclVopHdrCscSet(eEntity, u8data);
}

/**
 * @brief Set the HDR Gamma Enabled by Type
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] gammaType: HDR gamma type
 *                 - eHdrGamma1
 *                 - eHdrGamma3
 *                 - eHdrGamma4
 *                 - eHdrGamma3_7t
 * @param [in] enable: enable the selected gamma type
 *
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SclVopHdrGammaEnableSet(SclEntity eEntity, uint08 gammaType, bool enable)
{
    if(eEntity >= eSclEntity_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }
    uint32 gammaEn[3] = {eVopmHdrG1GammaEn, eVopmHdrG3GammaEn, eVopmHdrG4GammaEn};
    uint32 offset = (eEntity == eSclEntity_Main) ? 0 : (eVopsHdrG1GammaEn - eVopmHdrG1GammaEn);

    return dvProAV_AccessWrite(gammaEn[gammaType % sizeof(gammaEn)] + offset, (uint32)enable);
}

/**
 * @brief Color Gamut Matrix Transform
 * @param [in] sSrcClrGamutCoor : source color gamut coordinate
 * @param [in] sTgClrGamutCoor : target color gamut coordinate
 * @param [out] darMtx : color gamut transform matrix point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdrCscTransMtx(CIE_Coor sSrcClrGamutCoor, CIE_Coor sTgClrGamutCoor, double *darMtx)
{
    double dTempRz, dTempGz, dTempBz;

    dTempRz = (sSrcClrGamutCoor.dRx + sSrcClrGamutCoor.dRy);
    dTempGz = (sSrcClrGamutCoor.dGx + sSrcClrGamutCoor.dGy);
    dTempBz = (sSrcClrGamutCoor.dBx + sSrcClrGamutCoor.dBy);
    // coordinate to matrix
    double darSrcMtx[9] =
    {
        sSrcClrGamutCoor.dRx, sSrcClrGamutCoor.dGx, sSrcClrGamutCoor.dBx,
        sSrcClrGamutCoor.dRy, sSrcClrGamutCoor.dGy, sSrcClrGamutCoor.dBy,
        1 - dTempRz         , 1 - dTempGz          , 1 - dTempBz
    };

    dTempRz = (sTgClrGamutCoor.dRx + sTgClrGamutCoor.dRy);
    dTempGz = (sTgClrGamutCoor.dGx + sTgClrGamutCoor.dGy);
    dTempBz = (sTgClrGamutCoor.dBx + sTgClrGamutCoor.dBy);
    double darTgMtx[9] =
    {
        sTgClrGamutCoor.dRx , sTgClrGamutCoor.dGx, sTgClrGamutCoor.dBx,
        sTgClrGamutCoor.dRy , sTgClrGamutCoor.dGy, sTgClrGamutCoor.dBy,
        1 - dTempRz         , 1 - dTempGz        , 1 - dTempBz
    };

    // Inverse Matrix
    double dDet = (darSrcMtx[0] * ((darSrcMtx[4] * darSrcMtx[8]) - (darSrcMtx[5] * darSrcMtx[7]))) - (darSrcMtx[1] * ((darSrcMtx[3] * darSrcMtx[8]) - (darSrcMtx[5] * darSrcMtx[6]))) + (darSrcMtx[2] * ((darSrcMtx[3] * darSrcMtx[7]) - (darSrcMtx[4] * darSrcMtx[6])));
    double darInvSrcMtx[9] =
    {
        ((darSrcMtx[4] * darSrcMtx[8]) - (darSrcMtx[5] * darSrcMtx[7])) / dDet, ((darSrcMtx[2] * darSrcMtx[7]) - (darSrcMtx[1] * darSrcMtx[8])) / dDet, ((darSrcMtx[1] * darSrcMtx[5]) - (darSrcMtx[2] * darSrcMtx[4])) / dDet,
        ((darSrcMtx[5] * darSrcMtx[6]) - (darSrcMtx[3] * darSrcMtx[8])) / dDet, ((darSrcMtx[0] * darSrcMtx[8]) - (darSrcMtx[2] * darSrcMtx[6])) / dDet, ((darSrcMtx[2] * darSrcMtx[3]) - (darSrcMtx[0] * darSrcMtx[5])) / dDet,
        ((darSrcMtx[3] * darSrcMtx[7]) - (darSrcMtx[4] * darSrcMtx[6])) / dDet, ((darSrcMtx[1] * darSrcMtx[6]) - (darSrcMtx[0] * darSrcMtx[7])) / dDet, ((darSrcMtx[0] * darSrcMtx[4]) - (darSrcMtx[1] * darSrcMtx[3])) / dDet
    };
    dDet = (darTgMtx[0] * ((darTgMtx[4] * darTgMtx[8]) - (darTgMtx[5] * darTgMtx[7]))) - (darTgMtx[1] * ((darTgMtx[3] * darTgMtx[8]) - (darTgMtx[5] * darTgMtx[6]))) + (darTgMtx[2] * ((darTgMtx[3] * darTgMtx[7]) - (darTgMtx[4] * darTgMtx[6])));
    double darInvTgMtx[9] =
    {
        ((darTgMtx[4] * darTgMtx[8]) - (darTgMtx[5] * darTgMtx[7])) / dDet, ((darTgMtx[2] * darTgMtx[7]) - (darTgMtx[1] * darTgMtx[8])) / dDet, ((darTgMtx[1] * darTgMtx[5]) - (darTgMtx[2] * darTgMtx[4])) / dDet,
        ((darTgMtx[5] * darTgMtx[6]) - (darTgMtx[3] * darTgMtx[8])) / dDet, ((darTgMtx[0] * darTgMtx[8]) - (darTgMtx[2] * darTgMtx[6])) / dDet, ((darTgMtx[2] * darTgMtx[3]) - (darTgMtx[0] * darTgMtx[5])) / dDet,
        ((darTgMtx[3] * darTgMtx[7]) - (darTgMtx[4] * darTgMtx[6])) / dDet, ((darTgMtx[1] * darTgMtx[6]) - (darTgMtx[0] * darTgMtx[7])) / dDet, ((darTgMtx[0] * darTgMtx[4]) - (darTgMtx[1] * darTgMtx[3])) / dDet
    };

    // Transform to RGB Matrix
    double dXw = (sSrcClrGamutCoor.dWx / sSrcClrGamutCoor.dWy * sSrcClrGamutCoor.dW_Y);
    double dYw = sSrcClrGamutCoor.dW_Y;
    double dZw = (1 - sSrcClrGamutCoor.dWx - sSrcClrGamutCoor.dWy) / sSrcClrGamutCoor.dWy * sSrcClrGamutCoor.dW_Y;
    double dSr = (darInvSrcMtx[0] * dXw) + (darInvSrcMtx[1] * dYw) + (darInvSrcMtx[2] * dZw);
    double dSg = (darInvSrcMtx[3] * dXw) + (darInvSrcMtx[4] * dYw) + (darInvSrcMtx[5] * dZw);
    double dSb = (darInvSrcMtx[6] * dXw) + (darInvSrcMtx[7] * dYw) + (darInvSrcMtx[8] * dZw);
    for(uint8 i=0; i<9; i++)
        darSrcMtx[i] = darSrcMtx[i] * ((i%3 == 0)? dSr : (i%3 == 1)? dSg : dSb);

    dXw = (sTgClrGamutCoor.dWx / sTgClrGamutCoor.dWy * sTgClrGamutCoor.dW_Y);
    dYw = sTgClrGamutCoor.dW_Y;
    dZw = (1 - sTgClrGamutCoor.dWx - sTgClrGamutCoor.dWy) / sTgClrGamutCoor.dWy * sTgClrGamutCoor.dW_Y;
    dSr = (darInvTgMtx[0] * dXw) + (darInvTgMtx[1] * dYw) + (darInvTgMtx[2] * dZw);
    dSg = (darInvTgMtx[3] * dXw) + (darInvTgMtx[4] * dYw) + (darInvTgMtx[5] * dZw);
    dSb = (darInvTgMtx[6] * dXw) + (darInvTgMtx[7] * dYw) + (darInvTgMtx[8] * dZw);
    for(uint8 i=0; i<9; i++)
        darTgMtx[i] = darTgMtx[i] * ((i%3 == 0)? dSr : (i%3 == 1)? dSg : dSb);

    // Inverse Target RGB Matrix
    dDet = (darTgMtx[0] * ((darTgMtx[4] * darTgMtx[8]) - (darTgMtx[5] * darTgMtx[7]))) - (darTgMtx[1] * ((darTgMtx[3] * darTgMtx[8]) - (darTgMtx[5] * darTgMtx[6]))) + (darTgMtx[2] * ((darTgMtx[3] * darTgMtx[7]) - (darTgMtx[4] * darTgMtx[6])));
    darInvTgMtx[0] = ((darTgMtx[4] * darTgMtx[8]) - (darTgMtx[5] * darTgMtx[7])) / dDet;
    darInvTgMtx[1] = ((darTgMtx[2] * darTgMtx[7]) - (darTgMtx[1] * darTgMtx[8])) / dDet;
    darInvTgMtx[2] = ((darTgMtx[1] * darTgMtx[5]) - (darTgMtx[2] * darTgMtx[4])) / dDet;
    darInvTgMtx[3] = ((darTgMtx[5] * darTgMtx[6]) - (darTgMtx[3] * darTgMtx[8])) / dDet;
    darInvTgMtx[4] = ((darTgMtx[0] * darTgMtx[8]) - (darTgMtx[2] * darTgMtx[6])) / dDet;
    darInvTgMtx[5] = ((darTgMtx[2] * darTgMtx[3]) - (darTgMtx[0] * darTgMtx[5])) / dDet;
    darInvTgMtx[6] = ((darTgMtx[3] * darTgMtx[7]) - (darTgMtx[4] * darTgMtx[6])) / dDet;
    darInvTgMtx[7] = ((darTgMtx[1] * darTgMtx[6]) - (darTgMtx[0] * darTgMtx[7])) / dDet;
    darInvTgMtx[8] = ((darTgMtx[0] * darTgMtx[4]) - (darTgMtx[1] * darTgMtx[3])) / dDet;

    // Inverse RGB Target * Source RGB Matrix
    darMtx[0] = darInvTgMtx[0] * darSrcMtx[0] + darInvTgMtx[1] * darSrcMtx[3] + darInvTgMtx[2] * darSrcMtx[6];
    darMtx[1] = darInvTgMtx[0] * darSrcMtx[1] + darInvTgMtx[1] * darSrcMtx[4] + darInvTgMtx[2] * darSrcMtx[7];
    darMtx[2] = darInvTgMtx[0] * darSrcMtx[2] + darInvTgMtx[1] * darSrcMtx[5] + darInvTgMtx[2] * darSrcMtx[8];
    darMtx[3] = darInvTgMtx[3] * darSrcMtx[0] + darInvTgMtx[4] * darSrcMtx[3] + darInvTgMtx[5] * darSrcMtx[6];
    darMtx[4] = darInvTgMtx[3] * darSrcMtx[1] + darInvTgMtx[4] * darSrcMtx[4] + darInvTgMtx[5] * darSrcMtx[7];
    darMtx[5] = darInvTgMtx[3] * darSrcMtx[2] + darInvTgMtx[4] * darSrcMtx[5] + darInvTgMtx[5] * darSrcMtx[8];
    darMtx[6] = darInvTgMtx[6] * darSrcMtx[0] + darInvTgMtx[7] * darSrcMtx[3] + darInvTgMtx[8] * darSrcMtx[6];
    darMtx[7] = darInvTgMtx[6] * darSrcMtx[1] + darInvTgMtx[7] * darSrcMtx[4] + darInvTgMtx[8] * darSrcMtx[7];
    darMtx[8] = darInvTgMtx[6] * darSrcMtx[2] + darInvTgMtx[7] * darSrcMtx[5] + darInvTgMtx[8] * darSrcMtx[8];

    // clip bits
    for(uint8 i=0; i<9; i++)
    {
        int16 u16Temp = (int16)(darMtx[i] * 16384);
        darMtx[i] = ((double)u16Temp / 16384);
    }

    return rcSUCCESS;
}

/**
 * @brief Color Gamut Coordinate Transform And Set
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] sSrcClrGamutCoor : source color gamut coordinate
 * @param [in] sTgClrGamutCoor : target color gamut coordinate
 * @param [in] bHdrCscEn
 *          -false : disable HDR CSC transform
 *          -true  : enable HDR CSC transform
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdrCscCoorTransSet(SclEntity eEntity, CIE_Coor sSrcClrGamutCoor, CIE_Coor sTgClrGamutCoor, bool bHdrCscEn)
{
    int status = rcSUCCESS;

    double dMtx[9];
    uint08 u8data[27];

    status &= dvProAV_HdrCscTransMtx(sSrcClrGamutCoor, sTgClrGamutCoor, &dMtx[0]);

    for (int i=0 ; i<9 ; i++)
    {
        double coeff = dMtx[i];
        uint32 u32data;
        if (coeff >= 0)
        {
            coeff *= pow(2.0, 16);
        }
        else
        {
            coeff *= pow(2.0, 16) * (-1);
            coeff += pow(2.0, 18); // 18 for 2bit(18-16) integer
        }
        u32data = (uint32)(coeff);

        u8data[i * 3 + 2] = (u32data >> 16) & 0xFF;
        u8data[i * 3 + 1] = (u32data >>  8) & 0xFF;
        u8data[i * 3 + 0] = (u32data      ) & 0xFF;
    }

    status &= dvProAV_SclVopHdrCscSet(eEntity, u8data);

    if(bHdrCscEn)
    {
        // Load Gamma 1
        status &= dvProAV_SclVopHdrGammaDl2Chip((SclEntity)eEntity, eHdrGamma1, dvProAV_HdrGammaTableGet(eHdrGamma1)); // Gamma 1 Size : 1024 *4
        // Load Gamma 3
        status &= dvProAV_SclVopHdrGammaDl2Chip((SclEntity)eEntity, eHdrGamma3, dvProAV_HdrGammaTableGet(eHdrGamma3)); // Gamma 3 Size : 1024 *4
    }
    status &= dvProAV_SclVopHdrGammaEnableSet((SclEntity)eEntity, eHdrGamma1, bHdrCscEn);
    status &= dvProAV_SclVopHdrGammaEnableSet((SclEntity)eEntity, eHdrGamma3, bHdrCscEn);
    status &= dvProAV_SclVopHdrCscEn((SclEntity)eEntity, bHdrCscEn);

    return status;
}

/**
 * @brief Enable The HDR CSC Transform
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] eCscType: HDR csc type index
 *          -eHdrCsc2020to709
 *          -eHdrCsc2020toAdobe
 *          -eHdrCsc2020toP3
 *          -eHdrCscP3to709
 *          -eHdrCscP3Adobe709
 * @param [in] bHdrCscEn
 *          -false : disable HDR CSC transform
 *          -true  : enable HDR CSC transform
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdrCscTransEn(SclEntity eEntity, HdrCscType eCscType, bool bHdrCscEn)
{
    int status = rcSUCCESS;

    if(bHdrCscEn)
    {
        // Load Gamma 1
        status &= dvProAV_SclVopHdrGammaDl2Chip((SclEntity)eEntity, eHdrGamma1, dvProAV_HdrGammaTableGet(eHdrGamma1)); // Gamma 1 Size : 1024 *4
        // Load CSC
        status &= dvProAV_SclVopHdrCscByIndexSet((SclEntity)eEntity, eCscType);
        // Load Gamma 3
        status &= dvProAV_SclVopHdrGammaDl2Chip((SclEntity)eEntity, eHdrGamma3, dvProAV_HdrGammaTableGet(eHdrGamma3)); // Gamma 3 Size : 1024 *4
    }
    status &= dvProAV_SclVopHdrGammaEnableSet((SclEntity)eEntity, eHdrGamma1, bHdrCscEn);
    status &= dvProAV_SclVopHdrGammaEnableSet((SclEntity)eEntity, eHdrGamma3, bHdrCscEn);
    status &= dvProAV_SclVopHdrCscEn((SclEntity)eEntity, bHdrCscEn);

    return status;
}

/**
 * @brief Enable The HDR Tone Mapping Gamma Curve
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] eHdr4GammaType
 *          -eHdrGamma4_1
 *          -eHdrGamma4_2
 *          -eHdrGamma4_3
 *          -eHdrGamma4_4
 *          -eHdrGamma4_Auto
 * @param [in] sHdrParam : when eHdr4GammaType=eHdrGamma4_Auto, calculation parameters that need to be used
 * @param [in] bToneMappingEn
 *          -false : disable HDR Tone Mapping Gamma Curve
 *          -true  : enable HDR Tone Mapping Gamma Curve
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_HdrToneMappingEn(SclEntity eEntity, HDR_GAMMA_TYPE eHdr4GammaType, PQ_HDR_PARAM sHdrParam, bool bToneMappingEn)
{
    int status = rcSUCCESS;
    if((eHdr4GammaType >= eHdrGammaTypeMax) || (eHdr4GammaType < eHdrGamma4_1) || (eEntity >= eSclEntity_Max))
    {
        dvProAV_SclVopHdrGammaEnableSet(eEntity, eHdrGamma4, bToneMappingEn);
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    if(bToneMappingEn)
    {
        status &= dvProAV_SclVopHdrGammaDl2Chip(eEntity, eHdrGamma4, (eHdr4GammaType > eHdrGamma4_4)? dvProAV_HdrPq2SdrGammaCalculate(sHdrParam) : dvProAV_HdrGammaTableGet(eHdr4GammaType));   // Gamma 4 Size : 1024 *4
    }
    status &= dvProAV_SclVopHdrGammaEnableSet(eEntity, eHdrGamma4, bToneMappingEn);
    return status;
}

//-------------------------------------------------------------------------------------------------
// HDR 10+
//-------------------------------------------------------------------------------------------------
/**
 * @brief Hdr10+ Tone Mapping Enable
 * @param [in] eEntity
 * @param [in] bEnable
 *          -false : disable HDR Tone Mapping Gamma Curve
 *          -true  : enable HDR Tone Mapping Gamma Curve
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pEn(SclEntity eEntity, bool bEnable)
{
    return dvProAV_AccessWrite(eVopmHdrG4GammaEn + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), bEnable);
}

/**
 * @brief Hdr10+ Mode select
 * @param [in] eEntity : channel select
 * @param [in] eMode : mode select
 *          -eHDR10pMode_ForceReg,               // luminance & max level use register value
 *          -eHDR10pMode_InfoLum,                // get infoframe luminance & max level use register value
 *          -eHDR10pMode_InfoLumWithLevelDetect, // get infoframe luminance & max level detect
 *          -eHDR10pMode_RegLumWithLevelDetect,  // luminance use register value & max level detect
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pMode(SclEntity eEntity, eHDR10pMode eMode)
{
    return dvProAV_AccessWrite(eVopmHdrG4MODE + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), eMode);
}

/**
 * @brief Hdr10+ Signel Maximal Luminance Set
 * @param [in] eEntity : channel select
 * @param [in] u16SglMaxLum : DisMaxLum~16383
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pSglMaxLum(SclEntity eEntity, uint16 u16SglMaxLum)
{
    int status = rcSUCCESS;
    uint32 u32DisMaxLum = 0;

    status &= dvProAV_AccessRead(eVopmHdrG4Display_Max + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), &u32DisMaxLum);
    if (u16SglMaxLum < ((uint16)u32DisMaxLum))
        u16SglMaxLum = (uint16)u32DisMaxLum;
    if (u16SglMaxLum > 16383)
        u16SglMaxLum = 16383;
    status &= dvProAV_AccessWrite(eVopmHdrG4Signal_Max + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), u16SglMaxLum);

    return status;
}

/**
 * @brief Hdr10+ Signel Minimal Luminance Set
 * @param [in] eEntity : channel select
 * @param [in] fSglMinLum : 0 ~ 0.9999
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pSglMinLum(SclEntity eEntity, float fSglMinLum)
{
    if(fSglMinLum > 0.999)
        fSglMinLum = 0.999;
    if(fSglMinLum < 0)
        fSglMinLum = 0;
    uint16 u16SglMinLum = fSglMinLum * 8192;

    return dvProAV_AccessWrite(eVopmHdrG4Signal_Min + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), u16SglMinLum);
}

/**
 * @brief Hdr10+ Display Maximal Luminance Set
 * @param [in] eEntity : channel select
 * @param [in] u16DisMaxLum : 100 ~ 10000
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pDisMaxLum(SclEntity eEntity, uint16 u16DisMaxLum)
{
    if(u16DisMaxLum > 10000)
        u16DisMaxLum = 10000;
    if (u16DisMaxLum < 100)
        u16DisMaxLum = 100;
    return dvProAV_AccessWrite(eVopmHdrG4Display_Max + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), u16DisMaxLum);
}

/**
 * @brief Hdr10+ Display Minimal Luminance Set
 * @param [in] eEntity : channel select
 * @param [in] fDisMinLum : 0 ~ 0.9999
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pDisMinLum(SclEntity eEntity, float fDisMinLum)
{
    if(fDisMinLum > 0.999)
        fDisMinLum = 0.999;
    if(fDisMinLum < 0)
        fDisMinLum = 0;
    uint16 u16DisMinLum = fDisMinLum * 8192;

    return dvProAV_AccessWrite(eVopmHdrG4Display_Min + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), u16DisMinLum);
}

/**
 * @brief Hdr10+ HDR Gamma Set
 * @param [in] eEntity : channel select
 * @param [in] fGamma : 0.357 ~ 2.8
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pHdrGamma(SclEntity eEntity, float fGamma)
{
    if(fGamma < 0.357)
        fGamma = 0.357;
    if(fGamma > 2.8)
        fGamma = 2.8;
    uint16 u16Gamma = fGamma * 1024;
    return dvProAV_AccessWrite(eVopmHdrG4Gamma + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), u16Gamma);
}

/**
 * @brief Hdr10+ Frame Max Level Set
 * @param [in] eEntity : channel select
 * @param [in] u16MaxLevel : 256 ~ 1023
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pMaxLevelSet(SclEntity eEntity, uint16 u16MaxLevel)
{
    if(u16MaxLevel > 1023)
        u16MaxLevel = 1023;
    if(u16MaxLevel < 256)
        u16MaxLevel = 256;
    return dvProAV_AccessWrite(eVopmHdrG4FrameMaxLevel + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), u16MaxLevel);
}

/**
 * @brief Hdr10+ Frame Max Level Min Set
 * @param [in] eEntity : channel select
 * @param [in] u16MaxLevelMin : 256 ~ 1023
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pMaxLevelMinSet(SclEntity eEntity, uint16 u16MaxLevelMin)
{
    if(u16MaxLevelMin > 1023)
        u16MaxLevelMin = 1023;
    if(u16MaxLevelMin < 256)
        u16MaxLevelMin = 256;
    return dvProAV_AccessWrite(eVopmHdrG4MaxLevelMin + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), u16MaxLevelMin);
}

/**
 * @brief Hdr10+ Frame Adapt Level Set
 * @param [in] eEntity : channel select
 * @param [in] u8Inc : 0~255
 * @param [in] u8Dec : 0~255
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pFrameAdaptSet(SclEntity eEntity, uint8 u8Inc, uint8 u8Dec)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eVopmHdrG4IncLim + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), u8Inc);
    status &= dvProAV_AccessWrite(eVopmHdrG4DecLim + ((eEntity == eSclEntity_Main)? 0 : (eVopsHdrG1GamaAdr - eVopmHdrG1GamaAdr)), u8Dec);

    return status;
}

/**
 * @brief Hdr10+ Frame Detect Area Set
 * @param [in] u16StLine : start line
 * @param [in] u16EndLine : end line
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pFrameDecAreaSet(uint16 u16StLine, uint16 u16EndLine)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eIpmHdrMaxDetVst, u16StLine);
    status &= dvProAV_AccessWrite(eIpmHdrMaxDetHeight, u16EndLine);

    return status;
}

/**
 * @brief HDR Tone Mapping Config Set
 * @param [in] eEntity :  channel select
 *          -eSclEntity_Main
 *          -eSclEntity_Sub
 * @param [in] eHdrType
 *          -eHDRType_Off
 *          -eHDRType_10
 *          -eHDRType_10p
 *          -eHDRType_Sdr10p
 * @param [in] eLevel :
 *          -eHDRLevel_1
 *          -eHDRLevel_2
 *          -eHDRLevel_3
 *          -eHDRLevel_4
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Hdr10pToneMappingSet(SclEntity eEntity, eHDR_TYPE eHdrType, eHDR_LEVEL eHdrLevel)
{
    int status = rcSUCCESS;

    if(eHdrType != eHDRType_Off)
    {
        status &= dvProAV_Hdr10pMode(eEntity, sHdrCfgParam[eHdrType - 1][eHdrLevel].eMode);
        status &= dvProAV_Hdr10pDisMaxLum(eEntity, sHdrCfgParam[eHdrType - 1][eHdrLevel].u16DisMaxLum);
        status &= dvProAV_Hdr10pDisMinLum(eEntity, sHdrCfgParam[eHdrType - 1][eHdrLevel].fDisMinLum);
        status &= dvProAV_Hdr10pSglMaxLum(eEntity, sHdrCfgParam[eHdrType - 1][eHdrLevel].u16SglMaxLum);
        status &= dvProAV_Hdr10pSglMinLum(eEntity, sHdrCfgParam[eHdrType - 1][eHdrLevel].fSglMinxLum);
        status &= dvProAV_Hdr10pHdrGamma(eEntity, sHdrCfgParam[eHdrType - 1][eHdrLevel].fHdrGamma);
        status &= dvProAV_Hdr10pMaxLevelSet(eEntity, sHdrCfgParam[eHdrType - 1][eHdrLevel].u16MaxLevel);
        status &= dvProAV_Hdr10pMaxLevelMinSet(eEntity, sHdrCfgParam[eHdrType - 1][eHdrLevel].u16MaxLevelMin);
        status &= dvProAV_Hdr10pFrameAdaptSet(eEntity, sHdrCfgParam[eHdrType - 1][eHdrLevel].u8FrameAdaptUp, sHdrCfgParam[eHdrType - 1][eHdrLevel].u8FrameAdaptDown);
    }
    status &= dvProAV_Hdr10pEn(eEntity, (eHdrType == eHDRType_Off)? false : true);

    return status;
}
