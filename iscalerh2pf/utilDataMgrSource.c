// ===============================================================================
// FILE NAME: utilDataMgrSource.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2021/12/24, Larry Create
// --------------------
// ===============================================================================

#include "utilDataMgrSourceTable.h"
#include "utilDataMapping.h"

static sSOURCE_PARA_TABLE m_sSourceTable = {NULL};

// ==============================================================================
// FUNCTION NAME: utilDataMgrSourceInit
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/12/24, Larry Create
// --------------------
// ==============================================================================
void utilDataMgrSourceInit(sSOURCE_PARA_TABLE sSourceTable)
{
    m_sSourceTable.p_sSourceData = sSourceTable.p_sSourceData;
    m_sSourceTable.p_sWAPData    = sSourceTable.p_sWAPData;

}

// ==============================================================================
// FUNCTION NAME: utilDataMgrSource_ParaTableGet
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/12/24, Larry Create
// --------------------
// ==============================================================================
sPARA_FORMAT* utilDataMgrSource_ParaTableGet(UINT16 uiNode)
{
    sPARA_FORMAT *psPara = NULL;
    UINT16 uiNodeTemp = 0;

    if(m_sSourceTable.p_sSourceData == NULL)
    {
        return NULL;
    }

    switch(uiNode)
    {
        case eDATA_NODE_SOURCE0_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_RESERVED].sCommon;
            break;

        case eDATA_NODE_SOURCE0_HSG_PRESENTATION:
        case eDATA_NODE_SOURCE0_HSG_VIDEO:
        case eDATA_NODE_SOURCE0_HSG_BRIGHT:
        case eDATA_NODE_SOURCE0_HSG_ENHANCED:
        case eDATA_NODE_SOURCE0_HSG_REC709:
        case eDATA_NODE_SOURCE0_HSG_REAL:
        case eDATA_NODE_SOURCE0_HSG_DICOMSIM:
        case eDATA_NODE_SOURCE0_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_SOURCE0_HSG_3D:
        case eDATA_NODE_SOURCE0_HSG_BLENDING:
        case eDATA_NODE_SOURCE0_HSG_USER:
        case eDATA_NODE_SOURCE0_HSG_HDR:
        case eDATA_NODE_SOURCE0_HSG_SRGB:
        case eDATA_NODE_SOURCE0_HSG_SUPER_BRIGHT:
        case eDATA_NODE_SOURCE0_HSG_SUPER_RED:
        case eDATA_NODE_SOURCE0_HSG_3D_PASSIVE:
        case eDATA_NODE_SOURCE0_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SOURCE0_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_RESERVED].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_SOURCE0_COLOR_PRESENTATION:
        case eDATA_NODE_SOURCE0_COLOR_VIDEO:
        case eDATA_NODE_SOURCE0_COLOR_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_ENHANCED:
        case eDATA_NODE_SOURCE0_COLOR_REC709:
        case eDATA_NODE_SOURCE0_COLOR_REAL:
        case eDATA_NODE_SOURCE0_COLOR_DICOMSIM:
        case eDATA_NODE_SOURCE0_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_SOURCE0_COLOR_3D:
        case eDATA_NODE_SOURCE0_COLOR_BLENDING:
        case eDATA_NODE_SOURCE0_COLOR_USER:
        case eDATA_NODE_SOURCE0_COLOR_HDR:
        case eDATA_NODE_SOURCE0_COLOR_SRGB:
        case eDATA_NODE_SOURCE0_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_SUPER_RED:
        case eDATA_NODE_SOURCE0_COLOR_3D_PASSIVE:
        case eDATA_NODE_SOURCE0_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SOURCE0_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_RESERVED].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_VGA_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_VGA].sCommon;
            break;

        case eDATA_NODE_VGA_HSG_PRESENTATION:
        case eDATA_NODE_VGA_HSG_VIDEO:
        case eDATA_NODE_VGA_HSG_BRIGHT:
        case eDATA_NODE_VGA_HSG_ENHANCED:
        case eDATA_NODE_VGA_HSG_REC709:
        case eDATA_NODE_VGA_HSG_REAL:
        case eDATA_NODE_VGA_HSG_DICOMSIM:
        case eDATA_NODE_VGA_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_HSG_3D:
        case eDATA_NODE_VGA_HSG_BLENDING:
        case eDATA_NODE_VGA_HSG_USER:
        case eDATA_NODE_VGA_HSG_HDR:
        case eDATA_NODE_VGA_HSG_SRGB:
        case eDATA_NODE_VGA_HSG_SUPER_BRIGHT:
        case eDATA_NODE_VGA_HSG_SUPER_RED:
        case eDATA_NODE_VGA_HSG_3D_PASSIVE:
        case eDATA_NODE_VGA_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_VGA_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_VGA].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_VGA_COLOR_PRESENTATION:
        case eDATA_NODE_VGA_COLOR_VIDEO:
        case eDATA_NODE_VGA_COLOR_BRIGHT:
        case eDATA_NODE_VGA_COLOR_ENHANCED:
        case eDATA_NODE_VGA_COLOR_REC709:
        case eDATA_NODE_VGA_COLOR_REAL:
        case eDATA_NODE_VGA_COLOR_DICOMSIM:
        case eDATA_NODE_VGA_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_COLOR_3D:
        case eDATA_NODE_VGA_COLOR_BLENDING:
        case eDATA_NODE_VGA_COLOR_USER:
        case eDATA_NODE_VGA_COLOR_HDR:
        case eDATA_NODE_VGA_COLOR_SRGB:
        case eDATA_NODE_VGA_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_VGA_COLOR_SUPER_RED:
        case eDATA_NODE_VGA_COLOR_3D_PASSIVE:
        case eDATA_NODE_VGA_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_VGA_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_VGA].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_BNC_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_BNC].sCommon;
            break;

        case eDATA_NODE_BNC_HSG_PRESENTATION:
        case eDATA_NODE_BNC_HSG_VIDEO:
        case eDATA_NODE_BNC_HSG_BRIGHT:
        case eDATA_NODE_BNC_HSG_ENHANCED:
        case eDATA_NODE_BNC_HSG_REC709:
        case eDATA_NODE_BNC_HSG_REAL:
        case eDATA_NODE_BNC_HSG_DICOMSIM:
        case eDATA_NODE_BNC_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_HSG_3D:
        case eDATA_NODE_BNC_HSG_BLENDING:
        case eDATA_NODE_BNC_HSG_USER:
        case eDATA_NODE_BNC_HSG_HDR:
        case eDATA_NODE_BNC_HSG_SRGB:
        case eDATA_NODE_BNC_HSG_SUPER_BRIGHT:
        case eDATA_NODE_BNC_HSG_SUPER_RED:
        case eDATA_NODE_BNC_HSG_3D_PASSIVE:
        case eDATA_NODE_BNC_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_BNC_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_BNC].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_BNC_COLOR_PRESENTATION:
        case eDATA_NODE_BNC_COLOR_VIDEO:
        case eDATA_NODE_BNC_COLOR_BRIGHT:
        case eDATA_NODE_BNC_COLOR_ENHANCED:
        case eDATA_NODE_BNC_COLOR_REC709:
        case eDATA_NODE_BNC_COLOR_REAL:
        case eDATA_NODE_BNC_COLOR_DICOMSIM:
        case eDATA_NODE_BNC_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_COLOR_3D:
        case eDATA_NODE_BNC_COLOR_BLENDING:
        case eDATA_NODE_BNC_COLOR_USER:
        case eDATA_NODE_BNC_COLOR_HDR:
        case eDATA_NODE_BNC_COLOR_SRGB:
        case eDATA_NODE_BNC_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_BNC_COLOR_SUPER_RED:
        case eDATA_NODE_BNC_COLOR_3D_PASSIVE:
        case eDATA_NODE_BNC_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_BNC_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_BNC].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDMI1_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDMI1].sCommon;
            break;

        case eDATA_NODE_HDMI1_HSG_PRESENTATION:
        case eDATA_NODE_HDMI1_HSG_VIDEO:
        case eDATA_NODE_HDMI1_HSG_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_ENHANCED:
        case eDATA_NODE_HDMI1_HSG_REC709:
        case eDATA_NODE_HDMI1_HSG_REAL:
        case eDATA_NODE_HDMI1_HSG_DICOMSIM:
        case eDATA_NODE_HDMI1_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_HSG_3D:
        case eDATA_NODE_HDMI1_HSG_BLENDING:
        case eDATA_NODE_HDMI1_HSG_USER:
        case eDATA_NODE_HDMI1_HSG_HDR:
        case eDATA_NODE_HDMI1_HSG_SRGB:
        case eDATA_NODE_HDMI1_HSG_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_SUPER_RED:
        case eDATA_NODE_HDMI1_HSG_3D_PASSIVE:
        case eDATA_NODE_HDMI1_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDMI1_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDMI1].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDMI1_COLOR_PRESENTATION:
        case eDATA_NODE_HDMI1_COLOR_VIDEO:
        case eDATA_NODE_HDMI1_COLOR_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_ENHANCED:
        case eDATA_NODE_HDMI1_COLOR_REC709:
        case eDATA_NODE_HDMI1_COLOR_REAL:
        case eDATA_NODE_HDMI1_COLOR_DICOMSIM:
        case eDATA_NODE_HDMI1_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_COLOR_3D:
        case eDATA_NODE_HDMI1_COLOR_BLENDING:
        case eDATA_NODE_HDMI1_COLOR_USER:
        case eDATA_NODE_HDMI1_COLOR_HDR:
        case eDATA_NODE_HDMI1_COLOR_SRGB:
        case eDATA_NODE_HDMI1_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_SUPER_RED:
        case eDATA_NODE_HDMI1_COLOR_3D_PASSIVE:
        case eDATA_NODE_HDMI1_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDMI1_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDMI1].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDMI2_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDMI2].sCommon;
            break;

        case eDATA_NODE_HDMI2_HSG_PRESENTATION:
        case eDATA_NODE_HDMI2_HSG_VIDEO:
        case eDATA_NODE_HDMI2_HSG_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_ENHANCED:
        case eDATA_NODE_HDMI2_HSG_REC709:
        case eDATA_NODE_HDMI2_HSG_REAL:
        case eDATA_NODE_HDMI2_HSG_DICOMSIM:
        case eDATA_NODE_HDMI2_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_HSG_3D:
        case eDATA_NODE_HDMI2_HSG_BLENDING:
        case eDATA_NODE_HDMI2_HSG_USER:
        case eDATA_NODE_HDMI2_HSG_HDR:
        case eDATA_NODE_HDMI2_HSG_SRGB:
        case eDATA_NODE_HDMI2_HSG_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_SUPER_RED:
        case eDATA_NODE_HDMI2_HSG_3D_PASSIVE:
        case eDATA_NODE_HDMI2_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDMI2_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDMI2].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDMI2_COLOR_PRESENTATION:
        case eDATA_NODE_HDMI2_COLOR_VIDEO:
        case eDATA_NODE_HDMI2_COLOR_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_ENHANCED:
        case eDATA_NODE_HDMI2_COLOR_REC709:
        case eDATA_NODE_HDMI2_COLOR_REAL:
        case eDATA_NODE_HDMI2_COLOR_DICOMSIM:
        case eDATA_NODE_HDMI2_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_COLOR_3D:
        case eDATA_NODE_HDMI2_COLOR_BLENDING:
        case eDATA_NODE_HDMI2_COLOR_USER:
        case eDATA_NODE_HDMI2_COLOR_HDR:
        case eDATA_NODE_HDMI2_COLOR_SRGB:
        case eDATA_NODE_HDMI2_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_SUPER_RED:
        case eDATA_NODE_HDMI2_COLOR_3D_PASSIVE:
        case eDATA_NODE_HDMI2_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDMI2_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDMI2].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_DVID_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_DVI].sCommon;
            break;

        case eDATA_NODE_DVID_HSG_PRESENTATION:
        case eDATA_NODE_DVID_HSG_VIDEO:
        case eDATA_NODE_DVID_HSG_BRIGHT:
        case eDATA_NODE_DVID_HSG_ENHANCED:
        case eDATA_NODE_DVID_HSG_REC709:
        case eDATA_NODE_DVID_HSG_REAL:
        case eDATA_NODE_DVID_HSG_DICOMSIM:
        case eDATA_NODE_DVID_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_HSG_3D:
        case eDATA_NODE_DVID_HSG_BLENDING:
        case eDATA_NODE_DVID_HSG_USER:
        case eDATA_NODE_DVID_HSG_HDR:
        case eDATA_NODE_DVID_HSG_SRGB:
        case eDATA_NODE_DVID_HSG_SUPER_BRIGHT:
        case eDATA_NODE_DVID_HSG_SUPER_RED:
        case eDATA_NODE_DVID_HSG_3D_PASSIVE:
        case eDATA_NODE_DVID_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_DVID_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_DVI].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_DVID_COLOR_PRESENTATION:
        case eDATA_NODE_DVID_COLOR_VIDEO:
        case eDATA_NODE_DVID_COLOR_BRIGHT:
        case eDATA_NODE_DVID_COLOR_ENHANCED:
        case eDATA_NODE_DVID_COLOR_REC709:
        case eDATA_NODE_DVID_COLOR_REAL:
        case eDATA_NODE_DVID_COLOR_DICOMSIM:
        case eDATA_NODE_DVID_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_COLOR_3D:
        case eDATA_NODE_DVID_COLOR_BLENDING:
        case eDATA_NODE_DVID_COLOR_USER:
        case eDATA_NODE_DVID_COLOR_HDR:
        case eDATA_NODE_DVID_COLOR_SRGB:
        case eDATA_NODE_DVID_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_DVID_COLOR_SUPER_RED:
        case eDATA_NODE_DVID_COLOR_3D_PASSIVE:
        case eDATA_NODE_DVID_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_DVID_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_DVI].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_DP_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_DISPLAYPORT].sCommon;
            break;

        case eDATA_NODE_DP_HSG_PRESENTATION:
        case eDATA_NODE_DP_HSG_VIDEO:
        case eDATA_NODE_DP_HSG_BRIGHT:
        case eDATA_NODE_DP_HSG_ENHANCED:
        case eDATA_NODE_DP_HSG_REC709:
        case eDATA_NODE_DP_HSG_REAL:
        case eDATA_NODE_DP_HSG_DICOMSIM:
        case eDATA_NODE_DP_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_DP_HSG_3D:
        case eDATA_NODE_DP_HSG_BLENDING:
        case eDATA_NODE_DP_HSG_USER:
        case eDATA_NODE_DP_HSG_HDR:
        case eDATA_NODE_DP_HSG_SRGB:
        case eDATA_NODE_DP_HSG_SUPER_BRIGHT:
        case eDATA_NODE_DP_HSG_SUPER_RED:
        case eDATA_NODE_DP_HSG_3D_PASSIVE:
        case eDATA_NODE_DP_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_DP_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_DISPLAYPORT].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_DP_COLOR_PRESENTATION:
        case eDATA_NODE_DP_COLOR_VIDEO:
        case eDATA_NODE_DP_COLOR_BRIGHT:
        case eDATA_NODE_DP_COLOR_ENHANCED:
        case eDATA_NODE_DP_COLOR_REC709:
        case eDATA_NODE_DP_COLOR_REAL:
        case eDATA_NODE_DP_COLOR_DICOMSIM:
        case eDATA_NODE_DP_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_DP_COLOR_3D:
        case eDATA_NODE_DP_COLOR_BLENDING:
        case eDATA_NODE_DP_COLOR_USER:
        case eDATA_NODE_DP_COLOR_HDR:
        case eDATA_NODE_DP_COLOR_SRGB:
        case eDATA_NODE_DP_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_DP_COLOR_SUPER_RED:
        case eDATA_NODE_DP_COLOR_3D_PASSIVE:
        case eDATA_NODE_DP_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_DP_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_DISPLAYPORT].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_3GSDI_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_3GSDI].sCommon;
            break;

        case eDATA_NODE_3GSDI_HSG_PRESENTATION:
        case eDATA_NODE_3GSDI_HSG_VIDEO:
        case eDATA_NODE_3GSDI_HSG_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_ENHANCED:
        case eDATA_NODE_3GSDI_HSG_REC709:
        case eDATA_NODE_3GSDI_HSG_REAL:
        case eDATA_NODE_3GSDI_HSG_DICOMSIM:
        case eDATA_NODE_3GSDI_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_HSG_3D:
        case eDATA_NODE_3GSDI_HSG_BLENDING:
        case eDATA_NODE_3GSDI_HSG_USER:
        case eDATA_NODE_3GSDI_HSG_HDR:
        case eDATA_NODE_3GSDI_HSG_SRGB:
        case eDATA_NODE_3GSDI_HSG_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_SUPER_RED:
        case eDATA_NODE_3GSDI_HSG_3D_PASSIVE:
        case eDATA_NODE_3GSDI_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_3GSDI_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_3GSDI].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_3GSDI_COLOR_PRESENTATION:
        case eDATA_NODE_3GSDI_COLOR_VIDEO:
        case eDATA_NODE_3GSDI_COLOR_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_ENHANCED:
        case eDATA_NODE_3GSDI_COLOR_REC709:
        case eDATA_NODE_3GSDI_COLOR_REAL:
        case eDATA_NODE_3GSDI_COLOR_DICOMSIM:
        case eDATA_NODE_3GSDI_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_COLOR_3D:
        case eDATA_NODE_3GSDI_COLOR_BLENDING:
        case eDATA_NODE_3GSDI_COLOR_USER:
        case eDATA_NODE_3GSDI_COLOR_HDR:
        case eDATA_NODE_3GSDI_COLOR_SRGB:
        case eDATA_NODE_3GSDI_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_SUPER_RED:
        case eDATA_NODE_3GSDI_COLOR_3D_PASSIVE:
        case eDATA_NODE_3GSDI_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_3GSDI_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_3GSDI].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDBASET_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDBASET].sCommon;
            break;

        case eDATA_NODE_HDBASET_HSG_PRESENTATION:
        case eDATA_NODE_HDBASET_HSG_VIDEO:
        case eDATA_NODE_HDBASET_HSG_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_ENHANCED:
        case eDATA_NODE_HDBASET_HSG_REC709:
        case eDATA_NODE_HDBASET_HSG_REAL:
        case eDATA_NODE_HDBASET_HSG_DICOMSIM:
        case eDATA_NODE_HDBASET_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_HSG_3D:
        case eDATA_NODE_HDBASET_HSG_BLENDING:
        case eDATA_NODE_HDBASET_HSG_USER:
        case eDATA_NODE_HDBASET_HSG_HDR:
        case eDATA_NODE_HDBASET_HSG_SRGB:
        case eDATA_NODE_HDBASET_HSG_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_SUPER_RED:
        case eDATA_NODE_HDBASET_HSG_3D_PASSIVE:
        case eDATA_NODE_HDBASET_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDBASET_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDBASET].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDBASET_COLOR_PRESENTATION:
        case eDATA_NODE_HDBASET_COLOR_VIDEO:
        case eDATA_NODE_HDBASET_COLOR_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_ENHANCED:
        case eDATA_NODE_HDBASET_COLOR_REC709:
        case eDATA_NODE_HDBASET_COLOR_REAL:
        case eDATA_NODE_HDBASET_COLOR_DICOMSIM:
        case eDATA_NODE_HDBASET_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_COLOR_3D:
        case eDATA_NODE_HDBASET_COLOR_BLENDING:
        case eDATA_NODE_HDBASET_COLOR_USER:
        case eDATA_NODE_HDBASET_COLOR_HDR:
        case eDATA_NODE_HDBASET_COLOR_SRGB:
        case eDATA_NODE_HDBASET_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_SUPER_RED:
        case eDATA_NODE_HDBASET_COLOR_3D_PASSIVE:
        case eDATA_NODE_HDBASET_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDBASET_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDBASET].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_12GSDI_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_12GSDI].sCommon;
            break;

        case eDATA_NODE_12GSDI_HSG_PRESENTATION:
        case eDATA_NODE_12GSDI_HSG_VIDEO:
        case eDATA_NODE_12GSDI_HSG_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_ENHANCED:
        case eDATA_NODE_12GSDI_HSG_REC709:
        case eDATA_NODE_12GSDI_HSG_REAL:
        case eDATA_NODE_12GSDI_HSG_DICOMSIM:
        case eDATA_NODE_12GSDI_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_HSG_3D:
        case eDATA_NODE_12GSDI_HSG_BLENDING:
        case eDATA_NODE_12GSDI_HSG_USER:
        case eDATA_NODE_12GSDI_HSG_HDR:
        case eDATA_NODE_12GSDI_HSG_SRGB:
        case eDATA_NODE_12GSDI_HSG_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_SUPER_RED:
        case eDATA_NODE_12GSDI_HSG_3D_PASSIVE:
        case eDATA_NODE_12GSDI_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_12GSDI_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_12GSDI].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_12GSDI_COLOR_PRESENTATION:
        case eDATA_NODE_12GSDI_COLOR_VIDEO:
        case eDATA_NODE_12GSDI_COLOR_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_ENHANCED:
        case eDATA_NODE_12GSDI_COLOR_REC709:
        case eDATA_NODE_12GSDI_COLOR_REAL:
        case eDATA_NODE_12GSDI_COLOR_DICOMSIM:
        case eDATA_NODE_12GSDI_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_COLOR_3D:
        case eDATA_NODE_12GSDI_COLOR_BLENDING:
        case eDATA_NODE_12GSDI_COLOR_USER:
        case eDATA_NODE_12GSDI_COLOR_HDR:
        case eDATA_NODE_12GSDI_COLOR_SRGB:
        case eDATA_NODE_12GSDI_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_SUPER_RED:
        case eDATA_NODE_12GSDI_COLOR_3D_PASSIVE:
        case eDATA_NODE_12GSDI_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_12GSDI_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_12GSDI].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_PRESENTER_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_PRESENTER].sCommon;
            break;

        case eDATA_NODE_PRESENTER_HSG_PRESENTATION:
        case eDATA_NODE_PRESENTER_HSG_VIDEO:
        case eDATA_NODE_PRESENTER_HSG_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_ENHANCED:
        case eDATA_NODE_PRESENTER_HSG_REC709:
        case eDATA_NODE_PRESENTER_HSG_REAL:
        case eDATA_NODE_PRESENTER_HSG_DICOMSIM:
        case eDATA_NODE_PRESENTER_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_HSG_3D:
        case eDATA_NODE_PRESENTER_HSG_BLENDING:
        case eDATA_NODE_PRESENTER_HSG_USER:
        case eDATA_NODE_PRESENTER_HSG_HDR:
        case eDATA_NODE_PRESENTER_HSG_SRGB:
        case eDATA_NODE_PRESENTER_HSG_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_SUPER_RED:
        case eDATA_NODE_PRESENTER_HSG_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_PRESENTER_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_PRESENTER].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_PRESENTER_COLOR_PRESENTATION:
        case eDATA_NODE_PRESENTER_COLOR_VIDEO:
        case eDATA_NODE_PRESENTER_COLOR_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_ENHANCED:
        case eDATA_NODE_PRESENTER_COLOR_REC709:
        case eDATA_NODE_PRESENTER_COLOR_REAL:
        case eDATA_NODE_PRESENTER_COLOR_DICOMSIM:
        case eDATA_NODE_PRESENTER_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_COLOR_3D:
        case eDATA_NODE_PRESENTER_COLOR_BLENDING:
        case eDATA_NODE_PRESENTER_COLOR_USER:
        case eDATA_NODE_PRESENTER_COLOR_HDR:
        case eDATA_NODE_PRESENTER_COLOR_SRGB:
        case eDATA_NODE_PRESENTER_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_SUPER_RED:
        case eDATA_NODE_PRESENTER_COLOR_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_PRESENTER_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_PRESENTER].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_CARDREADER_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_CARDREADER].sCommon;
            break;

        case eDATA_NODE_CARDREADER_HSG_PRESENTATION:
        case eDATA_NODE_CARDREADER_HSG_VIDEO:
        case eDATA_NODE_CARDREADER_HSG_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_ENHANCED:
        case eDATA_NODE_CARDREADER_HSG_REC709:
        case eDATA_NODE_CARDREADER_HSG_REAL:
        case eDATA_NODE_CARDREADER_HSG_DICOMSIM:
        case eDATA_NODE_CARDREADER_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_HSG_3D:
        case eDATA_NODE_CARDREADER_HSG_BLENDING:
        case eDATA_NODE_CARDREADER_HSG_USER:
        case eDATA_NODE_CARDREADER_HSG_HDR:
        case eDATA_NODE_CARDREADER_HSG_SRGB:
        case eDATA_NODE_CARDREADER_HSG_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_SUPER_RED:
        case eDATA_NODE_CARDREADER_HSG_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_CARDREADER_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_CARDREADER].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_CARDREADER_COLOR_PRESENTATION:
        case eDATA_NODE_CARDREADER_COLOR_VIDEO:
        case eDATA_NODE_CARDREADER_COLOR_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_ENHANCED:
        case eDATA_NODE_CARDREADER_COLOR_REC709:
        case eDATA_NODE_CARDREADER_COLOR_REAL:
        case eDATA_NODE_CARDREADER_COLOR_DICOMSIM:
        case eDATA_NODE_CARDREADER_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_COLOR_3D:
        case eDATA_NODE_CARDREADER_COLOR_BLENDING:
        case eDATA_NODE_CARDREADER_COLOR_USER:
        case eDATA_NODE_CARDREADER_COLOR_HDR:
        case eDATA_NODE_CARDREADER_COLOR_SRGB:
        case eDATA_NODE_CARDREADER_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_SUPER_RED:
        case eDATA_NODE_CARDREADER_COLOR_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_CARDREADER_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_CARDREADER].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_MINIUSB_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_MINIUSB].sCommon;
            break;

        case eDATA_NODE_MINIUSB_HSG_PRESENTATION:
        case eDATA_NODE_MINIUSB_HSG_VIDEO:
        case eDATA_NODE_MINIUSB_HSG_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_ENHANCED:
        case eDATA_NODE_MINIUSB_HSG_REC709:
        case eDATA_NODE_MINIUSB_HSG_REAL:
        case eDATA_NODE_MINIUSB_HSG_DICOMSIM:
        case eDATA_NODE_MINIUSB_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_HSG_3D:
        case eDATA_NODE_MINIUSB_HSG_BLENDING:
        case eDATA_NODE_MINIUSB_HSG_USER:
        case eDATA_NODE_MINIUSB_HSG_HDR:
        case eDATA_NODE_MINIUSB_HSG_SRGB:
        case eDATA_NODE_MINIUSB_HSG_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_SUPER_RED:
        case eDATA_NODE_MINIUSB_HSG_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_MINIUSB_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_MINIUSB].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_MINIUSB_COLOR_PRESENTATION:
        case eDATA_NODE_MINIUSB_COLOR_VIDEO:
        case eDATA_NODE_MINIUSB_COLOR_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_ENHANCED:
        case eDATA_NODE_MINIUSB_COLOR_REC709:
        case eDATA_NODE_MINIUSB_COLOR_REAL:
        case eDATA_NODE_MINIUSB_COLOR_DICOMSIM:
        case eDATA_NODE_MINIUSB_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_COLOR_3D:
        case eDATA_NODE_MINIUSB_COLOR_BLENDING:
        case eDATA_NODE_MINIUSB_COLOR_USER:
        case eDATA_NODE_MINIUSB_COLOR_HDR:
        case eDATA_NODE_MINIUSB_COLOR_SRGB:
        case eDATA_NODE_MINIUSB_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_SUPER_RED:
        case eDATA_NODE_MINIUSB_COLOR_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_MINIUSB_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_MINIUSB].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_SLOT1_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_SLOT1].sCommon;
            break;

        case eDATA_NODE_SLOT1_HSG_PRESENTATION:
        case eDATA_NODE_SLOT1_HSG_VIDEO:
        case eDATA_NODE_SLOT1_HSG_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_ENHANCED:
        case eDATA_NODE_SLOT1_HSG_REC709:
        case eDATA_NODE_SLOT1_HSG_REAL:
        case eDATA_NODE_SLOT1_HSG_DICOMSIM:
        case eDATA_NODE_SLOT1_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_HSG_3D:
        case eDATA_NODE_SLOT1_HSG_BLENDING:
        case eDATA_NODE_SLOT1_HSG_USER:
        case eDATA_NODE_SLOT1_HSG_HDR:
        case eDATA_NODE_SLOT1_HSG_SRGB:
        case eDATA_NODE_SLOT1_HSG_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_SUPER_RED:
        case eDATA_NODE_SLOT1_HSG_3D_PASSIVE:
        case eDATA_NODE_SLOT1_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SLOT1_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_SLOT1].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_SLOT1_COLOR_PRESENTATION:
        case eDATA_NODE_SLOT1_COLOR_VIDEO:
        case eDATA_NODE_SLOT1_COLOR_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_ENHANCED:
        case eDATA_NODE_SLOT1_COLOR_REC709:
        case eDATA_NODE_SLOT1_COLOR_REAL:
        case eDATA_NODE_SLOT1_COLOR_DICOMSIM:
        case eDATA_NODE_SLOT1_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_COLOR_3D:
        case eDATA_NODE_SLOT1_COLOR_BLENDING:
        case eDATA_NODE_SLOT1_COLOR_USER:
        case eDATA_NODE_SLOT1_COLOR_HDR:
        case eDATA_NODE_SLOT1_COLOR_SRGB:
        case eDATA_NODE_SLOT1_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_SUPER_RED:
        case eDATA_NODE_SLOT1_COLOR_3D_PASSIVE:
        case eDATA_NODE_SLOT1_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SLOT1_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_SLOT1].sColor[uiNodeTemp];
            }
            break;

        case eDATA_NODE_SLOT2_COMMON:
            psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_SLOT2].sCommon;
            break;

        case eDATA_NODE_SLOT2_HSG_PRESENTATION:
        case eDATA_NODE_SLOT2_HSG_VIDEO:
        case eDATA_NODE_SLOT2_HSG_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_ENHANCED:
        case eDATA_NODE_SLOT2_HSG_REC709:
        case eDATA_NODE_SLOT2_HSG_REAL:
        case eDATA_NODE_SLOT2_HSG_DICOMSIM:
        case eDATA_NODE_SLOT2_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_HSG_3D:
        case eDATA_NODE_SLOT2_HSG_BLENDING:
        case eDATA_NODE_SLOT2_HSG_USER:
        case eDATA_NODE_SLOT2_HSG_HDR:
        case eDATA_NODE_SLOT2_HSG_SRGB:
        case eDATA_NODE_SLOT2_HSG_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_SUPER_RED:
        case eDATA_NODE_SLOT2_HSG_3D_PASSIVE:
        case eDATA_NODE_SLOT2_HSG_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SLOT2_HSG_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_SLOT2].sHSG[uiNodeTemp];
            }
            break;

        case eDATA_NODE_SLOT2_COLOR_PRESENTATION:
        case eDATA_NODE_SLOT2_COLOR_VIDEO:
        case eDATA_NODE_SLOT2_COLOR_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_ENHANCED:
        case eDATA_NODE_SLOT2_COLOR_REC709:
        case eDATA_NODE_SLOT2_COLOR_REAL:
        case eDATA_NODE_SLOT2_COLOR_DICOMSIM:
        case eDATA_NODE_SLOT2_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_COLOR_3D:
        case eDATA_NODE_SLOT2_COLOR_BLENDING:
        case eDATA_NODE_SLOT2_COLOR_USER:
        case eDATA_NODE_SLOT2_COLOR_HDR:
        case eDATA_NODE_SLOT2_COLOR_SRGB:
        case eDATA_NODE_SLOT2_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_SUPER_RED:
        case eDATA_NODE_SLOT2_COLOR_3D_PASSIVE:
        case eDATA_NODE_SLOT2_COLOR_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SLOT2_COLOR_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_SLOT2].sColor[uiNodeTemp];
            }
            break;


        case eDATA_NODE_SOURCE0_HSG_USER_PRESENTATION:
        case eDATA_NODE_SOURCE0_HSG_USER_VIDEO:
        case eDATA_NODE_SOURCE0_HSG_USER_BRIGHT:
        case eDATA_NODE_SOURCE0_HSG_USER_ENHANCED:
        case eDATA_NODE_SOURCE0_HSG_USER_REC709:
        case eDATA_NODE_SOURCE0_HSG_USER_REAL:
        case eDATA_NODE_SOURCE0_HSG_USER_DICOMSIM:
        case eDATA_NODE_SOURCE0_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SOURCE0_HSG_USER_3D:
        case eDATA_NODE_SOURCE0_HSG_USER_BLENDING:
        case eDATA_NODE_SOURCE0_HSG_USER_USER:
        case eDATA_NODE_SOURCE0_HSG_USER_HDR:
        case eDATA_NODE_SOURCE0_HSG_USER_SRGB:
        case eDATA_NODE_SOURCE0_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_SOURCE0_HSG_USER_SUPER_RED:
        case eDATA_NODE_SOURCE0_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_SOURCE0_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SOURCE0_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_RESERVED].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_SOURCE0_COLOR_USER_PRESENTATION:
        case eDATA_NODE_SOURCE0_COLOR_USER_VIDEO:
        case eDATA_NODE_SOURCE0_COLOR_USER_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_USER_ENHANCED:
        case eDATA_NODE_SOURCE0_COLOR_USER_REC709:
        case eDATA_NODE_SOURCE0_COLOR_USER_REAL:
        case eDATA_NODE_SOURCE0_COLOR_USER_DICOMSIM:
        case eDATA_NODE_SOURCE0_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SOURCE0_COLOR_USER_3D:
        case eDATA_NODE_SOURCE0_COLOR_USER_BLENDING:
        case eDATA_NODE_SOURCE0_COLOR_USER_USER:
        case eDATA_NODE_SOURCE0_COLOR_USER_HDR:
        case eDATA_NODE_SOURCE0_COLOR_USER_SRGB:
        case eDATA_NODE_SOURCE0_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_USER_SUPER_RED:
        case eDATA_NODE_SOURCE0_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_SOURCE0_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SOURCE0_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_RESERVED].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_VGA_HSG_USER_PRESENTATION:
        case eDATA_NODE_VGA_HSG_USER_VIDEO:
        case eDATA_NODE_VGA_HSG_USER_BRIGHT:
        case eDATA_NODE_VGA_HSG_USER_ENHANCED:
        case eDATA_NODE_VGA_HSG_USER_REC709:
        case eDATA_NODE_VGA_HSG_USER_REAL:
        case eDATA_NODE_VGA_HSG_USER_DICOMSIM:
        case eDATA_NODE_VGA_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_HSG_USER_3D:
        case eDATA_NODE_VGA_HSG_USER_BLENDING:
        case eDATA_NODE_VGA_HSG_USER_USER:
        case eDATA_NODE_VGA_HSG_USER_HDR:
        case eDATA_NODE_VGA_HSG_USER_SRGB:
        case eDATA_NODE_VGA_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_VGA_HSG_USER_SUPER_RED:
        case eDATA_NODE_VGA_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_VGA_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_VGA_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_VGA].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_VGA_COLOR_USER_PRESENTATION:
        case eDATA_NODE_VGA_COLOR_USER_VIDEO:
        case eDATA_NODE_VGA_COLOR_USER_BRIGHT:
        case eDATA_NODE_VGA_COLOR_USER_ENHANCED:
        case eDATA_NODE_VGA_COLOR_USER_REC709:
        case eDATA_NODE_VGA_COLOR_USER_REAL:
        case eDATA_NODE_VGA_COLOR_USER_DICOMSIM:
        case eDATA_NODE_VGA_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_COLOR_USER_3D:
        case eDATA_NODE_VGA_COLOR_USER_BLENDING:
        case eDATA_NODE_VGA_COLOR_USER_USER:
        case eDATA_NODE_VGA_COLOR_USER_HDR:
        case eDATA_NODE_VGA_COLOR_USER_SRGB:
        case eDATA_NODE_VGA_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_VGA_COLOR_USER_SUPER_RED:
        case eDATA_NODE_VGA_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_VGA_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_VGA_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_VGA].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_BNC_HSG_USER_PRESENTATION:
        case eDATA_NODE_BNC_HSG_USER_VIDEO:
        case eDATA_NODE_BNC_HSG_USER_BRIGHT:
        case eDATA_NODE_BNC_HSG_USER_ENHANCED:
        case eDATA_NODE_BNC_HSG_USER_REC709:
        case eDATA_NODE_BNC_HSG_USER_REAL:
        case eDATA_NODE_BNC_HSG_USER_DICOMSIM:
        case eDATA_NODE_BNC_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_HSG_USER_3D:
        case eDATA_NODE_BNC_HSG_USER_BLENDING:
        case eDATA_NODE_BNC_HSG_USER_USER:
        case eDATA_NODE_BNC_HSG_USER_HDR:
        case eDATA_NODE_BNC_HSG_USER_SRGB:
        case eDATA_NODE_BNC_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_BNC_HSG_USER_SUPER_RED:
        case eDATA_NODE_BNC_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_BNC_HSG_USER_HLG:
            {
                uiNodeTemp = uiNode - eDATA_NODE_BNC_HSG_USER_PRESENTATION;
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_BNC].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_BNC_COLOR_USER_PRESENTATION:
        case eDATA_NODE_BNC_COLOR_USER_VIDEO:
        case eDATA_NODE_BNC_COLOR_USER_BRIGHT:
        case eDATA_NODE_BNC_COLOR_USER_ENHANCED:
        case eDATA_NODE_BNC_COLOR_USER_REC709:
        case eDATA_NODE_BNC_COLOR_USER_REAL:
        case eDATA_NODE_BNC_COLOR_USER_DICOMSIM:
        case eDATA_NODE_BNC_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_COLOR_USER_3D:
        case eDATA_NODE_BNC_COLOR_USER_BLENDING:
        case eDATA_NODE_BNC_COLOR_USER_USER:
        case eDATA_NODE_BNC_COLOR_USER_HDR:
        case eDATA_NODE_BNC_COLOR_USER_SRGB:
        case eDATA_NODE_BNC_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_BNC_COLOR_USER_SUPER_RED:
        case eDATA_NODE_BNC_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_BNC_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_BNC_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_BNC].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDMI1_HSG_USER_PRESENTATION:
        case eDATA_NODE_HDMI1_HSG_USER_VIDEO:
        case eDATA_NODE_HDMI1_HSG_USER_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_USER_ENHANCED:
        case eDATA_NODE_HDMI1_HSG_USER_REC709:
        case eDATA_NODE_HDMI1_HSG_USER_REAL:
        case eDATA_NODE_HDMI1_HSG_USER_DICOMSIM:
        case eDATA_NODE_HDMI1_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_HSG_USER_3D:
        case eDATA_NODE_HDMI1_HSG_USER_BLENDING:
        case eDATA_NODE_HDMI1_HSG_USER_USER:
        case eDATA_NODE_HDMI1_HSG_USER_HDR:
        case eDATA_NODE_HDMI1_HSG_USER_SRGB:
        case eDATA_NODE_HDMI1_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_USER_SUPER_RED:
        case eDATA_NODE_HDMI1_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI1_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDMI1_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDMI1].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDMI1_COLOR_USER_PRESENTATION:
        case eDATA_NODE_HDMI1_COLOR_USER_VIDEO:
        case eDATA_NODE_HDMI1_COLOR_USER_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_USER_ENHANCED:
        case eDATA_NODE_HDMI1_COLOR_USER_REC709:
        case eDATA_NODE_HDMI1_COLOR_USER_REAL:
        case eDATA_NODE_HDMI1_COLOR_USER_DICOMSIM:
        case eDATA_NODE_HDMI1_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_COLOR_USER_3D:
        case eDATA_NODE_HDMI1_COLOR_USER_BLENDING:
        case eDATA_NODE_HDMI1_COLOR_USER_USER:
        case eDATA_NODE_HDMI1_COLOR_USER_HDR:
        case eDATA_NODE_HDMI1_COLOR_USER_SRGB:
        case eDATA_NODE_HDMI1_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_USER_SUPER_RED:
        case eDATA_NODE_HDMI1_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI1_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDMI1_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDMI1].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDMI2_HSG_USER_PRESENTATION:
        case eDATA_NODE_HDMI2_HSG_USER_VIDEO:
        case eDATA_NODE_HDMI2_HSG_USER_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_USER_ENHANCED:
        case eDATA_NODE_HDMI2_HSG_USER_REC709:
        case eDATA_NODE_HDMI2_HSG_USER_REAL:
        case eDATA_NODE_HDMI2_HSG_USER_DICOMSIM:
        case eDATA_NODE_HDMI2_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_HSG_USER_3D:
        case eDATA_NODE_HDMI2_HSG_USER_BLENDING:
        case eDATA_NODE_HDMI2_HSG_USER_USER:
        case eDATA_NODE_HDMI2_HSG_USER_HDR:
        case eDATA_NODE_HDMI2_HSG_USER_SRGB:
        case eDATA_NODE_HDMI2_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_USER_SUPER_RED:
        case eDATA_NODE_HDMI2_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI2_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDMI2_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDMI2].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDMI2_COLOR_USER_PRESENTATION:
        case eDATA_NODE_HDMI2_COLOR_USER_VIDEO:
        case eDATA_NODE_HDMI2_COLOR_USER_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_USER_ENHANCED:
        case eDATA_NODE_HDMI2_COLOR_USER_REC709:
        case eDATA_NODE_HDMI2_COLOR_USER_REAL:
        case eDATA_NODE_HDMI2_COLOR_USER_DICOMSIM:
        case eDATA_NODE_HDMI2_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_COLOR_USER_3D:
        case eDATA_NODE_HDMI2_COLOR_USER_BLENDING:
        case eDATA_NODE_HDMI2_COLOR_USER_USER:
        case eDATA_NODE_HDMI2_COLOR_USER_HDR:
        case eDATA_NODE_HDMI2_COLOR_USER_SRGB:
        case eDATA_NODE_HDMI2_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_USER_SUPER_RED:
        case eDATA_NODE_HDMI2_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI2_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDMI2_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDMI2].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_DVID_HSG_USER_PRESENTATION:
        case eDATA_NODE_DVID_HSG_USER_VIDEO:
        case eDATA_NODE_DVID_HSG_USER_BRIGHT:
        case eDATA_NODE_DVID_HSG_USER_ENHANCED:
        case eDATA_NODE_DVID_HSG_USER_REC709:
        case eDATA_NODE_DVID_HSG_USER_REAL:
        case eDATA_NODE_DVID_HSG_USER_DICOMSIM:
        case eDATA_NODE_DVID_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_HSG_USER_3D:
        case eDATA_NODE_DVID_HSG_USER_BLENDING:
        case eDATA_NODE_DVID_HSG_USER_USER:
        case eDATA_NODE_DVID_HSG_USER_HDR:
        case eDATA_NODE_DVID_HSG_USER_SRGB:
        case eDATA_NODE_DVID_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_DVID_HSG_USER_SUPER_RED:
        case eDATA_NODE_DVID_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_DVID_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_DVID_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_DVI].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_DVID_COLOR_USER_PRESENTATION:
        case eDATA_NODE_DVID_COLOR_USER_VIDEO:
        case eDATA_NODE_DVID_COLOR_USER_BRIGHT:
        case eDATA_NODE_DVID_COLOR_USER_ENHANCED:
        case eDATA_NODE_DVID_COLOR_USER_REC709:
        case eDATA_NODE_DVID_COLOR_USER_REAL:
        case eDATA_NODE_DVID_COLOR_USER_DICOMSIM:
        case eDATA_NODE_DVID_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_COLOR_USER_3D:
        case eDATA_NODE_DVID_COLOR_USER_BLENDING:
        case eDATA_NODE_DVID_COLOR_USER_USER:
        case eDATA_NODE_DVID_COLOR_USER_HDR:
        case eDATA_NODE_DVID_COLOR_USER_SRGB:
        case eDATA_NODE_DVID_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_DVID_COLOR_USER_SUPER_RED:
        case eDATA_NODE_DVID_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_DVID_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_DVID_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_DVI].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_DP_HSG_USER_PRESENTATION:
        case eDATA_NODE_DP_HSG_USER_VIDEO:
        case eDATA_NODE_DP_HSG_USER_BRIGHT:
        case eDATA_NODE_DP_HSG_USER_ENHANCED:
        case eDATA_NODE_DP_HSG_USER_REC709:
        case eDATA_NODE_DP_HSG_USER_REAL:
        case eDATA_NODE_DP_HSG_USER_DICOMSIM:
        case eDATA_NODE_DP_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DP_HSG_USER_3D:
        case eDATA_NODE_DP_HSG_USER_BLENDING:
        case eDATA_NODE_DP_HSG_USER_USER:
        case eDATA_NODE_DP_HSG_USER_HDR:
        case eDATA_NODE_DP_HSG_USER_SRGB:
        case eDATA_NODE_DP_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_DP_HSG_USER_SUPER_RED:
        case eDATA_NODE_DP_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_DP_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_DP_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_DISPLAYPORT].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_DP_COLOR_USER_PRESENTATION:
        case eDATA_NODE_DP_COLOR_USER_VIDEO:
        case eDATA_NODE_DP_COLOR_USER_BRIGHT:
        case eDATA_NODE_DP_COLOR_USER_ENHANCED:
        case eDATA_NODE_DP_COLOR_USER_REC709:
        case eDATA_NODE_DP_COLOR_USER_REAL:
        case eDATA_NODE_DP_COLOR_USER_DICOMSIM:
        case eDATA_NODE_DP_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DP_COLOR_USER_3D:
        case eDATA_NODE_DP_COLOR_USER_BLENDING:
        case eDATA_NODE_DP_COLOR_USER_USER:
        case eDATA_NODE_DP_COLOR_USER_HDR:
        case eDATA_NODE_DP_COLOR_USER_SRGB:
        case eDATA_NODE_DP_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_DP_COLOR_USER_SUPER_RED:
        case eDATA_NODE_DP_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_DP_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_DP_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_DISPLAYPORT].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_3GSDI_HSG_USER_PRESENTATION:
        case eDATA_NODE_3GSDI_HSG_USER_VIDEO:
        case eDATA_NODE_3GSDI_HSG_USER_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_USER_ENHANCED:
        case eDATA_NODE_3GSDI_HSG_USER_REC709:
        case eDATA_NODE_3GSDI_HSG_USER_REAL:
        case eDATA_NODE_3GSDI_HSG_USER_DICOMSIM:
        case eDATA_NODE_3GSDI_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_HSG_USER_3D:
        case eDATA_NODE_3GSDI_HSG_USER_BLENDING:
        case eDATA_NODE_3GSDI_HSG_USER_USER:
        case eDATA_NODE_3GSDI_HSG_USER_HDR:
        case eDATA_NODE_3GSDI_HSG_USER_SRGB:
        case eDATA_NODE_3GSDI_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_USER_SUPER_RED:
        case eDATA_NODE_3GSDI_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_3GSDI_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_3GSDI_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_3GSDI].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_3GSDI_COLOR_USER_PRESENTATION:
        case eDATA_NODE_3GSDI_COLOR_USER_VIDEO:
        case eDATA_NODE_3GSDI_COLOR_USER_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_USER_ENHANCED:
        case eDATA_NODE_3GSDI_COLOR_USER_REC709:
        case eDATA_NODE_3GSDI_COLOR_USER_REAL:
        case eDATA_NODE_3GSDI_COLOR_USER_DICOMSIM:
        case eDATA_NODE_3GSDI_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_COLOR_USER_3D:
        case eDATA_NODE_3GSDI_COLOR_USER_BLENDING:
        case eDATA_NODE_3GSDI_COLOR_USER_USER:
        case eDATA_NODE_3GSDI_COLOR_USER_HDR:
        case eDATA_NODE_3GSDI_COLOR_USER_SRGB:
        case eDATA_NODE_3GSDI_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_USER_SUPER_RED:
        case eDATA_NODE_3GSDI_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_3GSDI_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_3GSDI_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_3GSDI].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDBASET_HSG_USER_PRESENTATION:
        case eDATA_NODE_HDBASET_HSG_USER_VIDEO:
        case eDATA_NODE_HDBASET_HSG_USER_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_USER_ENHANCED:
        case eDATA_NODE_HDBASET_HSG_USER_REC709:
        case eDATA_NODE_HDBASET_HSG_USER_REAL:
        case eDATA_NODE_HDBASET_HSG_USER_DICOMSIM:
        case eDATA_NODE_HDBASET_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_HSG_USER_3D:
        case eDATA_NODE_HDBASET_HSG_USER_BLENDING:
        case eDATA_NODE_HDBASET_HSG_USER_USER:
        case eDATA_NODE_HDBASET_HSG_USER_HDR:
        case eDATA_NODE_HDBASET_HSG_USER_SRGB:
        case eDATA_NODE_HDBASET_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_USER_SUPER_RED:
        case eDATA_NODE_HDBASET_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_HDBASET_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDBASET_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDBASET].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_HDBASET_COLOR_USER_PRESENTATION:
        case eDATA_NODE_HDBASET_COLOR_USER_VIDEO:
        case eDATA_NODE_HDBASET_COLOR_USER_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_USER_ENHANCED:
        case eDATA_NODE_HDBASET_COLOR_USER_REC709:
        case eDATA_NODE_HDBASET_COLOR_USER_REAL:
        case eDATA_NODE_HDBASET_COLOR_USER_DICOMSIM:
        case eDATA_NODE_HDBASET_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_COLOR_USER_3D:
        case eDATA_NODE_HDBASET_COLOR_USER_BLENDING:
        case eDATA_NODE_HDBASET_COLOR_USER_USER:
        case eDATA_NODE_HDBASET_COLOR_USER_HDR:
        case eDATA_NODE_HDBASET_COLOR_USER_SRGB:
        case eDATA_NODE_HDBASET_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_USER_SUPER_RED:
        case eDATA_NODE_HDBASET_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_HDBASET_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_HDBASET_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_HDBASET].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_12GSDI_HSG_USER_PRESENTATION:
        case eDATA_NODE_12GSDI_HSG_USER_VIDEO:
        case eDATA_NODE_12GSDI_HSG_USER_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_USER_ENHANCED:
        case eDATA_NODE_12GSDI_HSG_USER_REC709:
        case eDATA_NODE_12GSDI_HSG_USER_REAL:
        case eDATA_NODE_12GSDI_HSG_USER_DICOMSIM:
        case eDATA_NODE_12GSDI_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_HSG_USER_3D:
        case eDATA_NODE_12GSDI_HSG_USER_BLENDING:
        case eDATA_NODE_12GSDI_HSG_USER_USER:
        case eDATA_NODE_12GSDI_HSG_USER_HDR:
        case eDATA_NODE_12GSDI_HSG_USER_SRGB:
        case eDATA_NODE_12GSDI_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_USER_SUPER_RED:
        case eDATA_NODE_12GSDI_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_12GSDI_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_12GSDI_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_12GSDI].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_12GSDI_COLOR_USER_PRESENTATION:
        case eDATA_NODE_12GSDI_COLOR_USER_VIDEO:
        case eDATA_NODE_12GSDI_COLOR_USER_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_USER_ENHANCED:
        case eDATA_NODE_12GSDI_COLOR_USER_REC709:
        case eDATA_NODE_12GSDI_COLOR_USER_REAL:
        case eDATA_NODE_12GSDI_COLOR_USER_DICOMSIM:
        case eDATA_NODE_12GSDI_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_COLOR_USER_3D:
        case eDATA_NODE_12GSDI_COLOR_USER_BLENDING:
        case eDATA_NODE_12GSDI_COLOR_USER_USER:
        case eDATA_NODE_12GSDI_COLOR_USER_HDR:
        case eDATA_NODE_12GSDI_COLOR_USER_SRGB:
        case eDATA_NODE_12GSDI_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_USER_SUPER_RED:
        case eDATA_NODE_12GSDI_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_12GSDI_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_12GSDI_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_12GSDI].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_PRESENTER_HSG_USER_PRESENTATION:
        case eDATA_NODE_PRESENTER_HSG_USER_VIDEO:
        case eDATA_NODE_PRESENTER_HSG_USER_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_USER_ENHANCED:
        case eDATA_NODE_PRESENTER_HSG_USER_REC709:
        case eDATA_NODE_PRESENTER_HSG_USER_REAL:
        case eDATA_NODE_PRESENTER_HSG_USER_DICOMSIM:
        case eDATA_NODE_PRESENTER_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_HSG_USER_3D:
        case eDATA_NODE_PRESENTER_HSG_USER_BLENDING:
        case eDATA_NODE_PRESENTER_HSG_USER_USER:
        case eDATA_NODE_PRESENTER_HSG_USER_HDR:
        case eDATA_NODE_PRESENTER_HSG_USER_SRGB:
        case eDATA_NODE_PRESENTER_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_USER_SUPER_RED:
        case eDATA_NODE_PRESENTER_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_PRESENTER_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_PRESENTER].sHSG_User[uiNodeTemp];
            }
            break;
        case eDATA_NODE_PRESENTER_COLOR_USER_PRESENTATION:
        case eDATA_NODE_PRESENTER_COLOR_USER_VIDEO:
        case eDATA_NODE_PRESENTER_COLOR_USER_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_USER_ENHANCED:
        case eDATA_NODE_PRESENTER_COLOR_USER_REC709:
        case eDATA_NODE_PRESENTER_COLOR_USER_REAL:
        case eDATA_NODE_PRESENTER_COLOR_USER_DICOMSIM:
        case eDATA_NODE_PRESENTER_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_COLOR_USER_3D:
        case eDATA_NODE_PRESENTER_COLOR_USER_BLENDING:
        case eDATA_NODE_PRESENTER_COLOR_USER_USER:
        case eDATA_NODE_PRESENTER_COLOR_USER_HDR:
        case eDATA_NODE_PRESENTER_COLOR_USER_SRGB:
        case eDATA_NODE_PRESENTER_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_USER_SUPER_RED:
        case eDATA_NODE_PRESENTER_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_PRESENTER_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_PRESENTER].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_CARDREADER_HSG_USER_PRESENTATION:
        case eDATA_NODE_CARDREADER_HSG_USER_VIDEO:
        case eDATA_NODE_CARDREADER_HSG_USER_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_USER_ENHANCED:
        case eDATA_NODE_CARDREADER_HSG_USER_REC709:
        case eDATA_NODE_CARDREADER_HSG_USER_REAL:
        case eDATA_NODE_CARDREADER_HSG_USER_DICOMSIM:
        case eDATA_NODE_CARDREADER_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_HSG_USER_3D:
        case eDATA_NODE_CARDREADER_HSG_USER_BLENDING:
        case eDATA_NODE_CARDREADER_HSG_USER_USER:
        case eDATA_NODE_CARDREADER_HSG_USER_HDR:
        case eDATA_NODE_CARDREADER_HSG_USER_SRGB:
        case eDATA_NODE_CARDREADER_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_USER_SUPER_RED:
        case eDATA_NODE_CARDREADER_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_CARDREADER_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_CARDREADER].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_CARDREADER_COLOR_USER_PRESENTATION:
        case eDATA_NODE_CARDREADER_COLOR_USER_VIDEO:
        case eDATA_NODE_CARDREADER_COLOR_USER_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_USER_ENHANCED:
        case eDATA_NODE_CARDREADER_COLOR_USER_REC709:
        case eDATA_NODE_CARDREADER_COLOR_USER_REAL:
        case eDATA_NODE_CARDREADER_COLOR_USER_DICOMSIM:
        case eDATA_NODE_CARDREADER_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_COLOR_USER_3D:
        case eDATA_NODE_CARDREADER_COLOR_USER_BLENDING:
        case eDATA_NODE_CARDREADER_COLOR_USER_USER:
        case eDATA_NODE_CARDREADER_COLOR_USER_HDR:
        case eDATA_NODE_CARDREADER_COLOR_USER_SRGB:
        case eDATA_NODE_CARDREADER_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_USER_SUPER_RED:
        case eDATA_NODE_CARDREADER_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_CARDREADER_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_CARDREADER].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_MINIUSB_HSG_USER_PRESENTATION:
        case eDATA_NODE_MINIUSB_HSG_USER_VIDEO:
        case eDATA_NODE_MINIUSB_HSG_USER_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_USER_ENHANCED:
        case eDATA_NODE_MINIUSB_HSG_USER_REC709:
        case eDATA_NODE_MINIUSB_HSG_USER_REAL:
        case eDATA_NODE_MINIUSB_HSG_USER_DICOMSIM:
        case eDATA_NODE_MINIUSB_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_HSG_USER_3D:
        case eDATA_NODE_MINIUSB_HSG_USER_BLENDING:
        case eDATA_NODE_MINIUSB_HSG_USER_USER:
        case eDATA_NODE_MINIUSB_HSG_USER_HDR:
        case eDATA_NODE_MINIUSB_HSG_USER_SRGB:
        case eDATA_NODE_MINIUSB_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_USER_SUPER_RED:
        case eDATA_NODE_MINIUSB_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_MINIUSB_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_MINIUSB].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_MINIUSB_COLOR_USER_PRESENTATION:
        case eDATA_NODE_MINIUSB_COLOR_USER_VIDEO:
        case eDATA_NODE_MINIUSB_COLOR_USER_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_USER_ENHANCED:
        case eDATA_NODE_MINIUSB_COLOR_USER_REC709:
        case eDATA_NODE_MINIUSB_COLOR_USER_REAL:
        case eDATA_NODE_MINIUSB_COLOR_USER_DICOMSIM:
        case eDATA_NODE_MINIUSB_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_COLOR_USER_3D:
        case eDATA_NODE_MINIUSB_COLOR_USER_BLENDING:
        case eDATA_NODE_MINIUSB_COLOR_USER_USER:
        case eDATA_NODE_MINIUSB_COLOR_USER_HDR:
        case eDATA_NODE_MINIUSB_COLOR_USER_SRGB:
        case eDATA_NODE_MINIUSB_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_USER_SUPER_RED:
        case eDATA_NODE_MINIUSB_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_MINIUSB_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_MINIUSB].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_SLOT1_HSG_USER_PRESENTATION:
        case eDATA_NODE_SLOT1_HSG_USER_VIDEO:
        case eDATA_NODE_SLOT1_HSG_USER_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_USER_ENHANCED:
        case eDATA_NODE_SLOT1_HSG_USER_REC709:
        case eDATA_NODE_SLOT1_HSG_USER_REAL:
        case eDATA_NODE_SLOT1_HSG_USER_DICOMSIM:
        case eDATA_NODE_SLOT1_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_HSG_USER_3D:
        case eDATA_NODE_SLOT1_HSG_USER_BLENDING:
        case eDATA_NODE_SLOT1_HSG_USER_USER:
        case eDATA_NODE_SLOT1_HSG_USER_HDR:
        case eDATA_NODE_SLOT1_HSG_USER_SRGB:
        case eDATA_NODE_SLOT1_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_USER_SUPER_RED:
        case eDATA_NODE_SLOT1_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT1_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SLOT1_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_SLOT1].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_SLOT1_COLOR_USER_PRESENTATION:
        case eDATA_NODE_SLOT1_COLOR_USER_VIDEO:
        case eDATA_NODE_SLOT1_COLOR_USER_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_USER_ENHANCED:
        case eDATA_NODE_SLOT1_COLOR_USER_REC709:
        case eDATA_NODE_SLOT1_COLOR_USER_REAL:
        case eDATA_NODE_SLOT1_COLOR_USER_DICOMSIM:
        case eDATA_NODE_SLOT1_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_COLOR_USER_3D:
        case eDATA_NODE_SLOT1_COLOR_USER_BLENDING:
        case eDATA_NODE_SLOT1_COLOR_USER_USER:
        case eDATA_NODE_SLOT1_COLOR_USER_HDR:
        case eDATA_NODE_SLOT1_COLOR_USER_SRGB:
        case eDATA_NODE_SLOT1_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_USER_SUPER_RED:
        case eDATA_NODE_SLOT1_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT1_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SLOT1_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_SLOT1].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_SLOT2_HSG_USER_PRESENTATION:
        case eDATA_NODE_SLOT2_HSG_USER_VIDEO:
        case eDATA_NODE_SLOT2_HSG_USER_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_USER_ENHANCED:
        case eDATA_NODE_SLOT2_HSG_USER_REC709:
        case eDATA_NODE_SLOT2_HSG_USER_REAL:
        case eDATA_NODE_SLOT2_HSG_USER_DICOMSIM:
        case eDATA_NODE_SLOT2_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_HSG_USER_3D:
        case eDATA_NODE_SLOT2_HSG_USER_BLENDING:
        case eDATA_NODE_SLOT2_HSG_USER_USER:
        case eDATA_NODE_SLOT2_HSG_USER_HDR:
        case eDATA_NODE_SLOT2_HSG_USER_SRGB:
        case eDATA_NODE_SLOT2_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_USER_SUPER_RED:
        case eDATA_NODE_SLOT2_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT2_HSG_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SLOT2_HSG_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_SLOT2].sHSG_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_SLOT2_COLOR_USER_PRESENTATION:
        case eDATA_NODE_SLOT2_COLOR_USER_VIDEO:
        case eDATA_NODE_SLOT2_COLOR_USER_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_USER_ENHANCED:
        case eDATA_NODE_SLOT2_COLOR_USER_REC709:
        case eDATA_NODE_SLOT2_COLOR_USER_REAL:
        case eDATA_NODE_SLOT2_COLOR_USER_DICOMSIM:
        case eDATA_NODE_SLOT2_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_COLOR_USER_3D:
        case eDATA_NODE_SLOT2_COLOR_USER_BLENDING:
        case eDATA_NODE_SLOT2_COLOR_USER_USER:
        case eDATA_NODE_SLOT2_COLOR_USER_HDR:
        case eDATA_NODE_SLOT2_COLOR_USER_SRGB:
        case eDATA_NODE_SLOT2_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_USER_SUPER_RED:
        case eDATA_NODE_SLOT2_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT2_COLOR_USER_HLG:
            {
                uiNodeTemp = (UINT16)(uiNode - eDATA_NODE_SLOT2_COLOR_USER_PRESENTATION);
                psPara = m_sSourceTable.p_sSourceData[eCM_SOURCE_SLOT2].sColor_User[uiNodeTemp];
            }
            break;

        case eDATA_NODE_WAP_PRESENTATION_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_PRESENTATION].sPWM_100;
            break;
        case eDATA_NODE_WAP_VIDEO_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_VIDEO].sPWM_100;
            break;
        case eDATA_NODE_WAP_BRIGHT_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_BRIGHT].sPWM_100;
            break;
        case eDATA_NODE_WAP_ENHANCED_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_ENHANCED].sPWM_100;
            break;
        case eDATA_NODE_WAP_REC709_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_REC709].sPWM_100;
            break;
        case eDATA_NODE_WAP_REAL_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_REAL].sPWM_100;
            break;
        case eDATA_NODE_WAP_DICOM_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_DICOMSIM].sPWM_100;
            break;
        case eDATA_NODE_WAP_2D_HIGH_SPEED_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_2DHIGHSPEED].sPWM_100;
            break;
        case eDATA_NODE_WAP_3D_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_3D].sPWM_100;
            break;
        case eDATA_NODE_WAP_BLENDING_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_BLENDING].sPWM_100;
            break;
        case eDATA_NODE_WAP_USER_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_USER].sPWM_100;
            break;
        case eDATA_NODE_WAP_HDR_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_HDR].sPWM_100;
            break;
        case eDATA_NODE_WAP_SRGB_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SRGB].sPWM_100;
            break;
        case eDATA_NODE_WAP_SUPER_BRIGHT_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SUPER_BRIGHT].sPWM_100;
            break;
        case eDATA_NODE_WAP_SUPER_RED_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SUPER_RED].sPWM_100;
            break;
        case eDATA_NODE_WAP_3D_PASSIVE_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_3D_PASSIVE].sPWM_100;
            break;
        case eDATA_NODE_WAP_HLG_PWM_100:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_HLG].sPWM_100;
            break;

        case eDATA_NODE_WAP_PRESENTATION_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_PRESENTATION].sPWM_50;
            break;
        case eDATA_NODE_WAP_VIDEO_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_VIDEO].sPWM_50;
            break;
        case eDATA_NODE_WAP_BRIGHT_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_BRIGHT].sPWM_50;
            break;
        case eDATA_NODE_WAP_ENHANCED_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_ENHANCED].sPWM_50;
            break;
        case eDATA_NODE_WAP_REC709_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_REC709].sPWM_50;
            break;
        case eDATA_NODE_WAP_REAL_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_REAL].sPWM_50;
            break;
        case eDATA_NODE_WAP_DICOM_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_DICOMSIM].sPWM_50;
            break;
        case eDATA_NODE_WAP_2D_HIGH_SPEED_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_2DHIGHSPEED].sPWM_50;
            break;
        case eDATA_NODE_WAP_3D_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_3D].sPWM_50;
            break;
        case eDATA_NODE_WAP_BLENDING_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_BLENDING].sPWM_50;
            break;
        case eDATA_NODE_WAP_USER_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_USER].sPWM_50;
            break;
        case eDATA_NODE_WAP_HDR_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_HDR].sPWM_50;
            break;
        case eDATA_NODE_WAP_SRGB_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SRGB].sPWM_50;
            break;
        case eDATA_NODE_WAP_SUPER_BRIGHT_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SUPER_BRIGHT].sPWM_50;
            break;
        case eDATA_NODE_WAP_SUPER_RED_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SUPER_RED].sPWM_50;
            break;
        case eDATA_NODE_WAP_3D_PASSIVE_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_3D_PASSIVE].sPWM_50;
            break;
        case eDATA_NODE_WAP_HLG_PWM_50:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_HLG].sPWM_50;
            break;

        case eDATA_NODE_WAP_PRESENTATION_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_PRESENTATION].sPWM_30;
            break;
        case eDATA_NODE_WAP_VIDEO_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_VIDEO].sPWM_30;
            break;
        case eDATA_NODE_WAP_BRIGHT_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_BRIGHT].sPWM_30;
            break;
        case eDATA_NODE_WAP_ENHANCED_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_ENHANCED].sPWM_30;
            break;
        case eDATA_NODE_WAP_REC709_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_REC709].sPWM_30;
            break;
        case eDATA_NODE_WAP_REAL_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_REAL].sPWM_30;
            break;
        case eDATA_NODE_WAP_DICOM_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_DICOMSIM].sPWM_30;
            break;
        case eDATA_NODE_WAP_2D_HIGH_SPEED_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_2DHIGHSPEED].sPWM_30;
            break;
        case eDATA_NODE_WAP_3D_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_3D].sPWM_30;
            break;
        case eDATA_NODE_WAP_BLENDING_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_BLENDING].sPWM_30;
            break;
        case eDATA_NODE_WAP_USER_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_USER].sPWM_30;
            break;
        case eDATA_NODE_WAP_HDR_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_HDR].sPWM_30;
            break;
        case eDATA_NODE_WAP_SRGB_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SRGB].sPWM_30;
            break;
        case eDATA_NODE_WAP_SUPER_BRIGHT_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SUPER_BRIGHT].sPWM_30;
            break;
        case eDATA_NODE_WAP_SUPER_RED_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SUPER_RED].sPWM_30;
            break;
        case eDATA_NODE_WAP_3D_PASSIVE_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_3D_PASSIVE].sPWM_30;
            break;
        case eDATA_NODE_WAP_HLG_PWM_30:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_HLG].sPWM_30;
            break;

        case eDATA_NODE_WAP_PRESENTATION_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_PRESENTATION].sPWM_10;
            break;
        case eDATA_NODE_WAP_VIDEO_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_VIDEO].sPWM_10;
            break;
        case eDATA_NODE_WAP_BRIGHT_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_BRIGHT].sPWM_10;
            break;
        case eDATA_NODE_WAP_ENHANCED_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_ENHANCED].sPWM_10;
            break;
        case eDATA_NODE_WAP_REC709_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_REC709].sPWM_10;
            break;
        case eDATA_NODE_WAP_REAL_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_REAL].sPWM_10;
            break;
        case eDATA_NODE_WAP_DICOM_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_DICOMSIM].sPWM_10;
            break;
        case eDATA_NODE_WAP_2D_HIGH_SPEED_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_2DHIGHSPEED].sPWM_10;
            break;
        case eDATA_NODE_WAP_3D_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_3D].sPWM_10;
            break;
        case eDATA_NODE_WAP_BLENDING_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_BLENDING].sPWM_10;
            break;
        case eDATA_NODE_WAP_USER_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_USER].sPWM_10;
            break;
        case eDATA_NODE_WAP_HDR_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_HDR].sPWM_10;
            break;
        case eDATA_NODE_WAP_SRGB_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SRGB].sPWM_10;
            break;
        case eDATA_NODE_WAP_SUPER_BRIGHT_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SUPER_BRIGHT].sPWM_10;
            break;
        case eDATA_NODE_WAP_SUPER_RED_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_SUPER_RED].sPWM_10;
            break;
        case eDATA_NODE_WAP_3D_PASSIVE_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_3D_PASSIVE].sPWM_10;
            break;
        case eDATA_NODE_WAP_HLG_PWM_10:
            psPara = m_sSourceTable.p_sWAPData[eCM_PICTURE_SETTINGS_HLG].sPWM_10;
            break;

        default:
            psPara = NULL;
            break;
    }

    return psPara;
}

static BOOL utilDataMgr_SourceSupport(UINT16 uiNode)
{
    UINT8   ucSource = eCM_SOURCE_NUMBER;
    UINT8   ucGuiSrcID = eGUI_SOURCE_ID_NUMBER;

    switch(uiNode)
    {
        case eDATA_NODE_SOURCE0_COMMON:
        case eDATA_NODE_SOURCE0_HSG_PRESENTATION:
        case eDATA_NODE_SOURCE0_HSG_VIDEO:
        case eDATA_NODE_SOURCE0_HSG_BRIGHT:
        case eDATA_NODE_SOURCE0_HSG_ENHANCED:
        case eDATA_NODE_SOURCE0_HSG_REC709:
        case eDATA_NODE_SOURCE0_HSG_REAL:
        case eDATA_NODE_SOURCE0_HSG_DICOMSIM:
        case eDATA_NODE_SOURCE0_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_SOURCE0_HSG_3D:
        case eDATA_NODE_SOURCE0_HSG_BLENDING:
        case eDATA_NODE_SOURCE0_HSG_USER:
        case eDATA_NODE_SOURCE0_HSG_HDR:
        case eDATA_NODE_SOURCE0_HSG_SRGB:
        case eDATA_NODE_SOURCE0_HSG_SUPER_BRIGHT:
        case eDATA_NODE_SOURCE0_HSG_SUPER_RED:
        case eDATA_NODE_SOURCE0_HSG_3D_PASSIVE:
        case eDATA_NODE_SOURCE0_HSG_HLG:

        case eDATA_NODE_SOURCE0_HSG_USER_PRESENTATION:
        case eDATA_NODE_SOURCE0_HSG_USER_VIDEO:
        case eDATA_NODE_SOURCE0_HSG_USER_BRIGHT:
        case eDATA_NODE_SOURCE0_HSG_USER_ENHANCED:
        case eDATA_NODE_SOURCE0_HSG_USER_REC709:
        case eDATA_NODE_SOURCE0_HSG_USER_REAL:
        case eDATA_NODE_SOURCE0_HSG_USER_DICOMSIM:
        case eDATA_NODE_SOURCE0_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SOURCE0_HSG_USER_3D:
        case eDATA_NODE_SOURCE0_HSG_USER_BLENDING:
        case eDATA_NODE_SOURCE0_HSG_USER_USER:
        case eDATA_NODE_SOURCE0_HSG_USER_HDR:
        case eDATA_NODE_SOURCE0_HSG_USER_SRGB:
        case eDATA_NODE_SOURCE0_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_SOURCE0_HSG_USER_SUPER_RED:
        case eDATA_NODE_SOURCE0_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_SOURCE0_HSG_USER_HLG:

        case eDATA_NODE_SOURCE0_COLOR_PRESENTATION:
        case eDATA_NODE_SOURCE0_COLOR_VIDEO:
        case eDATA_NODE_SOURCE0_COLOR_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_ENHANCED:
        case eDATA_NODE_SOURCE0_COLOR_REC709:
        case eDATA_NODE_SOURCE0_COLOR_REAL:
        case eDATA_NODE_SOURCE0_COLOR_DICOMSIM:
        case eDATA_NODE_SOURCE0_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_SOURCE0_COLOR_3D:
        case eDATA_NODE_SOURCE0_COLOR_BLENDING:
        case eDATA_NODE_SOURCE0_COLOR_USER:
        case eDATA_NODE_SOURCE0_COLOR_HDR:
        case eDATA_NODE_SOURCE0_COLOR_SRGB:
        case eDATA_NODE_SOURCE0_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_SUPER_RED:
        case eDATA_NODE_SOURCE0_COLOR_3D_PASSIVE:
        case eDATA_NODE_SOURCE0_COLOR_HLG:

        case eDATA_NODE_SOURCE0_COLOR_USER_PRESENTATION:
        case eDATA_NODE_SOURCE0_COLOR_USER_VIDEO:
        case eDATA_NODE_SOURCE0_COLOR_USER_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_USER_ENHANCED:
        case eDATA_NODE_SOURCE0_COLOR_USER_REC709:
        case eDATA_NODE_SOURCE0_COLOR_USER_REAL:
        case eDATA_NODE_SOURCE0_COLOR_USER_DICOMSIM:
        case eDATA_NODE_SOURCE0_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SOURCE0_COLOR_USER_3D:
        case eDATA_NODE_SOURCE0_COLOR_USER_BLENDING:
        case eDATA_NODE_SOURCE0_COLOR_USER_USER:
        case eDATA_NODE_SOURCE0_COLOR_USER_HDR:
        case eDATA_NODE_SOURCE0_COLOR_USER_SRGB:
        case eDATA_NODE_SOURCE0_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_USER_SUPER_RED:
        case eDATA_NODE_SOURCE0_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_SOURCE0_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_RESERVED;
            break;

        case eDATA_NODE_VGA_COMMON:
        case eDATA_NODE_VGA_HSG_PRESENTATION:
        case eDATA_NODE_VGA_HSG_VIDEO:
        case eDATA_NODE_VGA_HSG_BRIGHT:
        case eDATA_NODE_VGA_HSG_ENHANCED:
        case eDATA_NODE_VGA_HSG_REC709:
        case eDATA_NODE_VGA_HSG_REAL:
        case eDATA_NODE_VGA_HSG_DICOMSIM:
        case eDATA_NODE_VGA_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_HSG_3D:
        case eDATA_NODE_VGA_HSG_BLENDING:
        case eDATA_NODE_VGA_HSG_USER:
        case eDATA_NODE_VGA_HSG_HDR:
        case eDATA_NODE_VGA_HSG_SRGB:
        case eDATA_NODE_VGA_HSG_SUPER_BRIGHT:
        case eDATA_NODE_VGA_HSG_SUPER_RED:
        case eDATA_NODE_VGA_HSG_3D_PASSIVE:
        case eDATA_NODE_VGA_HSG_HLG:

        case eDATA_NODE_VGA_HSG_USER_PRESENTATION:
        case eDATA_NODE_VGA_HSG_USER_VIDEO:
        case eDATA_NODE_VGA_HSG_USER_BRIGHT:
        case eDATA_NODE_VGA_HSG_USER_ENHANCED:
        case eDATA_NODE_VGA_HSG_USER_REC709:
        case eDATA_NODE_VGA_HSG_USER_REAL:
        case eDATA_NODE_VGA_HSG_USER_DICOMSIM:
        case eDATA_NODE_VGA_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_HSG_USER_3D:
        case eDATA_NODE_VGA_HSG_USER_BLENDING:
        case eDATA_NODE_VGA_HSG_USER_USER:
        case eDATA_NODE_VGA_HSG_USER_HDR:
        case eDATA_NODE_VGA_HSG_USER_SRGB:
        case eDATA_NODE_VGA_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_VGA_HSG_USER_SUPER_RED:
        case eDATA_NODE_VGA_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_VGA_HSG_USER_HLG:

        case eDATA_NODE_VGA_COLOR_PRESENTATION:
        case eDATA_NODE_VGA_COLOR_VIDEO:
        case eDATA_NODE_VGA_COLOR_BRIGHT:
        case eDATA_NODE_VGA_COLOR_ENHANCED:
        case eDATA_NODE_VGA_COLOR_REC709:
        case eDATA_NODE_VGA_COLOR_REAL:
        case eDATA_NODE_VGA_COLOR_DICOMSIM:
        case eDATA_NODE_VGA_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_COLOR_3D:
        case eDATA_NODE_VGA_COLOR_BLENDING:
        case eDATA_NODE_VGA_COLOR_USER:
        case eDATA_NODE_VGA_COLOR_HDR:
        case eDATA_NODE_VGA_COLOR_SRGB:
        case eDATA_NODE_VGA_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_VGA_COLOR_SUPER_RED:
        case eDATA_NODE_VGA_COLOR_3D_PASSIVE:
        case eDATA_NODE_VGA_COLOR_HLG:

        case eDATA_NODE_VGA_COLOR_USER_PRESENTATION:
        case eDATA_NODE_VGA_COLOR_USER_VIDEO:
        case eDATA_NODE_VGA_COLOR_USER_BRIGHT:
        case eDATA_NODE_VGA_COLOR_USER_ENHANCED:
        case eDATA_NODE_VGA_COLOR_USER_REC709:
        case eDATA_NODE_VGA_COLOR_USER_REAL:
        case eDATA_NODE_VGA_COLOR_USER_DICOMSIM:
        case eDATA_NODE_VGA_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_COLOR_USER_3D:
        case eDATA_NODE_VGA_COLOR_USER_BLENDING:
        case eDATA_NODE_VGA_COLOR_USER_USER:
        case eDATA_NODE_VGA_COLOR_USER_HDR:
        case eDATA_NODE_VGA_COLOR_USER_SRGB:
        case eDATA_NODE_VGA_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_VGA_COLOR_USER_SUPER_RED:
        case eDATA_NODE_VGA_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_VGA_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_VGA;
            break;

        case eDATA_NODE_BNC_COMMON:
        case eDATA_NODE_BNC_HSG_PRESENTATION:
        case eDATA_NODE_BNC_HSG_VIDEO:
        case eDATA_NODE_BNC_HSG_BRIGHT:
        case eDATA_NODE_BNC_HSG_ENHANCED:
        case eDATA_NODE_BNC_HSG_REC709:
        case eDATA_NODE_BNC_HSG_REAL:
        case eDATA_NODE_BNC_HSG_DICOMSIM:
        case eDATA_NODE_BNC_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_HSG_3D:
        case eDATA_NODE_BNC_HSG_BLENDING:
        case eDATA_NODE_BNC_HSG_USER:
        case eDATA_NODE_BNC_HSG_HDR:
        case eDATA_NODE_BNC_HSG_SRGB:
        case eDATA_NODE_BNC_HSG_SUPER_BRIGHT:
        case eDATA_NODE_BNC_HSG_SUPER_RED:
        case eDATA_NODE_BNC_HSG_3D_PASSIVE:
        case eDATA_NODE_BNC_HSG_HLG:

        case eDATA_NODE_BNC_HSG_USER_PRESENTATION:
        case eDATA_NODE_BNC_HSG_USER_VIDEO:
        case eDATA_NODE_BNC_HSG_USER_BRIGHT:
        case eDATA_NODE_BNC_HSG_USER_ENHANCED:
        case eDATA_NODE_BNC_HSG_USER_REC709:
        case eDATA_NODE_BNC_HSG_USER_REAL:
        case eDATA_NODE_BNC_HSG_USER_DICOMSIM:
        case eDATA_NODE_BNC_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_HSG_USER_3D:
        case eDATA_NODE_BNC_HSG_USER_BLENDING:
        case eDATA_NODE_BNC_HSG_USER_USER:
        case eDATA_NODE_BNC_HSG_USER_HDR:
        case eDATA_NODE_BNC_HSG_USER_SRGB:
        case eDATA_NODE_BNC_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_BNC_HSG_USER_SUPER_RED:
        case eDATA_NODE_BNC_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_BNC_HSG_USER_HLG:

        case eDATA_NODE_BNC_COLOR_PRESENTATION:
        case eDATA_NODE_BNC_COLOR_VIDEO:
        case eDATA_NODE_BNC_COLOR_BRIGHT:
        case eDATA_NODE_BNC_COLOR_ENHANCED:
        case eDATA_NODE_BNC_COLOR_REC709:
        case eDATA_NODE_BNC_COLOR_REAL:
        case eDATA_NODE_BNC_COLOR_DICOMSIM:
        case eDATA_NODE_BNC_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_COLOR_3D:
        case eDATA_NODE_BNC_COLOR_BLENDING:
        case eDATA_NODE_BNC_COLOR_USER:
        case eDATA_NODE_BNC_COLOR_HDR:
        case eDATA_NODE_BNC_COLOR_SRGB:
        case eDATA_NODE_BNC_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_BNC_COLOR_SUPER_RED:
        case eDATA_NODE_BNC_COLOR_3D_PASSIVE:
        case eDATA_NODE_BNC_COLOR_HLG:

        case eDATA_NODE_BNC_COLOR_USER_PRESENTATION:
        case eDATA_NODE_BNC_COLOR_USER_VIDEO:
        case eDATA_NODE_BNC_COLOR_USER_BRIGHT:
        case eDATA_NODE_BNC_COLOR_USER_ENHANCED:
        case eDATA_NODE_BNC_COLOR_USER_REC709:
        case eDATA_NODE_BNC_COLOR_USER_REAL:
        case eDATA_NODE_BNC_COLOR_USER_DICOMSIM:
        case eDATA_NODE_BNC_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_COLOR_USER_3D:
        case eDATA_NODE_BNC_COLOR_USER_BLENDING:
        case eDATA_NODE_BNC_COLOR_USER_USER:
        case eDATA_NODE_BNC_COLOR_USER_HDR:
        case eDATA_NODE_BNC_COLOR_USER_SRGB:
        case eDATA_NODE_BNC_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_BNC_COLOR_USER_SUPER_RED:
        case eDATA_NODE_BNC_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_BNC_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_BNC;
            break;

        case eDATA_NODE_HDMI1_COMMON:
        case eDATA_NODE_HDMI1_HSG_PRESENTATION:
        case eDATA_NODE_HDMI1_HSG_VIDEO:
        case eDATA_NODE_HDMI1_HSG_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_ENHANCED:
        case eDATA_NODE_HDMI1_HSG_REC709:
        case eDATA_NODE_HDMI1_HSG_REAL:
        case eDATA_NODE_HDMI1_HSG_DICOMSIM:
        case eDATA_NODE_HDMI1_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_HSG_3D:
        case eDATA_NODE_HDMI1_HSG_BLENDING:
        case eDATA_NODE_HDMI1_HSG_USER:
        case eDATA_NODE_HDMI1_HSG_HDR:
        case eDATA_NODE_HDMI1_HSG_SRGB:
        case eDATA_NODE_HDMI1_HSG_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_SUPER_RED:
        case eDATA_NODE_HDMI1_HSG_3D_PASSIVE:
        case eDATA_NODE_HDMI1_HSG_HLG:

        case eDATA_NODE_HDMI1_HSG_USER_PRESENTATION:
        case eDATA_NODE_HDMI1_HSG_USER_VIDEO:
        case eDATA_NODE_HDMI1_HSG_USER_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_USER_ENHANCED:
        case eDATA_NODE_HDMI1_HSG_USER_REC709:
        case eDATA_NODE_HDMI1_HSG_USER_REAL:
        case eDATA_NODE_HDMI1_HSG_USER_DICOMSIM:
        case eDATA_NODE_HDMI1_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_HSG_USER_3D:
        case eDATA_NODE_HDMI1_HSG_USER_BLENDING:
        case eDATA_NODE_HDMI1_HSG_USER_USER:
        case eDATA_NODE_HDMI1_HSG_USER_HDR:
        case eDATA_NODE_HDMI1_HSG_USER_SRGB:
        case eDATA_NODE_HDMI1_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_USER_SUPER_RED:
        case eDATA_NODE_HDMI1_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI1_HSG_USER_HLG:

        case eDATA_NODE_HDMI1_COLOR_PRESENTATION:
        case eDATA_NODE_HDMI1_COLOR_VIDEO:
        case eDATA_NODE_HDMI1_COLOR_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_ENHANCED:
        case eDATA_NODE_HDMI1_COLOR_REC709:
        case eDATA_NODE_HDMI1_COLOR_REAL:
        case eDATA_NODE_HDMI1_COLOR_DICOMSIM:
        case eDATA_NODE_HDMI1_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_COLOR_3D:
        case eDATA_NODE_HDMI1_COLOR_BLENDING:
        case eDATA_NODE_HDMI1_COLOR_USER:
        case eDATA_NODE_HDMI1_COLOR_HDR:
        case eDATA_NODE_HDMI1_COLOR_SRGB:
        case eDATA_NODE_HDMI1_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_SUPER_RED:
        case eDATA_NODE_HDMI1_COLOR_3D_PASSIVE:
        case eDATA_NODE_HDMI1_COLOR_HLG:

        case eDATA_NODE_HDMI1_COLOR_USER_PRESENTATION:
        case eDATA_NODE_HDMI1_COLOR_USER_VIDEO:
        case eDATA_NODE_HDMI1_COLOR_USER_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_USER_ENHANCED:
        case eDATA_NODE_HDMI1_COLOR_USER_REC709:
        case eDATA_NODE_HDMI1_COLOR_USER_REAL:
        case eDATA_NODE_HDMI1_COLOR_USER_DICOMSIM:
        case eDATA_NODE_HDMI1_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_COLOR_USER_3D:
        case eDATA_NODE_HDMI1_COLOR_USER_BLENDING:
        case eDATA_NODE_HDMI1_COLOR_USER_USER:
        case eDATA_NODE_HDMI1_COLOR_USER_HDR:
        case eDATA_NODE_HDMI1_COLOR_USER_SRGB:
        case eDATA_NODE_HDMI1_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_USER_SUPER_RED:
        case eDATA_NODE_HDMI1_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI1_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_HDMI1;
            break;

        case eDATA_NODE_HDMI2_COMMON:
        case eDATA_NODE_HDMI2_HSG_PRESENTATION:
        case eDATA_NODE_HDMI2_HSG_VIDEO:
        case eDATA_NODE_HDMI2_HSG_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_ENHANCED:
        case eDATA_NODE_HDMI2_HSG_REC709:
        case eDATA_NODE_HDMI2_HSG_REAL:
        case eDATA_NODE_HDMI2_HSG_DICOMSIM:
        case eDATA_NODE_HDMI2_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_HSG_3D:
        case eDATA_NODE_HDMI2_HSG_BLENDING:
        case eDATA_NODE_HDMI2_HSG_USER:
        case eDATA_NODE_HDMI2_HSG_HDR:
        case eDATA_NODE_HDMI2_HSG_SRGB:
        case eDATA_NODE_HDMI2_HSG_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_SUPER_RED:
        case eDATA_NODE_HDMI2_HSG_3D_PASSIVE:
        case eDATA_NODE_HDMI2_HSG_HLG:

        case eDATA_NODE_HDMI2_HSG_USER_PRESENTATION:
        case eDATA_NODE_HDMI2_HSG_USER_VIDEO:
        case eDATA_NODE_HDMI2_HSG_USER_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_USER_ENHANCED:
        case eDATA_NODE_HDMI2_HSG_USER_REC709:
        case eDATA_NODE_HDMI2_HSG_USER_REAL:
        case eDATA_NODE_HDMI2_HSG_USER_DICOMSIM:
        case eDATA_NODE_HDMI2_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_HSG_USER_3D:
        case eDATA_NODE_HDMI2_HSG_USER_BLENDING:
        case eDATA_NODE_HDMI2_HSG_USER_USER:
        case eDATA_NODE_HDMI2_HSG_USER_HDR:
        case eDATA_NODE_HDMI2_HSG_USER_SRGB:
        case eDATA_NODE_HDMI2_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_USER_SUPER_RED:
        case eDATA_NODE_HDMI2_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI2_HSG_USER_HLG:

        case eDATA_NODE_HDMI2_COLOR_PRESENTATION:
        case eDATA_NODE_HDMI2_COLOR_VIDEO:
        case eDATA_NODE_HDMI2_COLOR_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_ENHANCED:
        case eDATA_NODE_HDMI2_COLOR_REC709:
        case eDATA_NODE_HDMI2_COLOR_REAL:
        case eDATA_NODE_HDMI2_COLOR_DICOMSIM:
        case eDATA_NODE_HDMI2_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_COLOR_3D:
        case eDATA_NODE_HDMI2_COLOR_BLENDING:
        case eDATA_NODE_HDMI2_COLOR_USER:
        case eDATA_NODE_HDMI2_COLOR_HDR:
        case eDATA_NODE_HDMI2_COLOR_SRGB:
        case eDATA_NODE_HDMI2_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_SUPER_RED:
        case eDATA_NODE_HDMI2_COLOR_3D_PASSIVE:
        case eDATA_NODE_HDMI2_COLOR_HLG:

        case eDATA_NODE_HDMI2_COLOR_USER_PRESENTATION:
        case eDATA_NODE_HDMI2_COLOR_USER_VIDEO:
        case eDATA_NODE_HDMI2_COLOR_USER_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_USER_ENHANCED:
        case eDATA_NODE_HDMI2_COLOR_USER_REC709:
        case eDATA_NODE_HDMI2_COLOR_USER_REAL:
        case eDATA_NODE_HDMI2_COLOR_USER_DICOMSIM:
        case eDATA_NODE_HDMI2_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_COLOR_USER_3D:
        case eDATA_NODE_HDMI2_COLOR_USER_BLENDING:
        case eDATA_NODE_HDMI2_COLOR_USER_USER:
        case eDATA_NODE_HDMI2_COLOR_USER_HDR:
        case eDATA_NODE_HDMI2_COLOR_USER_SRGB:
        case eDATA_NODE_HDMI2_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_USER_SUPER_RED:
        case eDATA_NODE_HDMI2_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI2_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_HDMI2;
            break;

        case eDATA_NODE_DVID_COMMON:
        case eDATA_NODE_DVID_HSG_PRESENTATION:
        case eDATA_NODE_DVID_HSG_VIDEO:
        case eDATA_NODE_DVID_HSG_BRIGHT:
        case eDATA_NODE_DVID_HSG_ENHANCED:
        case eDATA_NODE_DVID_HSG_REC709:
        case eDATA_NODE_DVID_HSG_REAL:
        case eDATA_NODE_DVID_HSG_DICOMSIM:
        case eDATA_NODE_DVID_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_HSG_3D:
        case eDATA_NODE_DVID_HSG_BLENDING:
        case eDATA_NODE_DVID_HSG_USER:
        case eDATA_NODE_DVID_HSG_HDR:
        case eDATA_NODE_DVID_HSG_SRGB:
        case eDATA_NODE_DVID_HSG_SUPER_BRIGHT:
        case eDATA_NODE_DVID_HSG_SUPER_RED:
        case eDATA_NODE_DVID_HSG_3D_PASSIVE:
        case eDATA_NODE_DVID_HSG_HLG:

        case eDATA_NODE_DVID_HSG_USER_PRESENTATION:
        case eDATA_NODE_DVID_HSG_USER_VIDEO:
        case eDATA_NODE_DVID_HSG_USER_BRIGHT:
        case eDATA_NODE_DVID_HSG_USER_ENHANCED:
        case eDATA_NODE_DVID_HSG_USER_REC709:
        case eDATA_NODE_DVID_HSG_USER_REAL:
        case eDATA_NODE_DVID_HSG_USER_DICOMSIM:
        case eDATA_NODE_DVID_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_HSG_USER_3D:
        case eDATA_NODE_DVID_HSG_USER_BLENDING:
        case eDATA_NODE_DVID_HSG_USER_USER:
        case eDATA_NODE_DVID_HSG_USER_HDR:
        case eDATA_NODE_DVID_HSG_USER_SRGB:
        case eDATA_NODE_DVID_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_DVID_HSG_USER_SUPER_RED:
        case eDATA_NODE_DVID_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_DVID_HSG_USER_HLG:

        case eDATA_NODE_DVID_COLOR_PRESENTATION:
        case eDATA_NODE_DVID_COLOR_VIDEO:
        case eDATA_NODE_DVID_COLOR_BRIGHT:
        case eDATA_NODE_DVID_COLOR_ENHANCED:
        case eDATA_NODE_DVID_COLOR_REC709:
        case eDATA_NODE_DVID_COLOR_REAL:
        case eDATA_NODE_DVID_COLOR_DICOMSIM:
        case eDATA_NODE_DVID_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_COLOR_3D:
        case eDATA_NODE_DVID_COLOR_BLENDING:
        case eDATA_NODE_DVID_COLOR_USER:
        case eDATA_NODE_DVID_COLOR_HDR:
        case eDATA_NODE_DVID_COLOR_SRGB:
        case eDATA_NODE_DVID_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_DVID_COLOR_SUPER_RED:
        case eDATA_NODE_DVID_COLOR_3D_PASSIVE:
        case eDATA_NODE_DVID_COLOR_HLG:

        case eDATA_NODE_DVID_COLOR_USER_PRESENTATION:
        case eDATA_NODE_DVID_COLOR_USER_VIDEO:
        case eDATA_NODE_DVID_COLOR_USER_BRIGHT:
        case eDATA_NODE_DVID_COLOR_USER_ENHANCED:
        case eDATA_NODE_DVID_COLOR_USER_REC709:
        case eDATA_NODE_DVID_COLOR_USER_REAL:
        case eDATA_NODE_DVID_COLOR_USER_DICOMSIM:
        case eDATA_NODE_DVID_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_COLOR_USER_3D:
        case eDATA_NODE_DVID_COLOR_USER_BLENDING:
        case eDATA_NODE_DVID_COLOR_USER_USER:
        case eDATA_NODE_DVID_COLOR_USER_HDR:
        case eDATA_NODE_DVID_COLOR_USER_SRGB:
        case eDATA_NODE_DVID_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_DVID_COLOR_USER_SUPER_RED:
        case eDATA_NODE_DVID_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_DVID_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_DVI;
            break;

        case eDATA_NODE_DP_COMMON:
        case eDATA_NODE_DP_HSG_PRESENTATION:
        case eDATA_NODE_DP_HSG_VIDEO:
        case eDATA_NODE_DP_HSG_BRIGHT:
        case eDATA_NODE_DP_HSG_ENHANCED:
        case eDATA_NODE_DP_HSG_REC709:
        case eDATA_NODE_DP_HSG_REAL:
        case eDATA_NODE_DP_HSG_DICOMSIM:
        case eDATA_NODE_DP_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_DP_HSG_3D:
        case eDATA_NODE_DP_HSG_BLENDING:
        case eDATA_NODE_DP_HSG_USER:
        case eDATA_NODE_DP_HSG_HDR:
        case eDATA_NODE_DP_HSG_SRGB:
        case eDATA_NODE_DP_HSG_SUPER_BRIGHT:
        case eDATA_NODE_DP_HSG_SUPER_RED:
        case eDATA_NODE_DP_HSG_3D_PASSIVE:
        case eDATA_NODE_DP_HSG_HLG:

        case eDATA_NODE_DP_HSG_USER_PRESENTATION:
        case eDATA_NODE_DP_HSG_USER_VIDEO:
        case eDATA_NODE_DP_HSG_USER_BRIGHT:
        case eDATA_NODE_DP_HSG_USER_ENHANCED:
        case eDATA_NODE_DP_HSG_USER_REC709:
        case eDATA_NODE_DP_HSG_USER_REAL:
        case eDATA_NODE_DP_HSG_USER_DICOMSIM:
        case eDATA_NODE_DP_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DP_HSG_USER_3D:
        case eDATA_NODE_DP_HSG_USER_BLENDING:
        case eDATA_NODE_DP_HSG_USER_USER:
        case eDATA_NODE_DP_HSG_USER_HDR:
        case eDATA_NODE_DP_HSG_USER_SRGB:
        case eDATA_NODE_DP_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_DP_HSG_USER_SUPER_RED:
        case eDATA_NODE_DP_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_DP_HSG_USER_HLG:

        case eDATA_NODE_DP_COLOR_PRESENTATION:
        case eDATA_NODE_DP_COLOR_VIDEO:
        case eDATA_NODE_DP_COLOR_BRIGHT:
        case eDATA_NODE_DP_COLOR_ENHANCED:
        case eDATA_NODE_DP_COLOR_REC709:
        case eDATA_NODE_DP_COLOR_REAL:
        case eDATA_NODE_DP_COLOR_DICOMSIM:
        case eDATA_NODE_DP_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_DP_COLOR_3D:
        case eDATA_NODE_DP_COLOR_BLENDING:
        case eDATA_NODE_DP_COLOR_USER:
        case eDATA_NODE_DP_COLOR_HDR:
        case eDATA_NODE_DP_COLOR_SRGB:
        case eDATA_NODE_DP_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_DP_COLOR_SUPER_RED:
        case eDATA_NODE_DP_COLOR_3D_PASSIVE:
        case eDATA_NODE_DP_COLOR_HLG:

        case eDATA_NODE_DP_COLOR_USER_PRESENTATION:
        case eDATA_NODE_DP_COLOR_USER_VIDEO:
        case eDATA_NODE_DP_COLOR_USER_BRIGHT:
        case eDATA_NODE_DP_COLOR_USER_ENHANCED:
        case eDATA_NODE_DP_COLOR_USER_REC709:
        case eDATA_NODE_DP_COLOR_USER_REAL:
        case eDATA_NODE_DP_COLOR_USER_DICOMSIM:
        case eDATA_NODE_DP_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DP_COLOR_USER_3D:
        case eDATA_NODE_DP_COLOR_USER_BLENDING:
        case eDATA_NODE_DP_COLOR_USER_USER:
        case eDATA_NODE_DP_COLOR_USER_HDR:
        case eDATA_NODE_DP_COLOR_USER_SRGB:
        case eDATA_NODE_DP_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_DP_COLOR_USER_SUPER_RED:
        case eDATA_NODE_DP_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_DP_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_DISPLAYPORT;
            break;

        case eDATA_NODE_3GSDI_COMMON:
        case eDATA_NODE_3GSDI_HSG_PRESENTATION:
        case eDATA_NODE_3GSDI_HSG_VIDEO:
        case eDATA_NODE_3GSDI_HSG_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_ENHANCED:
        case eDATA_NODE_3GSDI_HSG_REC709:
        case eDATA_NODE_3GSDI_HSG_REAL:
        case eDATA_NODE_3GSDI_HSG_DICOMSIM:
        case eDATA_NODE_3GSDI_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_HSG_3D:
        case eDATA_NODE_3GSDI_HSG_BLENDING:
        case eDATA_NODE_3GSDI_HSG_USER:
        case eDATA_NODE_3GSDI_HSG_HDR:
        case eDATA_NODE_3GSDI_HSG_SRGB:
        case eDATA_NODE_3GSDI_HSG_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_SUPER_RED:
        case eDATA_NODE_3GSDI_HSG_3D_PASSIVE:
        case eDATA_NODE_3GSDI_HSG_HLG:

        case eDATA_NODE_3GSDI_HSG_USER_PRESENTATION:
        case eDATA_NODE_3GSDI_HSG_USER_VIDEO:
        case eDATA_NODE_3GSDI_HSG_USER_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_USER_ENHANCED:
        case eDATA_NODE_3GSDI_HSG_USER_REC709:
        case eDATA_NODE_3GSDI_HSG_USER_REAL:
        case eDATA_NODE_3GSDI_HSG_USER_DICOMSIM:
        case eDATA_NODE_3GSDI_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_HSG_USER_3D:
        case eDATA_NODE_3GSDI_HSG_USER_BLENDING:
        case eDATA_NODE_3GSDI_HSG_USER_USER:
        case eDATA_NODE_3GSDI_HSG_USER_HDR:
        case eDATA_NODE_3GSDI_HSG_USER_SRGB:
        case eDATA_NODE_3GSDI_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_USER_SUPER_RED:
        case eDATA_NODE_3GSDI_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_3GSDI_HSG_USER_HLG:

        case eDATA_NODE_3GSDI_COLOR_PRESENTATION:
        case eDATA_NODE_3GSDI_COLOR_VIDEO:
        case eDATA_NODE_3GSDI_COLOR_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_ENHANCED:
        case eDATA_NODE_3GSDI_COLOR_REC709:
        case eDATA_NODE_3GSDI_COLOR_REAL:
        case eDATA_NODE_3GSDI_COLOR_DICOMSIM:
        case eDATA_NODE_3GSDI_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_COLOR_3D:
        case eDATA_NODE_3GSDI_COLOR_BLENDING:
        case eDATA_NODE_3GSDI_COLOR_USER:
        case eDATA_NODE_3GSDI_COLOR_HDR:
        case eDATA_NODE_3GSDI_COLOR_SRGB:
        case eDATA_NODE_3GSDI_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_SUPER_RED:
        case eDATA_NODE_3GSDI_COLOR_3D_PASSIVE:
        case eDATA_NODE_3GSDI_COLOR_HLG:

        case eDATA_NODE_3GSDI_COLOR_USER_PRESENTATION:
        case eDATA_NODE_3GSDI_COLOR_USER_VIDEO:
        case eDATA_NODE_3GSDI_COLOR_USER_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_USER_ENHANCED:
        case eDATA_NODE_3GSDI_COLOR_USER_REC709:
        case eDATA_NODE_3GSDI_COLOR_USER_REAL:
        case eDATA_NODE_3GSDI_COLOR_USER_DICOMSIM:
        case eDATA_NODE_3GSDI_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_COLOR_USER_3D:
        case eDATA_NODE_3GSDI_COLOR_USER_BLENDING:
        case eDATA_NODE_3GSDI_COLOR_USER_USER:
        case eDATA_NODE_3GSDI_COLOR_USER_HDR:
        case eDATA_NODE_3GSDI_COLOR_USER_SRGB:
        case eDATA_NODE_3GSDI_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_USER_SUPER_RED:
        case eDATA_NODE_3GSDI_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_3GSDI_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_3GSDI;
            break;

        case eDATA_NODE_HDBASET_COMMON:
        case eDATA_NODE_HDBASET_HSG_PRESENTATION:
        case eDATA_NODE_HDBASET_HSG_VIDEO:
        case eDATA_NODE_HDBASET_HSG_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_ENHANCED:
        case eDATA_NODE_HDBASET_HSG_REC709:
        case eDATA_NODE_HDBASET_HSG_REAL:
        case eDATA_NODE_HDBASET_HSG_DICOMSIM:
        case eDATA_NODE_HDBASET_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_HSG_3D:
        case eDATA_NODE_HDBASET_HSG_BLENDING:
        case eDATA_NODE_HDBASET_HSG_USER:
        case eDATA_NODE_HDBASET_HSG_HDR:
        case eDATA_NODE_HDBASET_HSG_SRGB:
        case eDATA_NODE_HDBASET_HSG_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_SUPER_RED:
        case eDATA_NODE_HDBASET_HSG_3D_PASSIVE:
        case eDATA_NODE_HDBASET_HSG_HLG:

        case eDATA_NODE_HDBASET_HSG_USER_PRESENTATION:
        case eDATA_NODE_HDBASET_HSG_USER_VIDEO:
        case eDATA_NODE_HDBASET_HSG_USER_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_USER_ENHANCED:
        case eDATA_NODE_HDBASET_HSG_USER_REC709:
        case eDATA_NODE_HDBASET_HSG_USER_REAL:
        case eDATA_NODE_HDBASET_HSG_USER_DICOMSIM:
        case eDATA_NODE_HDBASET_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_HSG_USER_3D:
        case eDATA_NODE_HDBASET_HSG_USER_BLENDING:
        case eDATA_NODE_HDBASET_HSG_USER_USER:
        case eDATA_NODE_HDBASET_HSG_USER_HDR:
        case eDATA_NODE_HDBASET_HSG_USER_SRGB:
        case eDATA_NODE_HDBASET_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_USER_SUPER_RED:
        case eDATA_NODE_HDBASET_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_HDBASET_HSG_USER_HLG:

        case eDATA_NODE_HDBASET_COLOR_PRESENTATION:
        case eDATA_NODE_HDBASET_COLOR_VIDEO:
        case eDATA_NODE_HDBASET_COLOR_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_ENHANCED:
        case eDATA_NODE_HDBASET_COLOR_REC709:
        case eDATA_NODE_HDBASET_COLOR_REAL:
        case eDATA_NODE_HDBASET_COLOR_DICOMSIM:
        case eDATA_NODE_HDBASET_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_COLOR_3D:
        case eDATA_NODE_HDBASET_COLOR_BLENDING:
        case eDATA_NODE_HDBASET_COLOR_USER:
        case eDATA_NODE_HDBASET_COLOR_HDR:
        case eDATA_NODE_HDBASET_COLOR_SRGB:
        case eDATA_NODE_HDBASET_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_SUPER_RED:
        case eDATA_NODE_HDBASET_COLOR_3D_PASSIVE:
        case eDATA_NODE_HDBASET_COLOR_HLG:

        case eDATA_NODE_HDBASET_COLOR_USER_PRESENTATION:
        case eDATA_NODE_HDBASET_COLOR_USER_VIDEO:
        case eDATA_NODE_HDBASET_COLOR_USER_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_USER_ENHANCED:
        case eDATA_NODE_HDBASET_COLOR_USER_REC709:
        case eDATA_NODE_HDBASET_COLOR_USER_REAL:
        case eDATA_NODE_HDBASET_COLOR_USER_DICOMSIM:
        case eDATA_NODE_HDBASET_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_COLOR_USER_3D:
        case eDATA_NODE_HDBASET_COLOR_USER_BLENDING:
        case eDATA_NODE_HDBASET_COLOR_USER_USER:
        case eDATA_NODE_HDBASET_COLOR_USER_HDR:
        case eDATA_NODE_HDBASET_COLOR_USER_SRGB:
        case eDATA_NODE_HDBASET_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_USER_SUPER_RED:
        case eDATA_NODE_HDBASET_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_HDBASET_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_HDBASET;
            break;

        case eDATA_NODE_12GSDI_COMMON:
        case eDATA_NODE_12GSDI_HSG_PRESENTATION:
        case eDATA_NODE_12GSDI_HSG_VIDEO:
        case eDATA_NODE_12GSDI_HSG_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_ENHANCED:
        case eDATA_NODE_12GSDI_HSG_REC709:
        case eDATA_NODE_12GSDI_HSG_REAL:
        case eDATA_NODE_12GSDI_HSG_DICOMSIM:
        case eDATA_NODE_12GSDI_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_HSG_3D:
        case eDATA_NODE_12GSDI_HSG_BLENDING:
        case eDATA_NODE_12GSDI_HSG_USER:
        case eDATA_NODE_12GSDI_HSG_HDR:
        case eDATA_NODE_12GSDI_HSG_SRGB:
        case eDATA_NODE_12GSDI_HSG_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_SUPER_RED:
        case eDATA_NODE_12GSDI_HSG_3D_PASSIVE:
        case eDATA_NODE_12GSDI_HSG_HLG:

        case eDATA_NODE_12GSDI_HSG_USER_PRESENTATION:
        case eDATA_NODE_12GSDI_HSG_USER_VIDEO:
        case eDATA_NODE_12GSDI_HSG_USER_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_USER_ENHANCED:
        case eDATA_NODE_12GSDI_HSG_USER_REC709:
        case eDATA_NODE_12GSDI_HSG_USER_REAL:
        case eDATA_NODE_12GSDI_HSG_USER_DICOMSIM:
        case eDATA_NODE_12GSDI_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_HSG_USER_3D:
        case eDATA_NODE_12GSDI_HSG_USER_BLENDING:
        case eDATA_NODE_12GSDI_HSG_USER_USER:
        case eDATA_NODE_12GSDI_HSG_USER_HDR:
        case eDATA_NODE_12GSDI_HSG_USER_SRGB:
        case eDATA_NODE_12GSDI_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_USER_SUPER_RED:
        case eDATA_NODE_12GSDI_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_12GSDI_HSG_USER_HLG:

        case eDATA_NODE_12GSDI_COLOR_PRESENTATION:
        case eDATA_NODE_12GSDI_COLOR_VIDEO:
        case eDATA_NODE_12GSDI_COLOR_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_ENHANCED:
        case eDATA_NODE_12GSDI_COLOR_REC709:
        case eDATA_NODE_12GSDI_COLOR_REAL:
        case eDATA_NODE_12GSDI_COLOR_DICOMSIM:
        case eDATA_NODE_12GSDI_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_COLOR_3D:
        case eDATA_NODE_12GSDI_COLOR_BLENDING:
        case eDATA_NODE_12GSDI_COLOR_USER:
        case eDATA_NODE_12GSDI_COLOR_HDR:
        case eDATA_NODE_12GSDI_COLOR_SRGB:
        case eDATA_NODE_12GSDI_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_SUPER_RED:
        case eDATA_NODE_12GSDI_COLOR_3D_PASSIVE:
        case eDATA_NODE_12GSDI_COLOR_HLG:

        case eDATA_NODE_12GSDI_COLOR_USER_PRESENTATION:
        case eDATA_NODE_12GSDI_COLOR_USER_VIDEO:
        case eDATA_NODE_12GSDI_COLOR_USER_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_USER_ENHANCED:
        case eDATA_NODE_12GSDI_COLOR_USER_REC709:
        case eDATA_NODE_12GSDI_COLOR_USER_REAL:
        case eDATA_NODE_12GSDI_COLOR_USER_DICOMSIM:
        case eDATA_NODE_12GSDI_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_COLOR_USER_3D:
        case eDATA_NODE_12GSDI_COLOR_USER_BLENDING:
        case eDATA_NODE_12GSDI_COLOR_USER_USER:
        case eDATA_NODE_12GSDI_COLOR_USER_HDR:
        case eDATA_NODE_12GSDI_COLOR_USER_SRGB:
        case eDATA_NODE_12GSDI_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_USER_SUPER_RED:
        case eDATA_NODE_12GSDI_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_12GSDI_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_12GSDI;
            break;

        case eDATA_NODE_PRESENTER_COMMON:
        case eDATA_NODE_PRESENTER_HSG_PRESENTATION:
        case eDATA_NODE_PRESENTER_HSG_VIDEO:
        case eDATA_NODE_PRESENTER_HSG_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_ENHANCED:
        case eDATA_NODE_PRESENTER_HSG_REC709:
        case eDATA_NODE_PRESENTER_HSG_REAL:
        case eDATA_NODE_PRESENTER_HSG_DICOMSIM:
        case eDATA_NODE_PRESENTER_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_HSG_3D:
        case eDATA_NODE_PRESENTER_HSG_BLENDING:
        case eDATA_NODE_PRESENTER_HSG_USER:
        case eDATA_NODE_PRESENTER_HSG_HDR:
        case eDATA_NODE_PRESENTER_HSG_SRGB:
        case eDATA_NODE_PRESENTER_HSG_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_SUPER_RED:
        case eDATA_NODE_PRESENTER_HSG_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_HSG_HLG:

        case eDATA_NODE_PRESENTER_HSG_USER_PRESENTATION:
        case eDATA_NODE_PRESENTER_HSG_USER_VIDEO:
        case eDATA_NODE_PRESENTER_HSG_USER_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_USER_ENHANCED:
        case eDATA_NODE_PRESENTER_HSG_USER_REC709:
        case eDATA_NODE_PRESENTER_HSG_USER_REAL:
        case eDATA_NODE_PRESENTER_HSG_USER_DICOMSIM:
        case eDATA_NODE_PRESENTER_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_HSG_USER_3D:
        case eDATA_NODE_PRESENTER_HSG_USER_BLENDING:
        case eDATA_NODE_PRESENTER_HSG_USER_USER:
        case eDATA_NODE_PRESENTER_HSG_USER_HDR:
        case eDATA_NODE_PRESENTER_HSG_USER_SRGB:
        case eDATA_NODE_PRESENTER_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_USER_SUPER_RED:
        case eDATA_NODE_PRESENTER_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_HSG_USER_HLG:

        case eDATA_NODE_PRESENTER_COLOR_PRESENTATION:
        case eDATA_NODE_PRESENTER_COLOR_VIDEO:
        case eDATA_NODE_PRESENTER_COLOR_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_ENHANCED:
        case eDATA_NODE_PRESENTER_COLOR_REC709:
        case eDATA_NODE_PRESENTER_COLOR_REAL:
        case eDATA_NODE_PRESENTER_COLOR_DICOMSIM:
        case eDATA_NODE_PRESENTER_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_COLOR_3D:
        case eDATA_NODE_PRESENTER_COLOR_BLENDING:
        case eDATA_NODE_PRESENTER_COLOR_USER:
        case eDATA_NODE_PRESENTER_COLOR_HDR:
        case eDATA_NODE_PRESENTER_COLOR_SRGB:
        case eDATA_NODE_PRESENTER_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_SUPER_RED:
        case eDATA_NODE_PRESENTER_COLOR_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_COLOR_HLG:

        case eDATA_NODE_PRESENTER_COLOR_USER_PRESENTATION:
        case eDATA_NODE_PRESENTER_COLOR_USER_VIDEO:
        case eDATA_NODE_PRESENTER_COLOR_USER_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_USER_ENHANCED:
        case eDATA_NODE_PRESENTER_COLOR_USER_REC709:
        case eDATA_NODE_PRESENTER_COLOR_USER_REAL:
        case eDATA_NODE_PRESENTER_COLOR_USER_DICOMSIM:
        case eDATA_NODE_PRESENTER_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_COLOR_USER_3D:
        case eDATA_NODE_PRESENTER_COLOR_USER_BLENDING:
        case eDATA_NODE_PRESENTER_COLOR_USER_USER:
        case eDATA_NODE_PRESENTER_COLOR_USER_HDR:
        case eDATA_NODE_PRESENTER_COLOR_USER_SRGB:
        case eDATA_NODE_PRESENTER_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_USER_SUPER_RED:
        case eDATA_NODE_PRESENTER_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_PRESENTER;
            break;

        case eDATA_NODE_CARDREADER_COMMON:
        case eDATA_NODE_CARDREADER_HSG_PRESENTATION:
        case eDATA_NODE_CARDREADER_HSG_VIDEO:
        case eDATA_NODE_CARDREADER_HSG_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_ENHANCED:
        case eDATA_NODE_CARDREADER_HSG_REC709:
        case eDATA_NODE_CARDREADER_HSG_REAL:
        case eDATA_NODE_CARDREADER_HSG_DICOMSIM:
        case eDATA_NODE_CARDREADER_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_HSG_3D:
        case eDATA_NODE_CARDREADER_HSG_BLENDING:
        case eDATA_NODE_CARDREADER_HSG_USER:
        case eDATA_NODE_CARDREADER_HSG_HDR:
        case eDATA_NODE_CARDREADER_HSG_SRGB:
        case eDATA_NODE_CARDREADER_HSG_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_SUPER_RED:
        case eDATA_NODE_CARDREADER_HSG_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_HSG_HLG:

        case eDATA_NODE_CARDREADER_HSG_USER_PRESENTATION:
        case eDATA_NODE_CARDREADER_HSG_USER_VIDEO:
        case eDATA_NODE_CARDREADER_HSG_USER_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_USER_ENHANCED:
        case eDATA_NODE_CARDREADER_HSG_USER_REC709:
        case eDATA_NODE_CARDREADER_HSG_USER_REAL:
        case eDATA_NODE_CARDREADER_HSG_USER_DICOMSIM:
        case eDATA_NODE_CARDREADER_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_HSG_USER_3D:
        case eDATA_NODE_CARDREADER_HSG_USER_BLENDING:
        case eDATA_NODE_CARDREADER_HSG_USER_USER:
        case eDATA_NODE_CARDREADER_HSG_USER_HDR:
        case eDATA_NODE_CARDREADER_HSG_USER_SRGB:
        case eDATA_NODE_CARDREADER_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_USER_SUPER_RED:
        case eDATA_NODE_CARDREADER_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_HSG_USER_HLG:

        case eDATA_NODE_CARDREADER_COLOR_PRESENTATION:
        case eDATA_NODE_CARDREADER_COLOR_VIDEO:
        case eDATA_NODE_CARDREADER_COLOR_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_ENHANCED:
        case eDATA_NODE_CARDREADER_COLOR_REC709:
        case eDATA_NODE_CARDREADER_COLOR_REAL:
        case eDATA_NODE_CARDREADER_COLOR_DICOMSIM:
        case eDATA_NODE_CARDREADER_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_COLOR_3D:
        case eDATA_NODE_CARDREADER_COLOR_BLENDING:
        case eDATA_NODE_CARDREADER_COLOR_USER:
        case eDATA_NODE_CARDREADER_COLOR_HDR:
        case eDATA_NODE_CARDREADER_COLOR_SRGB:
        case eDATA_NODE_CARDREADER_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_SUPER_RED:
        case eDATA_NODE_CARDREADER_COLOR_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_COLOR_HLG:

        case eDATA_NODE_CARDREADER_COLOR_USER_PRESENTATION:
        case eDATA_NODE_CARDREADER_COLOR_USER_VIDEO:
        case eDATA_NODE_CARDREADER_COLOR_USER_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_USER_ENHANCED:
        case eDATA_NODE_CARDREADER_COLOR_USER_REC709:
        case eDATA_NODE_CARDREADER_COLOR_USER_REAL:
        case eDATA_NODE_CARDREADER_COLOR_USER_DICOMSIM:
        case eDATA_NODE_CARDREADER_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_COLOR_USER_3D:
        case eDATA_NODE_CARDREADER_COLOR_USER_BLENDING:
        case eDATA_NODE_CARDREADER_COLOR_USER_USER:
        case eDATA_NODE_CARDREADER_COLOR_USER_HDR:
        case eDATA_NODE_CARDREADER_COLOR_USER_SRGB:
        case eDATA_NODE_CARDREADER_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_USER_SUPER_RED:
        case eDATA_NODE_CARDREADER_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_CARDREADER;
            break;

        case eDATA_NODE_MINIUSB_COMMON:
        case eDATA_NODE_MINIUSB_HSG_PRESENTATION:
        case eDATA_NODE_MINIUSB_HSG_VIDEO:
        case eDATA_NODE_MINIUSB_HSG_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_ENHANCED:
        case eDATA_NODE_MINIUSB_HSG_REC709:
        case eDATA_NODE_MINIUSB_HSG_REAL:
        case eDATA_NODE_MINIUSB_HSG_DICOMSIM:
        case eDATA_NODE_MINIUSB_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_HSG_3D:
        case eDATA_NODE_MINIUSB_HSG_BLENDING:
        case eDATA_NODE_MINIUSB_HSG_USER:
        case eDATA_NODE_MINIUSB_HSG_HDR:
        case eDATA_NODE_MINIUSB_HSG_SRGB:
        case eDATA_NODE_MINIUSB_HSG_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_SUPER_RED:
        case eDATA_NODE_MINIUSB_HSG_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_HSG_HLG:

        case eDATA_NODE_MINIUSB_HSG_USER_PRESENTATION:
        case eDATA_NODE_MINIUSB_HSG_USER_VIDEO:
        case eDATA_NODE_MINIUSB_HSG_USER_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_USER_ENHANCED:
        case eDATA_NODE_MINIUSB_HSG_USER_REC709:
        case eDATA_NODE_MINIUSB_HSG_USER_REAL:
        case eDATA_NODE_MINIUSB_HSG_USER_DICOMSIM:
        case eDATA_NODE_MINIUSB_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_HSG_USER_3D:
        case eDATA_NODE_MINIUSB_HSG_USER_BLENDING:
        case eDATA_NODE_MINIUSB_HSG_USER_USER:
        case eDATA_NODE_MINIUSB_HSG_USER_HDR:
        case eDATA_NODE_MINIUSB_HSG_USER_SRGB:
        case eDATA_NODE_MINIUSB_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_USER_SUPER_RED:
        case eDATA_NODE_MINIUSB_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_HSG_USER_HLG:

        case eDATA_NODE_MINIUSB_COLOR_PRESENTATION:
        case eDATA_NODE_MINIUSB_COLOR_VIDEO:
        case eDATA_NODE_MINIUSB_COLOR_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_ENHANCED:
        case eDATA_NODE_MINIUSB_COLOR_REC709:
        case eDATA_NODE_MINIUSB_COLOR_REAL:
        case eDATA_NODE_MINIUSB_COLOR_DICOMSIM:
        case eDATA_NODE_MINIUSB_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_COLOR_3D:
        case eDATA_NODE_MINIUSB_COLOR_BLENDING:
        case eDATA_NODE_MINIUSB_COLOR_USER:
        case eDATA_NODE_MINIUSB_COLOR_HDR:
        case eDATA_NODE_MINIUSB_COLOR_SRGB:
        case eDATA_NODE_MINIUSB_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_SUPER_RED:
        case eDATA_NODE_MINIUSB_COLOR_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_COLOR_HLG:

        case eDATA_NODE_MINIUSB_COLOR_USER_PRESENTATION:
        case eDATA_NODE_MINIUSB_COLOR_USER_VIDEO:
        case eDATA_NODE_MINIUSB_COLOR_USER_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_USER_ENHANCED:
        case eDATA_NODE_MINIUSB_COLOR_USER_REC709:
        case eDATA_NODE_MINIUSB_COLOR_USER_REAL:
        case eDATA_NODE_MINIUSB_COLOR_USER_DICOMSIM:
        case eDATA_NODE_MINIUSB_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_COLOR_USER_3D:
        case eDATA_NODE_MINIUSB_COLOR_USER_BLENDING:
        case eDATA_NODE_MINIUSB_COLOR_USER_USER:
        case eDATA_NODE_MINIUSB_COLOR_USER_HDR:
        case eDATA_NODE_MINIUSB_COLOR_USER_SRGB:
        case eDATA_NODE_MINIUSB_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_USER_SUPER_RED:
        case eDATA_NODE_MINIUSB_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_MINIUSB;
            break;

        case eDATA_NODE_SLOT1_COMMON:
        case eDATA_NODE_SLOT1_HSG_PRESENTATION:
        case eDATA_NODE_SLOT1_HSG_VIDEO:
        case eDATA_NODE_SLOT1_HSG_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_ENHANCED:
        case eDATA_NODE_SLOT1_HSG_REC709:
        case eDATA_NODE_SLOT1_HSG_REAL:
        case eDATA_NODE_SLOT1_HSG_DICOMSIM:
        case eDATA_NODE_SLOT1_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_HSG_3D:
        case eDATA_NODE_SLOT1_HSG_BLENDING:
        case eDATA_NODE_SLOT1_HSG_USER:
        case eDATA_NODE_SLOT1_HSG_HDR:
        case eDATA_NODE_SLOT1_HSG_SRGB:
        case eDATA_NODE_SLOT1_HSG_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_SUPER_RED:
        case eDATA_NODE_SLOT1_HSG_3D_PASSIVE:
        case eDATA_NODE_SLOT1_HSG_HLG:

        case eDATA_NODE_SLOT1_HSG_USER_PRESENTATION:
        case eDATA_NODE_SLOT1_HSG_USER_VIDEO:
        case eDATA_NODE_SLOT1_HSG_USER_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_USER_ENHANCED:
        case eDATA_NODE_SLOT1_HSG_USER_REC709:
        case eDATA_NODE_SLOT1_HSG_USER_REAL:
        case eDATA_NODE_SLOT1_HSG_USER_DICOMSIM:
        case eDATA_NODE_SLOT1_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_HSG_USER_3D:
        case eDATA_NODE_SLOT1_HSG_USER_BLENDING:
        case eDATA_NODE_SLOT1_HSG_USER_USER:
        case eDATA_NODE_SLOT1_HSG_USER_HDR:
        case eDATA_NODE_SLOT1_HSG_USER_SRGB:
        case eDATA_NODE_SLOT1_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_USER_SUPER_RED:
        case eDATA_NODE_SLOT1_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT1_HSG_USER_HLG:

        case eDATA_NODE_SLOT1_COLOR_PRESENTATION:
        case eDATA_NODE_SLOT1_COLOR_VIDEO:
        case eDATA_NODE_SLOT1_COLOR_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_ENHANCED:
        case eDATA_NODE_SLOT1_COLOR_REC709:
        case eDATA_NODE_SLOT1_COLOR_REAL:
        case eDATA_NODE_SLOT1_COLOR_DICOMSIM:
        case eDATA_NODE_SLOT1_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_COLOR_3D:
        case eDATA_NODE_SLOT1_COLOR_BLENDING:
        case eDATA_NODE_SLOT1_COLOR_USER:
        case eDATA_NODE_SLOT1_COLOR_HDR:
        case eDATA_NODE_SLOT1_COLOR_SRGB:
        case eDATA_NODE_SLOT1_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_SUPER_RED:
        case eDATA_NODE_SLOT1_COLOR_3D_PASSIVE:
        case eDATA_NODE_SLOT1_COLOR_HLG:

        case eDATA_NODE_SLOT1_COLOR_USER_PRESENTATION:
        case eDATA_NODE_SLOT1_COLOR_USER_VIDEO:
        case eDATA_NODE_SLOT1_COLOR_USER_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_USER_ENHANCED:
        case eDATA_NODE_SLOT1_COLOR_USER_REC709:
        case eDATA_NODE_SLOT1_COLOR_USER_REAL:
        case eDATA_NODE_SLOT1_COLOR_USER_DICOMSIM:
        case eDATA_NODE_SLOT1_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_COLOR_USER_3D:
        case eDATA_NODE_SLOT1_COLOR_USER_BLENDING:
        case eDATA_NODE_SLOT1_COLOR_USER_USER:
        case eDATA_NODE_SLOT1_COLOR_USER_HDR:
        case eDATA_NODE_SLOT1_COLOR_USER_SRGB:
        case eDATA_NODE_SLOT1_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_USER_SUPER_RED:
        case eDATA_NODE_SLOT1_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT1_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_SLOT1;
            break;

        case eDATA_NODE_SLOT2_COMMON:
        case eDATA_NODE_SLOT2_HSG_PRESENTATION:
        case eDATA_NODE_SLOT2_HSG_VIDEO:
        case eDATA_NODE_SLOT2_HSG_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_ENHANCED:
        case eDATA_NODE_SLOT2_HSG_REC709:
        case eDATA_NODE_SLOT2_HSG_REAL:
        case eDATA_NODE_SLOT2_HSG_DICOMSIM:
        case eDATA_NODE_SLOT2_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_HSG_3D:
        case eDATA_NODE_SLOT2_HSG_BLENDING:
        case eDATA_NODE_SLOT2_HSG_USER:
        case eDATA_NODE_SLOT2_HSG_HDR:
        case eDATA_NODE_SLOT2_HSG_SRGB:
        case eDATA_NODE_SLOT2_HSG_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_SUPER_RED:
        case eDATA_NODE_SLOT2_HSG_3D_PASSIVE:
        case eDATA_NODE_SLOT2_HSG_HLG:

        case eDATA_NODE_SLOT2_HSG_USER_PRESENTATION:
        case eDATA_NODE_SLOT2_HSG_USER_VIDEO:
        case eDATA_NODE_SLOT2_HSG_USER_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_USER_ENHANCED:
        case eDATA_NODE_SLOT2_HSG_USER_REC709:
        case eDATA_NODE_SLOT2_HSG_USER_REAL:
        case eDATA_NODE_SLOT2_HSG_USER_DICOMSIM:
        case eDATA_NODE_SLOT2_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_HSG_USER_3D:
        case eDATA_NODE_SLOT2_HSG_USER_BLENDING:
        case eDATA_NODE_SLOT2_HSG_USER_USER:
        case eDATA_NODE_SLOT2_HSG_USER_HDR:
        case eDATA_NODE_SLOT2_HSG_USER_SRGB:
        case eDATA_NODE_SLOT2_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_USER_SUPER_RED:
        case eDATA_NODE_SLOT2_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT2_HSG_USER_HLG:

        case eDATA_NODE_SLOT2_COLOR_PRESENTATION:
        case eDATA_NODE_SLOT2_COLOR_VIDEO:
        case eDATA_NODE_SLOT2_COLOR_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_ENHANCED:
        case eDATA_NODE_SLOT2_COLOR_REC709:
        case eDATA_NODE_SLOT2_COLOR_REAL:
        case eDATA_NODE_SLOT2_COLOR_DICOMSIM:
        case eDATA_NODE_SLOT2_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_COLOR_3D:
        case eDATA_NODE_SLOT2_COLOR_BLENDING:
        case eDATA_NODE_SLOT2_COLOR_USER:
        case eDATA_NODE_SLOT2_COLOR_HDR:
        case eDATA_NODE_SLOT2_COLOR_SRGB:
        case eDATA_NODE_SLOT2_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_SUPER_RED:
        case eDATA_NODE_SLOT2_COLOR_3D_PASSIVE:
        case eDATA_NODE_SLOT2_COLOR_HLG:

        case eDATA_NODE_SLOT2_COLOR_USER_PRESENTATION:
        case eDATA_NODE_SLOT2_COLOR_USER_VIDEO:
        case eDATA_NODE_SLOT2_COLOR_USER_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_USER_ENHANCED:
        case eDATA_NODE_SLOT2_COLOR_USER_REC709:
        case eDATA_NODE_SLOT2_COLOR_USER_REAL:
        case eDATA_NODE_SLOT2_COLOR_USER_DICOMSIM:
        case eDATA_NODE_SLOT2_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_COLOR_USER_3D:
        case eDATA_NODE_SLOT2_COLOR_USER_BLENDING:
        case eDATA_NODE_SLOT2_COLOR_USER_USER:
        case eDATA_NODE_SLOT2_COLOR_USER_HDR:
        case eDATA_NODE_SLOT2_COLOR_USER_SRGB:
        case eDATA_NODE_SLOT2_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_USER_SUPER_RED:
        case eDATA_NODE_SLOT2_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT2_COLOR_USER_HLG:
            ucSource = eCM_SOURCE_SLOT2;
            break;
    }

    ucGuiSrcID = (UINT8)CM2GUI(edcMAIN_INPUT, ucSource);

    //printf("ucGuiSrcID = %d, uiNode = %d\n", ucGuiSrcID, uiNode);

    if(ucGuiSrcID > eGUI_SOURCE_ID_NUMBER)
    {
        return FALSE;
    }

    return TRUE;
}


BOOL utilDataMgr_DisplayModeSupport(UINT16 uiNode)
{
    UINT8   ucDisplaymode = eCM_PICTURE_SETTINGS_NUMBER;
    UINT8   ucGuiDisplayModeID = eGUI_PICTURE_SETTINGS_NUMBER;

    if(uiNode == eDATA_NODE_NUMBER)
    {
        return FALSE;
    }

    if(uiNode < eDATA_NODE_SOURCE0_COMMON)
    {
        return TRUE;
    }

    if((uiNode >= eDATA_NODE_WAP_CALIBRATION_TAG) && (uiNode < eDATA_NODE_NUMBER)) //HICC2_Steven_0017
    {
        return TRUE;
    }

    switch(uiNode)
    {
        case eDATA_NODE_SOURCE0_COMMON:
        case eDATA_NODE_VGA_COMMON:
        case eDATA_NODE_BNC_COMMON:
        case eDATA_NODE_HDMI1_COMMON:
        case eDATA_NODE_HDMI2_COMMON:
        case eDATA_NODE_DVID_COMMON:
        case eDATA_NODE_DP_COMMON:
        case eDATA_NODE_3GSDI_COMMON:
        case eDATA_NODE_HDBASET_COMMON:
        case eDATA_NODE_12GSDI_COMMON:
        case eDATA_NODE_PRESENTER_COMMON:
        case eDATA_NODE_CARDREADER_COMMON:
        case eDATA_NODE_MINIUSB_COMMON:
        case eDATA_NODE_SLOT1_COMMON:
        case eDATA_NODE_SLOT2_COMMON:
            if(utilDataMgr_SourceSupport(uiNode) == FALSE)
            {
                return FALSE;
            }
            return TRUE;

        case eDATA_NODE_SOURCE0_HSG_PRESENTATION:
        case eDATA_NODE_SOURCE0_HSG_USER_PRESENTATION:
        case eDATA_NODE_SOURCE0_COLOR_PRESENTATION:
        case eDATA_NODE_SOURCE0_COLOR_USER_PRESENTATION:
        case eDATA_NODE_VGA_HSG_PRESENTATION:
        case eDATA_NODE_VGA_HSG_USER_PRESENTATION:
        case eDATA_NODE_VGA_COLOR_PRESENTATION:
        case eDATA_NODE_VGA_COLOR_USER_PRESENTATION:
        case eDATA_NODE_BNC_HSG_PRESENTATION:
        case eDATA_NODE_BNC_HSG_USER_PRESENTATION:
        case eDATA_NODE_BNC_COLOR_PRESENTATION:
        case eDATA_NODE_BNC_COLOR_USER_PRESENTATION:
        case eDATA_NODE_HDMI1_HSG_PRESENTATION:
        case eDATA_NODE_HDMI1_HSG_USER_PRESENTATION:
        case eDATA_NODE_HDMI1_COLOR_PRESENTATION:
        case eDATA_NODE_HDMI1_COLOR_USER_PRESENTATION:
        case eDATA_NODE_HDMI2_HSG_PRESENTATION:
        case eDATA_NODE_HDMI2_HSG_USER_PRESENTATION:
        case eDATA_NODE_HDMI2_COLOR_PRESENTATION:
        case eDATA_NODE_HDMI2_COLOR_USER_PRESENTATION:
        case eDATA_NODE_DVID_HSG_PRESENTATION:
        case eDATA_NODE_DVID_HSG_USER_PRESENTATION:
        case eDATA_NODE_DVID_COLOR_PRESENTATION:
        case eDATA_NODE_DVID_COLOR_USER_PRESENTATION:
        case eDATA_NODE_DP_HSG_PRESENTATION:
        case eDATA_NODE_DP_HSG_USER_PRESENTATION:
        case eDATA_NODE_DP_COLOR_PRESENTATION:
        case eDATA_NODE_DP_COLOR_USER_PRESENTATION:
        case eDATA_NODE_3GSDI_HSG_PRESENTATION:
        case eDATA_NODE_3GSDI_HSG_USER_PRESENTATION:
        case eDATA_NODE_3GSDI_COLOR_PRESENTATION:
        case eDATA_NODE_3GSDI_COLOR_USER_PRESENTATION:
        case eDATA_NODE_HDBASET_HSG_PRESENTATION:
        case eDATA_NODE_HDBASET_HSG_USER_PRESENTATION:
        case eDATA_NODE_HDBASET_COLOR_PRESENTATION:
        case eDATA_NODE_HDBASET_COLOR_USER_PRESENTATION:
        case eDATA_NODE_12GSDI_HSG_PRESENTATION:
        case eDATA_NODE_12GSDI_HSG_USER_PRESENTATION:
        case eDATA_NODE_12GSDI_COLOR_PRESENTATION:
        case eDATA_NODE_12GSDI_COLOR_USER_PRESENTATION:
        case eDATA_NODE_PRESENTER_HSG_PRESENTATION:
        case eDATA_NODE_PRESENTER_HSG_USER_PRESENTATION:
        case eDATA_NODE_PRESENTER_COLOR_PRESENTATION:
        case eDATA_NODE_PRESENTER_COLOR_USER_PRESENTATION:
        case eDATA_NODE_CARDREADER_HSG_PRESENTATION:
        case eDATA_NODE_CARDREADER_HSG_USER_PRESENTATION:
        case eDATA_NODE_CARDREADER_COLOR_PRESENTATION:
        case eDATA_NODE_CARDREADER_COLOR_USER_PRESENTATION:
        case eDATA_NODE_MINIUSB_HSG_PRESENTATION:
        case eDATA_NODE_MINIUSB_HSG_USER_PRESENTATION:
        case eDATA_NODE_MINIUSB_COLOR_PRESENTATION:
        case eDATA_NODE_MINIUSB_COLOR_USER_PRESENTATION:
        case eDATA_NODE_SLOT1_HSG_PRESENTATION:
        case eDATA_NODE_SLOT1_HSG_USER_PRESENTATION:
        case eDATA_NODE_SLOT1_COLOR_PRESENTATION:
        case eDATA_NODE_SLOT1_COLOR_USER_PRESENTATION:
        case eDATA_NODE_SLOT2_HSG_PRESENTATION:
        case eDATA_NODE_SLOT2_HSG_USER_PRESENTATION:
        case eDATA_NODE_SLOT2_COLOR_PRESENTATION:
        case eDATA_NODE_SLOT2_COLOR_USER_PRESENTATION:
            ucDisplaymode = eCM_PICTURE_SETTINGS_PRESENTATION;
            break;

        case eDATA_NODE_SOURCE0_HSG_VIDEO:
        case eDATA_NODE_SOURCE0_HSG_USER_VIDEO:
        case eDATA_NODE_SOURCE0_COLOR_VIDEO:
        case eDATA_NODE_SOURCE0_COLOR_USER_VIDEO:
        case eDATA_NODE_VGA_HSG_VIDEO:
        case eDATA_NODE_VGA_HSG_USER_VIDEO:
        case eDATA_NODE_VGA_COLOR_VIDEO:
        case eDATA_NODE_VGA_COLOR_USER_VIDEO:
        case eDATA_NODE_BNC_HSG_VIDEO:
        case eDATA_NODE_BNC_HSG_USER_VIDEO:
        case eDATA_NODE_BNC_COLOR_VIDEO:
        case eDATA_NODE_BNC_COLOR_USER_VIDEO:
        case eDATA_NODE_HDMI1_HSG_VIDEO:
        case eDATA_NODE_HDMI1_HSG_USER_VIDEO:
        case eDATA_NODE_HDMI1_COLOR_VIDEO:
        case eDATA_NODE_HDMI1_COLOR_USER_VIDEO:
        case eDATA_NODE_HDMI2_HSG_VIDEO:
        case eDATA_NODE_HDMI2_HSG_USER_VIDEO:
        case eDATA_NODE_HDMI2_COLOR_VIDEO:
        case eDATA_NODE_HDMI2_COLOR_USER_VIDEO:
        case eDATA_NODE_DVID_HSG_VIDEO:
        case eDATA_NODE_DVID_HSG_USER_VIDEO:
        case eDATA_NODE_DVID_COLOR_VIDEO:
        case eDATA_NODE_DVID_COLOR_USER_VIDEO:
        case eDATA_NODE_DP_HSG_VIDEO:
        case eDATA_NODE_DP_HSG_USER_VIDEO:
        case eDATA_NODE_DP_COLOR_VIDEO:
        case eDATA_NODE_DP_COLOR_USER_VIDEO:
        case eDATA_NODE_3GSDI_HSG_VIDEO:
        case eDATA_NODE_3GSDI_HSG_USER_VIDEO:
        case eDATA_NODE_3GSDI_COLOR_VIDEO:
        case eDATA_NODE_3GSDI_COLOR_USER_VIDEO:
        case eDATA_NODE_HDBASET_HSG_VIDEO:
        case eDATA_NODE_HDBASET_HSG_USER_VIDEO:
        case eDATA_NODE_HDBASET_COLOR_VIDEO:
        case eDATA_NODE_HDBASET_COLOR_USER_VIDEO:
        case eDATA_NODE_12GSDI_HSG_VIDEO:
        case eDATA_NODE_12GSDI_HSG_USER_VIDEO:
        case eDATA_NODE_12GSDI_COLOR_VIDEO:
        case eDATA_NODE_12GSDI_COLOR_USER_VIDEO:
        case eDATA_NODE_PRESENTER_HSG_VIDEO:
        case eDATA_NODE_PRESENTER_HSG_USER_VIDEO:
        case eDATA_NODE_PRESENTER_COLOR_VIDEO:
        case eDATA_NODE_PRESENTER_COLOR_USER_VIDEO:
        case eDATA_NODE_CARDREADER_HSG_VIDEO:
        case eDATA_NODE_CARDREADER_HSG_USER_VIDEO:
        case eDATA_NODE_CARDREADER_COLOR_VIDEO:
        case eDATA_NODE_CARDREADER_COLOR_USER_VIDEO:
        case eDATA_NODE_MINIUSB_HSG_VIDEO:
        case eDATA_NODE_MINIUSB_HSG_USER_VIDEO:
        case eDATA_NODE_MINIUSB_COLOR_VIDEO:
        case eDATA_NODE_MINIUSB_COLOR_USER_VIDEO:
        case eDATA_NODE_SLOT1_HSG_VIDEO:
        case eDATA_NODE_SLOT1_HSG_USER_VIDEO:
        case eDATA_NODE_SLOT1_COLOR_VIDEO:
        case eDATA_NODE_SLOT1_COLOR_USER_VIDEO:
        case eDATA_NODE_SLOT2_HSG_VIDEO:
        case eDATA_NODE_SLOT2_HSG_USER_VIDEO:
        case eDATA_NODE_SLOT2_COLOR_VIDEO:
        case eDATA_NODE_SLOT2_COLOR_USER_VIDEO:
            ucDisplaymode = eCM_PICTURE_SETTINGS_VIDEO;
            break;

        case eDATA_NODE_SOURCE0_HSG_BRIGHT:
        case eDATA_NODE_SOURCE0_HSG_USER_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_USER_BRIGHT:
        case eDATA_NODE_VGA_HSG_BRIGHT:
        case eDATA_NODE_VGA_HSG_USER_BRIGHT:
        case eDATA_NODE_VGA_COLOR_BRIGHT:
        case eDATA_NODE_VGA_COLOR_USER_BRIGHT:
        case eDATA_NODE_BNC_HSG_BRIGHT:
        case eDATA_NODE_BNC_HSG_USER_BRIGHT:
        case eDATA_NODE_BNC_COLOR_BRIGHT:
        case eDATA_NODE_BNC_COLOR_USER_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_USER_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_USER_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_USER_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_USER_BRIGHT:
        case eDATA_NODE_DVID_HSG_BRIGHT:
        case eDATA_NODE_DVID_HSG_USER_BRIGHT:
        case eDATA_NODE_DVID_COLOR_BRIGHT:
        case eDATA_NODE_DVID_COLOR_USER_BRIGHT:
        case eDATA_NODE_DP_HSG_BRIGHT:
        case eDATA_NODE_DP_HSG_USER_BRIGHT:
        case eDATA_NODE_DP_COLOR_BRIGHT:
        case eDATA_NODE_DP_COLOR_USER_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_USER_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_USER_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_USER_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_USER_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_USER_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_USER_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_USER_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_USER_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_USER_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_USER_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_USER_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_USER_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_USER_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_USER_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_USER_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_USER_BRIGHT:
            ucDisplaymode = eCM_PICTURE_SETTINGS_BRIGHT;
            break;

        case eDATA_NODE_SOURCE0_HSG_ENHANCED:
        case eDATA_NODE_SOURCE0_HSG_USER_ENHANCED:
        case eDATA_NODE_SOURCE0_COLOR_ENHANCED:
        case eDATA_NODE_SOURCE0_COLOR_USER_ENHANCED:
        case eDATA_NODE_VGA_HSG_ENHANCED:
        case eDATA_NODE_VGA_HSG_USER_ENHANCED:
        case eDATA_NODE_VGA_COLOR_ENHANCED:
        case eDATA_NODE_VGA_COLOR_USER_ENHANCED:
        case eDATA_NODE_BNC_HSG_ENHANCED:
        case eDATA_NODE_BNC_HSG_USER_ENHANCED:
        case eDATA_NODE_BNC_COLOR_ENHANCED:
        case eDATA_NODE_BNC_COLOR_USER_ENHANCED:
        case eDATA_NODE_HDMI1_HSG_ENHANCED:
        case eDATA_NODE_HDMI1_HSG_USER_ENHANCED:
        case eDATA_NODE_HDMI1_COLOR_ENHANCED:
        case eDATA_NODE_HDMI1_COLOR_USER_ENHANCED:
        case eDATA_NODE_HDMI2_HSG_ENHANCED:
        case eDATA_NODE_HDMI2_HSG_USER_ENHANCED:
        case eDATA_NODE_HDMI2_COLOR_ENHANCED:
        case eDATA_NODE_HDMI2_COLOR_USER_ENHANCED:
        case eDATA_NODE_DVID_HSG_ENHANCED:
        case eDATA_NODE_DVID_HSG_USER_ENHANCED:
        case eDATA_NODE_DVID_COLOR_ENHANCED:
        case eDATA_NODE_DVID_COLOR_USER_ENHANCED:
        case eDATA_NODE_DP_HSG_ENHANCED:
        case eDATA_NODE_DP_HSG_USER_ENHANCED:
        case eDATA_NODE_DP_COLOR_ENHANCED:
        case eDATA_NODE_DP_COLOR_USER_ENHANCED:
        case eDATA_NODE_3GSDI_HSG_ENHANCED:
        case eDATA_NODE_3GSDI_HSG_USER_ENHANCED:
        case eDATA_NODE_3GSDI_COLOR_ENHANCED:
        case eDATA_NODE_3GSDI_COLOR_USER_ENHANCED:
        case eDATA_NODE_HDBASET_HSG_ENHANCED:
        case eDATA_NODE_HDBASET_HSG_USER_ENHANCED:
        case eDATA_NODE_HDBASET_COLOR_ENHANCED:
        case eDATA_NODE_HDBASET_COLOR_USER_ENHANCED:
        case eDATA_NODE_12GSDI_HSG_ENHANCED:
        case eDATA_NODE_12GSDI_HSG_USER_ENHANCED:
        case eDATA_NODE_12GSDI_COLOR_ENHANCED:
        case eDATA_NODE_12GSDI_COLOR_USER_ENHANCED:
        case eDATA_NODE_PRESENTER_HSG_ENHANCED:
        case eDATA_NODE_PRESENTER_HSG_USER_ENHANCED:
        case eDATA_NODE_PRESENTER_COLOR_ENHANCED:
        case eDATA_NODE_PRESENTER_COLOR_USER_ENHANCED:
        case eDATA_NODE_CARDREADER_HSG_ENHANCED:
        case eDATA_NODE_CARDREADER_HSG_USER_ENHANCED:
        case eDATA_NODE_CARDREADER_COLOR_ENHANCED:
        case eDATA_NODE_CARDREADER_COLOR_USER_ENHANCED:
        case eDATA_NODE_MINIUSB_HSG_ENHANCED:
        case eDATA_NODE_MINIUSB_HSG_USER_ENHANCED:
        case eDATA_NODE_MINIUSB_COLOR_ENHANCED:
        case eDATA_NODE_MINIUSB_COLOR_USER_ENHANCED:
        case eDATA_NODE_SLOT1_HSG_ENHANCED:
        case eDATA_NODE_SLOT1_HSG_USER_ENHANCED:
        case eDATA_NODE_SLOT1_COLOR_ENHANCED:
        case eDATA_NODE_SLOT1_COLOR_USER_ENHANCED:
        case eDATA_NODE_SLOT2_HSG_ENHANCED:
        case eDATA_NODE_SLOT2_HSG_USER_ENHANCED:
        case eDATA_NODE_SLOT2_COLOR_ENHANCED:
        case eDATA_NODE_SLOT2_COLOR_USER_ENHANCED:
            ucDisplaymode = eCM_PICTURE_SETTINGS_ENHANCED;
            break;

        case eDATA_NODE_SOURCE0_HSG_REC709:
        case eDATA_NODE_SOURCE0_HSG_USER_REC709:
        case eDATA_NODE_SOURCE0_COLOR_REC709:
        case eDATA_NODE_SOURCE0_COLOR_USER_REC709:
        case eDATA_NODE_VGA_HSG_REC709:
        case eDATA_NODE_VGA_HSG_USER_REC709:
        case eDATA_NODE_VGA_COLOR_REC709:
        case eDATA_NODE_VGA_COLOR_USER_REC709:
        case eDATA_NODE_BNC_HSG_REC709:
        case eDATA_NODE_BNC_HSG_USER_REC709:
        case eDATA_NODE_BNC_COLOR_REC709:
        case eDATA_NODE_BNC_COLOR_USER_REC709:
        case eDATA_NODE_HDMI1_HSG_REC709:
        case eDATA_NODE_HDMI1_HSG_USER_REC709:
        case eDATA_NODE_HDMI1_COLOR_REC709:
        case eDATA_NODE_HDMI1_COLOR_USER_REC709:
        case eDATA_NODE_HDMI2_HSG_REC709:
        case eDATA_NODE_HDMI2_HSG_USER_REC709:
        case eDATA_NODE_HDMI2_COLOR_REC709:
        case eDATA_NODE_HDMI2_COLOR_USER_REC709:
        case eDATA_NODE_DVID_HSG_REC709:
        case eDATA_NODE_DVID_HSG_USER_REC709:
        case eDATA_NODE_DVID_COLOR_REC709:
        case eDATA_NODE_DVID_COLOR_USER_REC709:
        case eDATA_NODE_DP_HSG_REC709:
        case eDATA_NODE_DP_HSG_USER_REC709:
        case eDATA_NODE_DP_COLOR_REC709:
        case eDATA_NODE_DP_COLOR_USER_REC709:
        case eDATA_NODE_3GSDI_HSG_REC709:
        case eDATA_NODE_3GSDI_HSG_USER_REC709:
        case eDATA_NODE_3GSDI_COLOR_REC709:
        case eDATA_NODE_3GSDI_COLOR_USER_REC709:
        case eDATA_NODE_HDBASET_HSG_REC709:
        case eDATA_NODE_HDBASET_HSG_USER_REC709:
        case eDATA_NODE_HDBASET_COLOR_REC709:
        case eDATA_NODE_HDBASET_COLOR_USER_REC709:
        case eDATA_NODE_12GSDI_HSG_REC709:
        case eDATA_NODE_12GSDI_HSG_USER_REC709:
        case eDATA_NODE_12GSDI_COLOR_REC709:
        case eDATA_NODE_12GSDI_COLOR_USER_REC709:
        case eDATA_NODE_PRESENTER_HSG_REC709:
        case eDATA_NODE_PRESENTER_HSG_USER_REC709:
        case eDATA_NODE_PRESENTER_COLOR_REC709:
        case eDATA_NODE_PRESENTER_COLOR_USER_REC709:
        case eDATA_NODE_CARDREADER_HSG_REC709:
        case eDATA_NODE_CARDREADER_HSG_USER_REC709:
        case eDATA_NODE_CARDREADER_COLOR_REC709:
        case eDATA_NODE_CARDREADER_COLOR_USER_REC709:
        case eDATA_NODE_MINIUSB_HSG_REC709:
        case eDATA_NODE_MINIUSB_HSG_USER_REC709:
        case eDATA_NODE_MINIUSB_COLOR_REC709:
        case eDATA_NODE_MINIUSB_COLOR_USER_REC709:
        case eDATA_NODE_SLOT1_HSG_REC709:
        case eDATA_NODE_SLOT1_HSG_USER_REC709:
        case eDATA_NODE_SLOT1_COLOR_REC709:
        case eDATA_NODE_SLOT1_COLOR_USER_REC709:
        case eDATA_NODE_SLOT2_HSG_REC709:
        case eDATA_NODE_SLOT2_HSG_USER_REC709:
        case eDATA_NODE_SLOT2_COLOR_REC709:
        case eDATA_NODE_SLOT2_COLOR_USER_REC709:
            ucDisplaymode = eCM_PICTURE_SETTINGS_REC709;
            break;

        case eDATA_NODE_SOURCE0_HSG_REAL:
        case eDATA_NODE_SOURCE0_HSG_USER_REAL:
        case eDATA_NODE_SOURCE0_COLOR_REAL:
        case eDATA_NODE_SOURCE0_COLOR_USER_REAL:
        case eDATA_NODE_VGA_HSG_REAL:
        case eDATA_NODE_VGA_HSG_USER_REAL:
        case eDATA_NODE_VGA_COLOR_REAL:
        case eDATA_NODE_VGA_COLOR_USER_REAL:
        case eDATA_NODE_BNC_HSG_REAL:
        case eDATA_NODE_BNC_HSG_USER_REAL:
        case eDATA_NODE_BNC_COLOR_REAL:
        case eDATA_NODE_BNC_COLOR_USER_REAL:
        case eDATA_NODE_HDMI1_HSG_REAL:
        case eDATA_NODE_HDMI1_HSG_USER_REAL:
        case eDATA_NODE_HDMI1_COLOR_REAL:
        case eDATA_NODE_HDMI1_COLOR_USER_REAL:
        case eDATA_NODE_HDMI2_HSG_REAL:
        case eDATA_NODE_HDMI2_HSG_USER_REAL:
        case eDATA_NODE_HDMI2_COLOR_REAL:
        case eDATA_NODE_HDMI2_COLOR_USER_REAL:
        case eDATA_NODE_DVID_HSG_REAL:
        case eDATA_NODE_DVID_HSG_USER_REAL:
        case eDATA_NODE_DVID_COLOR_REAL:
        case eDATA_NODE_DVID_COLOR_USER_REAL:
        case eDATA_NODE_DP_HSG_REAL:
        case eDATA_NODE_DP_HSG_USER_REAL:
        case eDATA_NODE_DP_COLOR_REAL:
        case eDATA_NODE_DP_COLOR_USER_REAL:
        case eDATA_NODE_3GSDI_HSG_REAL:
        case eDATA_NODE_3GSDI_HSG_USER_REAL:
        case eDATA_NODE_3GSDI_COLOR_REAL:
        case eDATA_NODE_3GSDI_COLOR_USER_REAL:
        case eDATA_NODE_HDBASET_HSG_REAL:
        case eDATA_NODE_HDBASET_HSG_USER_REAL:
        case eDATA_NODE_HDBASET_COLOR_REAL:
        case eDATA_NODE_HDBASET_COLOR_USER_REAL:
        case eDATA_NODE_12GSDI_HSG_REAL:
        case eDATA_NODE_12GSDI_HSG_USER_REAL:
        case eDATA_NODE_12GSDI_COLOR_REAL:
        case eDATA_NODE_12GSDI_COLOR_USER_REAL:
        case eDATA_NODE_PRESENTER_HSG_REAL:
        case eDATA_NODE_PRESENTER_HSG_USER_REAL:
        case eDATA_NODE_PRESENTER_COLOR_REAL:
        case eDATA_NODE_PRESENTER_COLOR_USER_REAL:
        case eDATA_NODE_CARDREADER_HSG_REAL:
        case eDATA_NODE_CARDREADER_HSG_USER_REAL:
        case eDATA_NODE_CARDREADER_COLOR_REAL:
        case eDATA_NODE_CARDREADER_COLOR_USER_REAL:
        case eDATA_NODE_MINIUSB_HSG_REAL:
        case eDATA_NODE_MINIUSB_HSG_USER_REAL:
        case eDATA_NODE_MINIUSB_COLOR_REAL:
        case eDATA_NODE_MINIUSB_COLOR_USER_REAL:
        case eDATA_NODE_SLOT1_HSG_REAL:
        case eDATA_NODE_SLOT1_HSG_USER_REAL:
        case eDATA_NODE_SLOT1_COLOR_REAL:
        case eDATA_NODE_SLOT1_COLOR_USER_REAL:
        case eDATA_NODE_SLOT2_HSG_REAL:
        case eDATA_NODE_SLOT2_HSG_USER_REAL:
        case eDATA_NODE_SLOT2_COLOR_REAL:
        case eDATA_NODE_SLOT2_COLOR_USER_REAL:
            ucDisplaymode = eCM_PICTURE_SETTINGS_REAL;
            break;

        case eDATA_NODE_SOURCE0_HSG_DICOMSIM:
        case eDATA_NODE_SOURCE0_HSG_USER_DICOMSIM:
        case eDATA_NODE_SOURCE0_COLOR_DICOMSIM:
        case eDATA_NODE_SOURCE0_COLOR_USER_DICOMSIM:
        case eDATA_NODE_VGA_HSG_DICOMSIM:
        case eDATA_NODE_VGA_HSG_USER_DICOMSIM:
        case eDATA_NODE_VGA_COLOR_DICOMSIM:
        case eDATA_NODE_VGA_COLOR_USER_DICOMSIM:
        case eDATA_NODE_BNC_HSG_DICOMSIM:
        case eDATA_NODE_BNC_HSG_USER_DICOMSIM:
        case eDATA_NODE_BNC_COLOR_DICOMSIM:
        case eDATA_NODE_BNC_COLOR_USER_DICOMSIM:
        case eDATA_NODE_HDMI1_HSG_DICOMSIM:
        case eDATA_NODE_HDMI1_HSG_USER_DICOMSIM:
        case eDATA_NODE_HDMI1_COLOR_DICOMSIM:
        case eDATA_NODE_HDMI1_COLOR_USER_DICOMSIM:
        case eDATA_NODE_HDMI2_HSG_DICOMSIM:
        case eDATA_NODE_HDMI2_HSG_USER_DICOMSIM:
        case eDATA_NODE_HDMI2_COLOR_DICOMSIM:
        case eDATA_NODE_HDMI2_COLOR_USER_DICOMSIM:
        case eDATA_NODE_DVID_HSG_DICOMSIM:
        case eDATA_NODE_DVID_HSG_USER_DICOMSIM:
        case eDATA_NODE_DVID_COLOR_DICOMSIM:
        case eDATA_NODE_DVID_COLOR_USER_DICOMSIM:
        case eDATA_NODE_DP_HSG_DICOMSIM:
        case eDATA_NODE_DP_HSG_USER_DICOMSIM:
        case eDATA_NODE_DP_COLOR_DICOMSIM:
        case eDATA_NODE_DP_COLOR_USER_DICOMSIM:
        case eDATA_NODE_3GSDI_HSG_DICOMSIM:
        case eDATA_NODE_3GSDI_HSG_USER_DICOMSIM:
        case eDATA_NODE_3GSDI_COLOR_DICOMSIM:
        case eDATA_NODE_3GSDI_COLOR_USER_DICOMSIM:
        case eDATA_NODE_HDBASET_HSG_DICOMSIM:
        case eDATA_NODE_HDBASET_HSG_USER_DICOMSIM:
        case eDATA_NODE_HDBASET_COLOR_DICOMSIM:
        case eDATA_NODE_HDBASET_COLOR_USER_DICOMSIM:
        case eDATA_NODE_12GSDI_HSG_DICOMSIM:
        case eDATA_NODE_12GSDI_HSG_USER_DICOMSIM:
        case eDATA_NODE_12GSDI_COLOR_DICOMSIM:
        case eDATA_NODE_12GSDI_COLOR_USER_DICOMSIM:
        case eDATA_NODE_PRESENTER_HSG_DICOMSIM:
        case eDATA_NODE_PRESENTER_HSG_USER_DICOMSIM:
        case eDATA_NODE_PRESENTER_COLOR_DICOMSIM:
        case eDATA_NODE_PRESENTER_COLOR_USER_DICOMSIM:
        case eDATA_NODE_CARDREADER_HSG_DICOMSIM:
        case eDATA_NODE_CARDREADER_HSG_USER_DICOMSIM:
        case eDATA_NODE_CARDREADER_COLOR_DICOMSIM:
        case eDATA_NODE_CARDREADER_COLOR_USER_DICOMSIM:
        case eDATA_NODE_MINIUSB_HSG_DICOMSIM:
        case eDATA_NODE_MINIUSB_HSG_USER_DICOMSIM:
        case eDATA_NODE_MINIUSB_COLOR_DICOMSIM:
        case eDATA_NODE_MINIUSB_COLOR_USER_DICOMSIM:
        case eDATA_NODE_SLOT1_HSG_DICOMSIM:
        case eDATA_NODE_SLOT1_HSG_USER_DICOMSIM:
        case eDATA_NODE_SLOT1_COLOR_DICOMSIM:
        case eDATA_NODE_SLOT1_COLOR_USER_DICOMSIM:
        case eDATA_NODE_SLOT2_HSG_DICOMSIM:
        case eDATA_NODE_SLOT2_HSG_USER_DICOMSIM:
        case eDATA_NODE_SLOT2_COLOR_DICOMSIM:
        case eDATA_NODE_SLOT2_COLOR_USER_DICOMSIM:
            ucDisplaymode = eCM_PICTURE_SETTINGS_DICOMSIM;
            break;

        case eDATA_NODE_SOURCE0_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_SOURCE0_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SOURCE0_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_SOURCE0_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_VGA_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_BNC_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI1_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_HDMI2_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_DVID_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DP_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_DP_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_DP_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_DP_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_3GSDI_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_HDBASET_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_12GSDI_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_PRESENTER_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_CARDREADER_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_MINIUSB_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT1_COLOR_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_HSG_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_HSG_USER_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_COLOR_2D_HIGH_SPEED:
        case eDATA_NODE_SLOT2_COLOR_USER_2D_HIGH_SPEED:
            ucDisplaymode = eCM_PICTURE_SETTINGS_2DHIGHSPEED;
            break;

        case eDATA_NODE_SOURCE0_HSG_3D:
        case eDATA_NODE_SOURCE0_HSG_USER_3D:
        case eDATA_NODE_SOURCE0_COLOR_3D:
        case eDATA_NODE_SOURCE0_COLOR_USER_3D:
        case eDATA_NODE_VGA_HSG_3D:
        case eDATA_NODE_VGA_HSG_USER_3D:
        case eDATA_NODE_VGA_COLOR_3D:
        case eDATA_NODE_VGA_COLOR_USER_3D:
        case eDATA_NODE_BNC_HSG_3D:
        case eDATA_NODE_BNC_HSG_USER_3D:
        case eDATA_NODE_BNC_COLOR_3D:
        case eDATA_NODE_BNC_COLOR_USER_3D:
        case eDATA_NODE_HDMI1_HSG_3D:
        case eDATA_NODE_HDMI1_HSG_USER_3D:
        case eDATA_NODE_HDMI1_COLOR_3D:
        case eDATA_NODE_HDMI1_COLOR_USER_3D:
        case eDATA_NODE_HDMI2_HSG_3D:
        case eDATA_NODE_HDMI2_HSG_USER_3D:
        case eDATA_NODE_HDMI2_COLOR_3D:
        case eDATA_NODE_HDMI2_COLOR_USER_3D:
        case eDATA_NODE_DVID_HSG_3D:
        case eDATA_NODE_DVID_HSG_USER_3D:
        case eDATA_NODE_DVID_COLOR_3D:
        case eDATA_NODE_DVID_COLOR_USER_3D:
        case eDATA_NODE_DP_HSG_3D:
        case eDATA_NODE_DP_HSG_USER_3D:
        case eDATA_NODE_DP_COLOR_3D:
        case eDATA_NODE_DP_COLOR_USER_3D:
        case eDATA_NODE_3GSDI_HSG_3D:
        case eDATA_NODE_3GSDI_HSG_USER_3D:
        case eDATA_NODE_3GSDI_COLOR_3D:
        case eDATA_NODE_3GSDI_COLOR_USER_3D:
        case eDATA_NODE_HDBASET_HSG_3D:
        case eDATA_NODE_HDBASET_HSG_USER_3D:
        case eDATA_NODE_HDBASET_COLOR_3D:
        case eDATA_NODE_HDBASET_COLOR_USER_3D:
        case eDATA_NODE_12GSDI_HSG_3D:
        case eDATA_NODE_12GSDI_HSG_USER_3D:
        case eDATA_NODE_12GSDI_COLOR_3D:
        case eDATA_NODE_12GSDI_COLOR_USER_3D:
        case eDATA_NODE_PRESENTER_HSG_3D:
        case eDATA_NODE_PRESENTER_HSG_USER_3D:
        case eDATA_NODE_PRESENTER_COLOR_3D:
        case eDATA_NODE_PRESENTER_COLOR_USER_3D:
        case eDATA_NODE_CARDREADER_HSG_3D:
        case eDATA_NODE_CARDREADER_HSG_USER_3D:
        case eDATA_NODE_CARDREADER_COLOR_3D:
        case eDATA_NODE_CARDREADER_COLOR_USER_3D:
        case eDATA_NODE_MINIUSB_HSG_3D:
        case eDATA_NODE_MINIUSB_HSG_USER_3D:
        case eDATA_NODE_MINIUSB_COLOR_3D:
        case eDATA_NODE_MINIUSB_COLOR_USER_3D:
        case eDATA_NODE_SLOT1_HSG_3D:
        case eDATA_NODE_SLOT1_HSG_USER_3D:
        case eDATA_NODE_SLOT1_COLOR_3D:
        case eDATA_NODE_SLOT1_COLOR_USER_3D:
        case eDATA_NODE_SLOT2_HSG_3D:
        case eDATA_NODE_SLOT2_HSG_USER_3D:
        case eDATA_NODE_SLOT2_COLOR_3D:
        case eDATA_NODE_SLOT2_COLOR_USER_3D:
            ucDisplaymode = eCM_PICTURE_SETTINGS_3D;
            break;

        case eDATA_NODE_SOURCE0_HSG_BLENDING:
        case eDATA_NODE_SOURCE0_HSG_USER_BLENDING:
        case eDATA_NODE_SOURCE0_COLOR_BLENDING:
        case eDATA_NODE_SOURCE0_COLOR_USER_BLENDING:
        case eDATA_NODE_VGA_HSG_BLENDING:
        case eDATA_NODE_VGA_HSG_USER_BLENDING:
        case eDATA_NODE_VGA_COLOR_BLENDING:
        case eDATA_NODE_VGA_COLOR_USER_BLENDING:
        case eDATA_NODE_BNC_HSG_BLENDING:
        case eDATA_NODE_BNC_HSG_USER_BLENDING:
        case eDATA_NODE_BNC_COLOR_BLENDING:
        case eDATA_NODE_BNC_COLOR_USER_BLENDING:
        case eDATA_NODE_HDMI1_HSG_BLENDING:
        case eDATA_NODE_HDMI1_HSG_USER_BLENDING:
        case eDATA_NODE_HDMI1_COLOR_BLENDING:
        case eDATA_NODE_HDMI1_COLOR_USER_BLENDING:
        case eDATA_NODE_HDMI2_HSG_BLENDING:
        case eDATA_NODE_HDMI2_HSG_USER_BLENDING:
        case eDATA_NODE_HDMI2_COLOR_BLENDING:
        case eDATA_NODE_HDMI2_COLOR_USER_BLENDING:
        case eDATA_NODE_DVID_HSG_BLENDING:
        case eDATA_NODE_DVID_HSG_USER_BLENDING:
        case eDATA_NODE_DVID_COLOR_BLENDING:
        case eDATA_NODE_DVID_COLOR_USER_BLENDING:
        case eDATA_NODE_DP_HSG_BLENDING:
        case eDATA_NODE_DP_HSG_USER_BLENDING:
        case eDATA_NODE_DP_COLOR_BLENDING:
        case eDATA_NODE_DP_COLOR_USER_BLENDING:
        case eDATA_NODE_3GSDI_HSG_BLENDING:
        case eDATA_NODE_3GSDI_HSG_USER_BLENDING:
        case eDATA_NODE_3GSDI_COLOR_BLENDING:
        case eDATA_NODE_3GSDI_COLOR_USER_BLENDING:
        case eDATA_NODE_HDBASET_HSG_BLENDING:
        case eDATA_NODE_HDBASET_HSG_USER_BLENDING:
        case eDATA_NODE_HDBASET_COLOR_BLENDING:
        case eDATA_NODE_HDBASET_COLOR_USER_BLENDING:
        case eDATA_NODE_12GSDI_HSG_BLENDING:
        case eDATA_NODE_12GSDI_HSG_USER_BLENDING:
        case eDATA_NODE_12GSDI_COLOR_BLENDING:
        case eDATA_NODE_12GSDI_COLOR_USER_BLENDING:
        case eDATA_NODE_PRESENTER_HSG_BLENDING:
        case eDATA_NODE_PRESENTER_HSG_USER_BLENDING:
        case eDATA_NODE_PRESENTER_COLOR_BLENDING:
        case eDATA_NODE_PRESENTER_COLOR_USER_BLENDING:
        case eDATA_NODE_CARDREADER_HSG_BLENDING:
        case eDATA_NODE_CARDREADER_HSG_USER_BLENDING:
        case eDATA_NODE_CARDREADER_COLOR_BLENDING:
        case eDATA_NODE_CARDREADER_COLOR_USER_BLENDING:
        case eDATA_NODE_MINIUSB_HSG_BLENDING:
        case eDATA_NODE_MINIUSB_HSG_USER_BLENDING:
        case eDATA_NODE_MINIUSB_COLOR_BLENDING:
        case eDATA_NODE_MINIUSB_COLOR_USER_BLENDING:
        case eDATA_NODE_SLOT1_HSG_BLENDING:
        case eDATA_NODE_SLOT1_HSG_USER_BLENDING:
        case eDATA_NODE_SLOT1_COLOR_BLENDING:
        case eDATA_NODE_SLOT1_COLOR_USER_BLENDING:
        case eDATA_NODE_SLOT2_HSG_BLENDING:
        case eDATA_NODE_SLOT2_HSG_USER_BLENDING:
        case eDATA_NODE_SLOT2_COLOR_BLENDING:
        case eDATA_NODE_SLOT2_COLOR_USER_BLENDING:
            ucDisplaymode = eCM_PICTURE_SETTINGS_BLENDING;
            break;

        case eDATA_NODE_SOURCE0_HSG_USER:
        case eDATA_NODE_SOURCE0_HSG_USER_USER:
        case eDATA_NODE_SOURCE0_COLOR_USER:
        case eDATA_NODE_SOURCE0_COLOR_USER_USER:
        case eDATA_NODE_VGA_HSG_USER:
        case eDATA_NODE_VGA_HSG_USER_USER:
        case eDATA_NODE_VGA_COLOR_USER:
        case eDATA_NODE_VGA_COLOR_USER_USER:
        case eDATA_NODE_BNC_HSG_USER:
        case eDATA_NODE_BNC_HSG_USER_USER:
        case eDATA_NODE_BNC_COLOR_USER:
        case eDATA_NODE_BNC_COLOR_USER_USER:
        case eDATA_NODE_HDMI1_HSG_USER:
        case eDATA_NODE_HDMI1_HSG_USER_USER:
        case eDATA_NODE_HDMI1_COLOR_USER:
        case eDATA_NODE_HDMI1_COLOR_USER_USER:
        case eDATA_NODE_HDMI2_HSG_USER:
        case eDATA_NODE_HDMI2_HSG_USER_USER:
        case eDATA_NODE_HDMI2_COLOR_USER:
        case eDATA_NODE_HDMI2_COLOR_USER_USER:
        case eDATA_NODE_DVID_HSG_USER:
        case eDATA_NODE_DVID_HSG_USER_USER:
        case eDATA_NODE_DVID_COLOR_USER:
        case eDATA_NODE_DVID_COLOR_USER_USER:
        case eDATA_NODE_DP_HSG_USER:
        case eDATA_NODE_DP_HSG_USER_USER:
        case eDATA_NODE_DP_COLOR_USER:
        case eDATA_NODE_DP_COLOR_USER_USER:
        case eDATA_NODE_3GSDI_HSG_USER:
        case eDATA_NODE_3GSDI_HSG_USER_USER:
        case eDATA_NODE_3GSDI_COLOR_USER:
        case eDATA_NODE_3GSDI_COLOR_USER_USER:
        case eDATA_NODE_HDBASET_HSG_USER:
        case eDATA_NODE_HDBASET_HSG_USER_USER:
        case eDATA_NODE_HDBASET_COLOR_USER:
        case eDATA_NODE_HDBASET_COLOR_USER_USER:
        case eDATA_NODE_12GSDI_HSG_USER:
        case eDATA_NODE_12GSDI_HSG_USER_USER:
        case eDATA_NODE_12GSDI_COLOR_USER:
        case eDATA_NODE_12GSDI_COLOR_USER_USER:
        case eDATA_NODE_PRESENTER_HSG_USER:
        case eDATA_NODE_PRESENTER_HSG_USER_USER:
        case eDATA_NODE_PRESENTER_COLOR_USER:
        case eDATA_NODE_PRESENTER_COLOR_USER_USER:
        case eDATA_NODE_CARDREADER_HSG_USER:
        case eDATA_NODE_CARDREADER_HSG_USER_USER:
        case eDATA_NODE_CARDREADER_COLOR_USER:
        case eDATA_NODE_CARDREADER_COLOR_USER_USER:
        case eDATA_NODE_MINIUSB_HSG_USER:
        case eDATA_NODE_MINIUSB_HSG_USER_USER:
        case eDATA_NODE_MINIUSB_COLOR_USER:
        case eDATA_NODE_MINIUSB_COLOR_USER_USER:
        case eDATA_NODE_SLOT1_HSG_USER:
        case eDATA_NODE_SLOT1_HSG_USER_USER:
        case eDATA_NODE_SLOT1_COLOR_USER:
        case eDATA_NODE_SLOT1_COLOR_USER_USER:
        case eDATA_NODE_SLOT2_HSG_USER:
        case eDATA_NODE_SLOT2_HSG_USER_USER:
        case eDATA_NODE_SLOT2_COLOR_USER:
        case eDATA_NODE_SLOT2_COLOR_USER_USER:
            ucDisplaymode = eCM_PICTURE_SETTINGS_USER;
            break;

        case eDATA_NODE_SOURCE0_HSG_HDR:
        case eDATA_NODE_SOURCE0_HSG_USER_HDR:
        case eDATA_NODE_SOURCE0_COLOR_HDR:
        case eDATA_NODE_SOURCE0_COLOR_USER_HDR:
        case eDATA_NODE_VGA_HSG_HDR:
        case eDATA_NODE_VGA_HSG_USER_HDR:
        case eDATA_NODE_VGA_COLOR_HDR:
        case eDATA_NODE_VGA_COLOR_USER_HDR:
        case eDATA_NODE_BNC_HSG_HDR:
        case eDATA_NODE_BNC_HSG_USER_HDR:
        case eDATA_NODE_BNC_COLOR_HDR:
        case eDATA_NODE_BNC_COLOR_USER_HDR:
        case eDATA_NODE_HDMI1_HSG_HDR:
        case eDATA_NODE_HDMI1_HSG_USER_HDR:
        case eDATA_NODE_HDMI1_COLOR_HDR:
        case eDATA_NODE_HDMI1_COLOR_USER_HDR:
        case eDATA_NODE_HDMI2_HSG_HDR:
        case eDATA_NODE_HDMI2_HSG_USER_HDR:
        case eDATA_NODE_HDMI2_COLOR_HDR:
        case eDATA_NODE_HDMI2_COLOR_USER_HDR:
        case eDATA_NODE_DVID_HSG_HDR:
        case eDATA_NODE_DVID_HSG_USER_HDR:
        case eDATA_NODE_DVID_COLOR_HDR:
        case eDATA_NODE_DVID_COLOR_USER_HDR:
        case eDATA_NODE_DP_HSG_HDR:
        case eDATA_NODE_DP_HSG_USER_HDR:
        case eDATA_NODE_DP_COLOR_HDR:
        case eDATA_NODE_DP_COLOR_USER_HDR:
        case eDATA_NODE_3GSDI_HSG_HDR:
        case eDATA_NODE_3GSDI_HSG_USER_HDR:
        case eDATA_NODE_3GSDI_COLOR_HDR:
        case eDATA_NODE_3GSDI_COLOR_USER_HDR:
        case eDATA_NODE_HDBASET_HSG_HDR:
        case eDATA_NODE_HDBASET_HSG_USER_HDR:
        case eDATA_NODE_HDBASET_COLOR_HDR:
        case eDATA_NODE_HDBASET_COLOR_USER_HDR:
        case eDATA_NODE_12GSDI_HSG_HDR:
        case eDATA_NODE_12GSDI_HSG_USER_HDR:
        case eDATA_NODE_12GSDI_COLOR_HDR:
        case eDATA_NODE_12GSDI_COLOR_USER_HDR:
        case eDATA_NODE_PRESENTER_HSG_HDR:
        case eDATA_NODE_PRESENTER_HSG_USER_HDR:
        case eDATA_NODE_PRESENTER_COLOR_HDR:
        case eDATA_NODE_PRESENTER_COLOR_USER_HDR:
        case eDATA_NODE_CARDREADER_HSG_HDR:
        case eDATA_NODE_CARDREADER_HSG_USER_HDR:
        case eDATA_NODE_CARDREADER_COLOR_HDR:
        case eDATA_NODE_CARDREADER_COLOR_USER_HDR:
        case eDATA_NODE_MINIUSB_HSG_HDR:
        case eDATA_NODE_MINIUSB_HSG_USER_HDR:
        case eDATA_NODE_MINIUSB_COLOR_HDR:
        case eDATA_NODE_MINIUSB_COLOR_USER_HDR:
        case eDATA_NODE_SLOT1_HSG_HDR:
        case eDATA_NODE_SLOT1_HSG_USER_HDR:
        case eDATA_NODE_SLOT1_COLOR_HDR:
        case eDATA_NODE_SLOT1_COLOR_USER_HDR:
        case eDATA_NODE_SLOT2_HSG_HDR:
        case eDATA_NODE_SLOT2_HSG_USER_HDR:
        case eDATA_NODE_SLOT2_COLOR_HDR:
        case eDATA_NODE_SLOT2_COLOR_USER_HDR:
            ucDisplaymode = eCM_PICTURE_SETTINGS_HDR;
            break;

        case eDATA_NODE_SOURCE0_HSG_SRGB:
        case eDATA_NODE_SOURCE0_HSG_USER_SRGB:
        case eDATA_NODE_SOURCE0_COLOR_SRGB:
        case eDATA_NODE_SOURCE0_COLOR_USER_SRGB:
        case eDATA_NODE_VGA_HSG_SRGB:
        case eDATA_NODE_VGA_HSG_USER_SRGB:
        case eDATA_NODE_VGA_COLOR_SRGB:
        case eDATA_NODE_VGA_COLOR_USER_SRGB:
        case eDATA_NODE_BNC_HSG_SRGB:
        case eDATA_NODE_BNC_HSG_USER_SRGB:
        case eDATA_NODE_BNC_COLOR_SRGB:
        case eDATA_NODE_BNC_COLOR_USER_SRGB:
        case eDATA_NODE_HDMI1_HSG_SRGB:
        case eDATA_NODE_HDMI1_HSG_USER_SRGB:
        case eDATA_NODE_HDMI1_COLOR_SRGB:
        case eDATA_NODE_HDMI1_COLOR_USER_SRGB:
        case eDATA_NODE_HDMI2_HSG_SRGB:
        case eDATA_NODE_HDMI2_HSG_USER_SRGB:
        case eDATA_NODE_HDMI2_COLOR_SRGB:
        case eDATA_NODE_HDMI2_COLOR_USER_SRGB:
        case eDATA_NODE_DVID_HSG_SRGB:
        case eDATA_NODE_DVID_HSG_USER_SRGB:
        case eDATA_NODE_DVID_COLOR_SRGB:
        case eDATA_NODE_DVID_COLOR_USER_SRGB:
        case eDATA_NODE_DP_HSG_SRGB:
        case eDATA_NODE_DP_HSG_USER_SRGB:
        case eDATA_NODE_DP_COLOR_SRGB:
        case eDATA_NODE_DP_COLOR_USER_SRGB:
        case eDATA_NODE_3GSDI_HSG_SRGB:
        case eDATA_NODE_3GSDI_HSG_USER_SRGB:
        case eDATA_NODE_3GSDI_COLOR_SRGB:
        case eDATA_NODE_3GSDI_COLOR_USER_SRGB:
        case eDATA_NODE_HDBASET_HSG_SRGB:
        case eDATA_NODE_HDBASET_HSG_USER_SRGB:
        case eDATA_NODE_HDBASET_COLOR_SRGB:
        case eDATA_NODE_HDBASET_COLOR_USER_SRGB:
        case eDATA_NODE_12GSDI_HSG_SRGB:
        case eDATA_NODE_12GSDI_HSG_USER_SRGB:
        case eDATA_NODE_12GSDI_COLOR_SRGB:
        case eDATA_NODE_12GSDI_COLOR_USER_SRGB:
        case eDATA_NODE_PRESENTER_HSG_SRGB:
        case eDATA_NODE_PRESENTER_HSG_USER_SRGB:
        case eDATA_NODE_PRESENTER_COLOR_SRGB:
        case eDATA_NODE_PRESENTER_COLOR_USER_SRGB:
        case eDATA_NODE_CARDREADER_HSG_SRGB:
        case eDATA_NODE_CARDREADER_HSG_USER_SRGB:
        case eDATA_NODE_CARDREADER_COLOR_SRGB:
        case eDATA_NODE_CARDREADER_COLOR_USER_SRGB:
        case eDATA_NODE_MINIUSB_HSG_SRGB:
        case eDATA_NODE_MINIUSB_HSG_USER_SRGB:
        case eDATA_NODE_MINIUSB_COLOR_SRGB:
        case eDATA_NODE_MINIUSB_COLOR_USER_SRGB:
        case eDATA_NODE_SLOT1_HSG_SRGB:
        case eDATA_NODE_SLOT1_HSG_USER_SRGB:
        case eDATA_NODE_SLOT1_COLOR_SRGB:
        case eDATA_NODE_SLOT1_COLOR_USER_SRGB:
        case eDATA_NODE_SLOT2_HSG_SRGB:
        case eDATA_NODE_SLOT2_HSG_USER_SRGB:
        case eDATA_NODE_SLOT2_COLOR_SRGB:
        case eDATA_NODE_SLOT2_COLOR_USER_SRGB:
            ucDisplaymode = eCM_PICTURE_SETTINGS_SRGB;
            break;

        case eDATA_NODE_SOURCE0_HSG_SUPER_BRIGHT:
        case eDATA_NODE_SOURCE0_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_SOURCE0_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_VGA_HSG_SUPER_BRIGHT:
        case eDATA_NODE_VGA_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_VGA_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_VGA_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_BNC_HSG_SUPER_BRIGHT:
        case eDATA_NODE_BNC_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_BNC_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_BNC_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_HDMI1_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_HDMI2_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_DVID_HSG_SUPER_BRIGHT:
        case eDATA_NODE_DVID_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_DVID_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_DVID_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_DP_HSG_SUPER_BRIGHT:
        case eDATA_NODE_DP_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_DP_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_DP_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_3GSDI_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_HDBASET_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_12GSDI_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_PRESENTER_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_CARDREADER_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_MINIUSB_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_SLOT1_COLOR_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_HSG_USER_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_SUPER_BRIGHT:
        case eDATA_NODE_SLOT2_COLOR_USER_SUPER_BRIGHT:
            ucDisplaymode = eCM_PICTURE_SETTINGS_SUPER_BRIGHT;
            break;

        case eDATA_NODE_SOURCE0_HSG_SUPER_RED:
        case eDATA_NODE_SOURCE0_HSG_USER_SUPER_RED:
        case eDATA_NODE_SOURCE0_COLOR_SUPER_RED:
        case eDATA_NODE_SOURCE0_COLOR_USER_SUPER_RED:
        case eDATA_NODE_VGA_HSG_SUPER_RED:
        case eDATA_NODE_VGA_HSG_USER_SUPER_RED:
        case eDATA_NODE_VGA_COLOR_SUPER_RED:
        case eDATA_NODE_VGA_COLOR_USER_SUPER_RED:
        case eDATA_NODE_BNC_HSG_SUPER_RED:
        case eDATA_NODE_BNC_HSG_USER_SUPER_RED:
        case eDATA_NODE_BNC_COLOR_SUPER_RED:
        case eDATA_NODE_BNC_COLOR_USER_SUPER_RED:
        case eDATA_NODE_HDMI1_HSG_SUPER_RED:
        case eDATA_NODE_HDMI1_HSG_USER_SUPER_RED:
        case eDATA_NODE_HDMI1_COLOR_SUPER_RED:
        case eDATA_NODE_HDMI1_COLOR_USER_SUPER_RED:
        case eDATA_NODE_HDMI2_HSG_SUPER_RED:
        case eDATA_NODE_HDMI2_HSG_USER_SUPER_RED:
        case eDATA_NODE_HDMI2_COLOR_SUPER_RED:
        case eDATA_NODE_HDMI2_COLOR_USER_SUPER_RED:
        case eDATA_NODE_DVID_HSG_SUPER_RED:
        case eDATA_NODE_DVID_HSG_USER_SUPER_RED:
        case eDATA_NODE_DVID_COLOR_SUPER_RED:
        case eDATA_NODE_DVID_COLOR_USER_SUPER_RED:
        case eDATA_NODE_DP_HSG_SUPER_RED:
        case eDATA_NODE_DP_HSG_USER_SUPER_RED:
        case eDATA_NODE_DP_COLOR_SUPER_RED:
        case eDATA_NODE_DP_COLOR_USER_SUPER_RED:
        case eDATA_NODE_3GSDI_HSG_SUPER_RED:
        case eDATA_NODE_3GSDI_HSG_USER_SUPER_RED:
        case eDATA_NODE_3GSDI_COLOR_SUPER_RED:
        case eDATA_NODE_3GSDI_COLOR_USER_SUPER_RED:
        case eDATA_NODE_HDBASET_HSG_SUPER_RED:
        case eDATA_NODE_HDBASET_HSG_USER_SUPER_RED:
        case eDATA_NODE_HDBASET_COLOR_SUPER_RED:
        case eDATA_NODE_HDBASET_COLOR_USER_SUPER_RED:
        case eDATA_NODE_12GSDI_HSG_SUPER_RED:
        case eDATA_NODE_12GSDI_HSG_USER_SUPER_RED:
        case eDATA_NODE_12GSDI_COLOR_SUPER_RED:
        case eDATA_NODE_12GSDI_COLOR_USER_SUPER_RED:
        case eDATA_NODE_PRESENTER_HSG_SUPER_RED:
        case eDATA_NODE_PRESENTER_HSG_USER_SUPER_RED:
        case eDATA_NODE_PRESENTER_COLOR_SUPER_RED:
        case eDATA_NODE_PRESENTER_COLOR_USER_SUPER_RED:
        case eDATA_NODE_CARDREADER_HSG_SUPER_RED:
        case eDATA_NODE_CARDREADER_HSG_USER_SUPER_RED:
        case eDATA_NODE_CARDREADER_COLOR_SUPER_RED:
        case eDATA_NODE_CARDREADER_COLOR_USER_SUPER_RED:
        case eDATA_NODE_MINIUSB_HSG_SUPER_RED:
        case eDATA_NODE_MINIUSB_HSG_USER_SUPER_RED:
        case eDATA_NODE_MINIUSB_COLOR_SUPER_RED:
        case eDATA_NODE_MINIUSB_COLOR_USER_SUPER_RED:
        case eDATA_NODE_SLOT1_HSG_SUPER_RED:
        case eDATA_NODE_SLOT1_HSG_USER_SUPER_RED:
        case eDATA_NODE_SLOT1_COLOR_SUPER_RED:
        case eDATA_NODE_SLOT1_COLOR_USER_SUPER_RED:
        case eDATA_NODE_SLOT2_HSG_SUPER_RED:
        case eDATA_NODE_SLOT2_HSG_USER_SUPER_RED:
        case eDATA_NODE_SLOT2_COLOR_SUPER_RED:
        case eDATA_NODE_SLOT2_COLOR_USER_SUPER_RED:
            ucDisplaymode = eCM_PICTURE_SETTINGS_SUPER_RED;
            break;

        case eDATA_NODE_SOURCE0_HSG_3D_PASSIVE:
        case eDATA_NODE_SOURCE0_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_SOURCE0_COLOR_3D_PASSIVE:
        case eDATA_NODE_SOURCE0_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_VGA_HSG_3D_PASSIVE:
        case eDATA_NODE_VGA_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_VGA_COLOR_3D_PASSIVE:
        case eDATA_NODE_VGA_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_BNC_HSG_3D_PASSIVE:
        case eDATA_NODE_BNC_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_BNC_COLOR_3D_PASSIVE:
        case eDATA_NODE_BNC_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI1_HSG_3D_PASSIVE:
        case eDATA_NODE_HDMI1_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI1_COLOR_3D_PASSIVE:
        case eDATA_NODE_HDMI1_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI2_HSG_3D_PASSIVE:
        case eDATA_NODE_HDMI2_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_HDMI2_COLOR_3D_PASSIVE:
        case eDATA_NODE_HDMI2_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_DVID_HSG_3D_PASSIVE:
        case eDATA_NODE_DVID_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_DVID_COLOR_3D_PASSIVE:
        case eDATA_NODE_DVID_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_DP_HSG_3D_PASSIVE:
        case eDATA_NODE_DP_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_DP_COLOR_3D_PASSIVE:
        case eDATA_NODE_DP_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_3GSDI_HSG_3D_PASSIVE:
        case eDATA_NODE_3GSDI_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_3GSDI_COLOR_3D_PASSIVE:
        case eDATA_NODE_3GSDI_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_HDBASET_HSG_3D_PASSIVE:
        case eDATA_NODE_HDBASET_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_HDBASET_COLOR_3D_PASSIVE:
        case eDATA_NODE_HDBASET_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_12GSDI_HSG_3D_PASSIVE:
        case eDATA_NODE_12GSDI_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_12GSDI_COLOR_3D_PASSIVE:
        case eDATA_NODE_12GSDI_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_HSG_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_COLOR_3D_PASSIVE:
        case eDATA_NODE_PRESENTER_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_HSG_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_COLOR_3D_PASSIVE:
        case eDATA_NODE_CARDREADER_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_HSG_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_COLOR_3D_PASSIVE:
        case eDATA_NODE_MINIUSB_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT1_HSG_3D_PASSIVE:
        case eDATA_NODE_SLOT1_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT1_COLOR_3D_PASSIVE:
        case eDATA_NODE_SLOT1_COLOR_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT2_HSG_3D_PASSIVE:
        case eDATA_NODE_SLOT2_HSG_USER_3D_PASSIVE:
        case eDATA_NODE_SLOT2_COLOR_3D_PASSIVE:
        case eDATA_NODE_SLOT2_COLOR_USER_3D_PASSIVE:
            ucDisplaymode = eCM_PICTURE_SETTINGS_3D_PASSIVE;
            break;

        case eDATA_NODE_SOURCE0_HSG_HLG:
        case eDATA_NODE_SOURCE0_HSG_USER_HLG:
        case eDATA_NODE_SOURCE0_COLOR_HLG:
        case eDATA_NODE_SOURCE0_COLOR_USER_HLG:
        case eDATA_NODE_VGA_HSG_HLG:
        case eDATA_NODE_VGA_HSG_USER_HLG:
        case eDATA_NODE_VGA_COLOR_HLG:
        case eDATA_NODE_VGA_COLOR_USER_HLG:
        case eDATA_NODE_BNC_HSG_HLG:
        case eDATA_NODE_BNC_HSG_USER_HLG:
        case eDATA_NODE_BNC_COLOR_HLG:
        case eDATA_NODE_BNC_COLOR_USER_HLG:
        case eDATA_NODE_HDMI1_HSG_HLG:
        case eDATA_NODE_HDMI1_HSG_USER_HLG:
        case eDATA_NODE_HDMI1_COLOR_HLG:
        case eDATA_NODE_HDMI1_COLOR_USER_HLG:
        case eDATA_NODE_HDMI2_HSG_HLG:
        case eDATA_NODE_HDMI2_HSG_USER_HLG:
        case eDATA_NODE_HDMI2_COLOR_HLG:
        case eDATA_NODE_HDMI2_COLOR_USER_HLG:
        case eDATA_NODE_DVID_HSG_HLG:
        case eDATA_NODE_DVID_HSG_USER_HLG:
        case eDATA_NODE_DVID_COLOR_HLG:
        case eDATA_NODE_DVID_COLOR_USER_HLG:
        case eDATA_NODE_DP_HSG_HLG:
        case eDATA_NODE_DP_HSG_USER_HLG:
        case eDATA_NODE_DP_COLOR_HLG:
        case eDATA_NODE_DP_COLOR_USER_HLG:
        case eDATA_NODE_3GSDI_HSG_HLG:
        case eDATA_NODE_3GSDI_HSG_USER_HLG:
        case eDATA_NODE_3GSDI_COLOR_HLG:
        case eDATA_NODE_3GSDI_COLOR_USER_HLG:
        case eDATA_NODE_HDBASET_HSG_HLG:
        case eDATA_NODE_HDBASET_HSG_USER_HLG:
        case eDATA_NODE_HDBASET_COLOR_HLG:
        case eDATA_NODE_HDBASET_COLOR_USER_HLG:
        case eDATA_NODE_12GSDI_HSG_HLG:
        case eDATA_NODE_12GSDI_HSG_USER_HLG:
        case eDATA_NODE_12GSDI_COLOR_HLG:
        case eDATA_NODE_12GSDI_COLOR_USER_HLG:
        case eDATA_NODE_PRESENTER_HSG_HLG:
        case eDATA_NODE_PRESENTER_HSG_USER_HLG:
        case eDATA_NODE_PRESENTER_COLOR_HLG:
        case eDATA_NODE_PRESENTER_COLOR_USER_HLG:
        case eDATA_NODE_CARDREADER_HSG_HLG:
        case eDATA_NODE_CARDREADER_HSG_USER_HLG:
        case eDATA_NODE_CARDREADER_COLOR_HLG:
        case eDATA_NODE_CARDREADER_COLOR_USER_HLG:
        case eDATA_NODE_MINIUSB_HSG_HLG:
        case eDATA_NODE_MINIUSB_HSG_USER_HLG:
        case eDATA_NODE_MINIUSB_COLOR_HLG:
        case eDATA_NODE_MINIUSB_COLOR_USER_HLG:
        case eDATA_NODE_SLOT1_HSG_HLG:
        case eDATA_NODE_SLOT1_HSG_USER_HLG:
        case eDATA_NODE_SLOT1_COLOR_HLG:
        case eDATA_NODE_SLOT1_COLOR_USER_HLG:
        case eDATA_NODE_SLOT2_HSG_HLG:
        case eDATA_NODE_SLOT2_HSG_USER_HLG:
        case eDATA_NODE_SLOT2_COLOR_HLG:
        case eDATA_NODE_SLOT2_COLOR_USER_HLG:
            ucDisplaymode = eCM_PICTURE_SETTINGS_HLG;
            break;
    }

    ucGuiDisplayModeID = (UINT8)CM2GUI(edcPICTURE_SETTINGS, ucDisplaymode);

    //printf("ucGuiSrcID = %d, uiNode = %d\n", ucGuiSrcID, uiNode);

    if(ucGuiDisplayModeID > eGUI_PICTURE_SETTINGS_NUMBER)
    {
        return FALSE;
    }

    if(utilDataMgr_SourceSupport(uiNode) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}



