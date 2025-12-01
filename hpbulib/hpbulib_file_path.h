#ifndef HPBU_LIB_FILE_PATH_H
#define HPBU_LIB_FILE_PATH_H
//=================================================================================================
#ifndef __UBUNTU_SIMULATOR__
#define PATH_USER_CONFS_DIR                 "/usr/configs/"
#define PATH_MNT_CONFS_DIR                  "/mnt/configs/"
#define PATH_TMP_CONFS_DIR                  "/tmp/"
#else
#define PATH_USER_CONFS_DIR                 "../hpbulib/conf/usr/"
#define PATH_MNT_CONFS_DIR                  "../hpbulib/conf/mnt/"
#define PATH_TMP_CONFS_DIR                  "../hpbulib/conf/tmp/"
#endif /* __UBUNTU_SIMULATOR__ */

//=================================================================================================
#define PATH_UPGRADE_CNT_FILE                   PATH_USER_CONFS_DIR"upgrade_cnt.conf"
#define PATH_PROJECTOR_GROUP_DEFAULT            PATH_USER_CONFS_DIR"projector_group_select.conf"
#define PATH_PROJECTOR_GROUP_SELECT             PATH_MNT_CONFS_DIR"projector_group_select.conf"
#define PATH_PROJECTOR_GROUP_SEARCH             PATH_TMP_CONFS_DIR"projector_group_search.conf"
//-------------------------------------------------------------------------------------------------
#define PATH_LAN_VERSION_FILE                   PATH_USER_CONFS_DIR"/version.conf"
#define PATH_DEFAULT_LAN_CONFIG_FILE            PATH_USER_CONFS_DIR"/net.conf"
#define PATH_DEFAULT_LAN_IPV6_CONFIG_FILE       PATH_USER_CONFS_DIR"/net6.conf"
#define PATH_DEFAULT_LAN_AP_CONFIG_FILE         PATH_USER_CONFS_DIR"/ap_status.conf"
#define PATH_DEFAULT_WLAN_CONFIG_FILE           PATH_USER_CONFS_DIR"/wifi.conf"
#define PATH_DEFAULT_SNMP_CONFIG_FILE           PATH_USER_CONFS_DIR"/snmp.conf"
#define PATH_DEFAULT_NOTIFY_CONFIG_FILE         PATH_USER_CONFS_DIR"/notifications.conf"
#define PATH_DEFAULT_DATETIME_CONFIG_FILE       PATH_USER_CONFS_DIR"/time.conf"
#define PATH_DEFAULT_PJLINK_CONFIG_FILE         PATH_USER_CONFS_DIR"/pjlink.conf"
#define PATH_DEFAULT_SCHEDULE_CONFIG_FILE       PATH_USER_CONFS_DIR"/schedule.conf"
#define PATH_DEFAULT_CRESTRONTWO_CONFIG_FILE    PATH_USER_CONFS_DIR"/crestron2.conf"
#define PATH_DEFAULT_SCHEDULE_DAILY_CONFIG_FILE PATH_USER_CONFS_DIR"/schedule_daily.conf"
//-------------------------------------------------------------------------------------------------
#define PATH_LAN_CONFIG_FILE                    PATH_MNT_CONFS_DIR"/net.conf"
#define PATH_LAN_AP_CONFIG_FILE                 PATH_MNT_CONFS_DIR"/ap_status.conf"
#define PATH_WLAN_CONFIG_FILE                   PATH_MNT_CONFS_DIR"/wifi.conf"
#define PATH_SNMP_CONFIG_FILE                   PATH_MNT_CONFS_DIR"/snmp.conf"
#define PATH_NOTIFY_CONFIG_FILE                 PATH_MNT_CONFS_DIR"/notifications.conf"
#define PATH_DATETIME_CONFIG_FILE               PATH_MNT_CONFS_DIR"/time.conf"
#define PATH_PJLINK_CONFIG_FILE                 PATH_MNT_CONFS_DIR"/pjlink.conf"
#define PATH_SCHEDULE_CONFIG_FILE               PATH_MNT_CONFS_DIR"/schedule.conf"
#define PATH_CRESTRONTWO_CONFIG_FILE            PATH_MNT_CONFS_DIR"/crestron2.conf"
#define PATH_SCHEDULE_DAILY_CONFIG_FILE         PATH_MNT_CONFS_DIR"/schedule_daily.conf"
//=================================================================================================
#define NAME_UPGRADE_CNT                        "UpgradeCnt"
//-------------------------------------------------------------------------------------------------
#define NAME_ITEM_LAN_DHCP                      "Dhcp"
#define NAME_ITEM_LAN_IP                        "IpAddr"
#define NAME_ITEM_LAN_SUBMASK                   "SubMask"
#define NAME_ITEM_LAN_GATEWAY                   "Gateway"
#define NAME_ITEM_LAN_DNS_1                     "PrimaryDns"
#define NAME_ITEM_LAN_DNS_2                     "SecondDns"
#define NAME_ITEM_LAN_MAC                       "Mac"
#define NAME_ITEM_LAN_INTERFACE_PORT            "Port"
//-------------------------------------------------------------------------------------------------
#define NAME_ITEM_LAN_IPV6_DHCP                 "Dhcp"
#define NAME_ITEM_LAN_IPV6_IP                   "IpAddr"
#define NAME_ITEM_LAN_IPV6_SUBMASK              "PrefixLen"
#define NAME_ITEM_LAN_IPV6_GATEWAY              "Gateway"
#define NAME_ITEM_LAN_IPV6_DNS_1                "Dns"
//-------------------------------------------------------------------------------------------------
#define NAME_ITEM_WLAN_WIFI                     "Wifi"
#define NAME_ITEM_WLAN_SSID                     "WifiSSID"
#define NAME_ITEM_WLAN_PW                       "WifiPassword"
#define NAME_ITEM_WLAN_START_IP                 "StartIpAddr"
#define NAME_ITEM_WLAN_END_IP                   "EndIpAddr"
#define NAME_ITEM_WLAN_SUBMASK                  "WifiSubMask"
#define NAME_ITEM_WLAN_GATEWAY                  "WifiGateway"
#define NAME_ITEM_WLAN_DONGLE_STATUS            "WifiDongleStatus"
//=================================================================================================
#define LOGO_PICTURE_PATH                       "/mnt/patterns/Logo"
#define LOGO_REPLACEMENT_DEFAULT_2D             LOGO_PICTURE_PATH"/Logo_Replacement_Default_2D.png"
#define LOGO_REPLACEMENT_DEFAULT_3D             LOGO_PICTURE_PATH"/Logo_Replacement_Default_3D.png"
#define LOGO_REPLACEMENT_DEFAULT_2D_RGB565      LOGO_PICTURE_PATH"/Logo_Replacement_Default_2D.bin"
#define LOGO_REPLACEMENT_DEFAULT_3D_RGB565      LOGO_PICTURE_PATH"/Logo_Replacement_Default_3D.bin"
#define LOGO_REPLACEMENT_DEFAULT_2D_RAW         LOGO_PICTURE_PATH"/Logo_Replacement_Default_2D.raw"
#define LOGO_REPLACEMENT_DEFAULT_3D_RAW         LOGO_PICTURE_PATH"/Logo_Replacement_Default_3D.raw"
#define LOGO_REPLACEMENT_DEFAULT_2D_PALETTE     LOGO_PICTURE_PATH"/Logo_Replacement_Default_2D.palette"
#define LOGO_REPLACEMENT_DEFAULT_3D_PALETTE     LOGO_PICTURE_PATH"/Logo_Replacement_Default_3D.palette"
#define LOGO_REPLACEMENT_DEFAULT_2D_STATUS      LOGO_PICTURE_PATH"/Logo_Replacement_Default_2D_Status"
#define LOGO_REPLACEMENT_DEFAULT_3D_STATUS      LOGO_PICTURE_PATH"/Logo_Replacement_Default_3D_Status"
#define LOGO_REPLACEMENT_USER_2D                LOGO_PICTURE_PATH"/Logo_Replacement_User_3D.png"
#define LOGO_REPLACEMENT_USER_3D                LOGO_PICTURE_PATH"/Logo_Replacement_User_2D.png"
#define LOGO_REPLACEMENT_USER_2D_RGB565         LOGO_PICTURE_PATH"/Logo_Replacement_User_3D.bin"
#define LOGO_REPLACEMENT_USER_3D_RGB565         LOGO_PICTURE_PATH"/Logo_Replacement_User_2D.bin"
#define LOGO_REPLACEMENT_USER_2D_RAW            LOGO_PICTURE_PATH"/Logo_Replacement_User_3D.raw"
#define LOGO_REPLACEMENT_USER_3D_RAW            LOGO_PICTURE_PATH"/Logo_Replacement_User_2D.raw"
#define LOGO_REPLACEMENT_USER_2D_PALETTE        LOGO_PICTURE_PATH"/Logo_Replacement_User_3D.palette"
#define LOGO_REPLACEMENT_USER_3D_PALETTE        LOGO_PICTURE_PATH"/Logo_Replacement_User_2D.palette"
#define LOGO_REPLACEMENT_USER_2D_STATUS         LOGO_PICTURE_PATH"/Logo_Replacement_User_3D_Status"
#define LOGO_REPLACEMENT_USER_3D_STATUS         LOGO_PICTURE_PATH"/Logo_Replacement_User_2D_Status"
//=================================================================================================
#define MAX_PROJECTOR_GROUP_SEARCH_NUM      60
#define MAX_PROJECTOR_GROUP_SELECT_NUM      8
//-------------------------------------------------------------------------------------------------
#define MAX_LENGTH_NET_INFO_STR             32
//=================================================================================================
#endif //HPBU_LIB_FILE_PATH_H
