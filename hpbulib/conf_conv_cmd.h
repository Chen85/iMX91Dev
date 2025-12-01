#ifndef CONF_CONV_CMD_H
#define CONF_CONV_CMD_H

//=================================================================================================
#define NAME_GROUPING_CONFIG_NUMBER                 "Number"
#define NAME_GROUPING_CONFIG_GROUP_MASTER           "master"
#define NAME_GROUPING_CONFIG_ENABLE                 "En"
#define NAME_GROUPING_CONFIG_GROUP_ID               "config_group_id"
#define NAME_GROUPING_CONFIG_GROUP_NAME             "config_group_name"
#define NAME_GROUPING_CONFIG_PROJ_NAME              "proj_name"
#define NAME_GROUPING_CONFIG_EVENT_FREEZE           "FreezeEn"
#define NAME_GROUPING_CONFIG_EVENT_SHUTTER          "ShutterEn"
#define NAME_GROUPING_CONFIG_EVENT_DY_BLACL         "DynamicBlackEn"
#define NAME_GROUPING_CONFIG_EVENT_SCHEDULE         "ScheduleEn"
#define NAME_GROUPING_CONFIG_EVENT_AUTO_FOCUS       "AutoFocusEn"
#define NAME_GROUPING_CONFIG_EVENT_AUTO_COLOR_MATCH "AutoColorMatchEn"

#define NAME_GROUPING_GROUP_PREFIX                  "projector_"
//----------------------------------------
#define NUM_GROUPING_GROUP_PARAM                    12
#define NAME_GROUPING_GROUP_STATUS                  "status"
#define NAME_GROUPING_GROUP_GROUP_STATUS            "group_status"
#define NAME_GROUPING_GROUP_GROUP_ID_STATUS         "group_id_status"
#define NAME_GROUPING_GROUP_SN                      "sn"
#define NAME_GROUPING_GROUP_MODEL_NAME              "model_name"
#define NAME_GROUPING_GROUP_GROUP_NAME              "group_name"
#define NAME_GROUPING_GROUP_PROJ_NAME               "group_proj_name"
#define NAME_GROUPING_GROUP_IP                      "ip"
#define NAME_GROUPING_GROUP_MAC                     "mac"
#define NAME_GROUPING_GROUP_CAMERA_STATUS           "camera_status"
#define NAME_GROUPING_GROUP_FD                      "Fd"
#define NAME_GROUPING_GROUP_NOTE                    "note"
//----------------------------------------

#define MAX_PROJECTOR_STR_LEN                       32

typedef enum
{
    eCONF_CONV_MUTEX_SET_UNIT = 0,
    eCONF_CONV_MUTEX_SET_SELECT_LIST,
    eCONF_CONV_MUTEX_SET_SEARCH_LIST,

    eCONF_CONV_MUTEX_MAX
}eCONF_CONV_MUTEX;
//=================================================================================================
void ConfConvCmd_MutexInit();
u32_t ConfConvCmd_getProjectGroupSelectCmd(_sGroupSelectInfo *pInfo);
u32_t ConfConvCmd_getProjectGroupSearchCmd(_sGroupSearchInfo *pInfo);
u32_t ConfConvCmd_writeProjectGroupSearchConfByCmd(_sGroupSearchInfo *pData);
u32_t ConfConvCmd_writeProjectGroupSelectConfByCmd(_sGroupSelectInfo *pData);
//=================================================================================================

#endif //CONF_TO_CMD_H
