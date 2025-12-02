SUMMARY = "iScalerH2PF APP"
SECTION = "iScalerH2PF"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://*.h \
           file://*.c \ 
"


# PLATFORM_DEF      Select => 
#                              PLATFORM_A70G2 
#                              PLATFORM_A35G2 
#                              PLATFORM_H30_2K 
#                              PLATFORM_H60_2K 
#                              PLATFORM_A70LK 
#                              PLATFORM_H30_4K 
#                              PLATFORM_H60_4K 
#                              PLATFORM_R70K 

# SCALER_TYPE       Select => 
#                              FPGA_F34
#                              C821_C789 
#                              C341

# OUTPUT_PANEL_CFG  Select => 
#                              OUTPUT_PANEL_4K
#                              OUTPUT_PANEL_2K 

# DMD_RESOLUTION    Select => 
#                              DMD_RES_WUXGA
#                              DMD_RES_HD 

PLATFORM_DEF = "PLATFORM_A70LK"
CUSTOM_DEF   = "CUSTOM_CHRISTIE"
S = "${WORKDIR}"
EXEC_APP = "iScalerH2PF"
DEPENDS = "glib-2.0 dbus libpng dbus-glib hpbulib"
RDEPENDS_${PN} = "glib-2.0 dbus libpng dbus-glib hpbulib"
EXCLUDE_FROM_SHLIBS = "1"
LD_CTRL_DIR = "./LD_Ctrl/LD_LPC54605"
IMAGE_PROCESS_DIR = "./ImageProc"
DATAPATH_DIR = ""
INCLUDE_PLATFORM_PATH = "PLATFORM_Standard"
OUTPUT_PANEL_CFG = "OUTPUT_PANEL_4K"
DMD_RESOLUTION = "DMD_RES_WUXGA"

python () {
    platform = d.getVar('PLATFORM_DEF', True)
    
    if platform == 'PLATFORM_A70G2':
        d.setVar('LD_CTRL_DIR', './LD_Ctrl/LD_LPC54113')
        d.setVar('INCLUDE_PLATFORM_PATH', 'PLATFORM_A70G2')
        d.setVar('SCALER_TYPE', 'C821_C789')
        d.setVar('OUTPUT_PANEL_CFG', 'OUTPUT_PANEL_2K')
        d.setVar('DMD_RESOLUTION', 'DMD_RES_WUXGA')
    elif platform == 'PLATFORM_A35G2':
        d.setVar('LD_CTRL_DIR', './LD_Ctrl/LD_LPC54605')
        d.setVar('INCLUDE_PLATFORM_PATH', 'PLATFORM_A35G2')
        d.setVar('SCALER_TYPE', 'C821_C789')
        d.setVar('OUTPUT_PANEL_CFG', 'OUTPUT_PANEL_2K')
        d.setVar('DMD_RESOLUTION', 'DMD_RES_WUXGA')
    elif platform == 'PLATFORM_H30_2K':
        d.setVar('LD_CTRL_DIR', './LD_Ctrl/LD_LPC54113')
        d.setVar('INCLUDE_PLATFORM_PATH', 'PLATFORM_Standard')
        d.setVar('SCALER_TYPE', 'C341')
        d.setVar('OUTPUT_PANEL_CFG', 'OUTPUT_PANEL_2K')
        d.setVar('DMD_RESOLUTION', 'DMD_RES_WUXGA')
    elif platform == 'PLATFORM_H60_2K':
        d.setVar('LD_CTRL_DIR', './LD_Ctrl/LD_LPC54113')
        d.setVar('INCLUDE_PLATFORM_PATH', 'PLATFORM_Standard')
        d.setVar('SCALER_TYPE', 'C821_C789')
        d.setVar('OUTPUT_PANEL_CFG', 'OUTPUT_PANEL_2K')
        d.setVar('DMD_RESOLUTION', 'DMD_RES_WUXGA')
        
    elif platform == 'PLATFORM_A70LK':
        d.setVar('LD_CTRL_DIR', './LD_Ctrl/LD_LPC54113')
        d.setVar('INCLUDE_PLATFORM_PATH', 'PLATFORM_Standard_4K')
        d.setVar('SCALER_TYPE', 'FPGA_F34')
        d.setVar('OUTPUT_PANEL_CFG', 'OUTPUT_PANEL_4K')
        d.setVar('DMD_RESOLUTION', 'DMD_RES_HD')
        
    elif platform == 'PLATFORM_H30_4K':
        d.setVar('LD_CTRL_DIR', './LD_Ctrl/LD_LPC54605')
        d.setVar('INCLUDE_PLATFORM_PATH', 'PLATFORM_Standard_4K')
        d.setVar('SCALER_TYPE', 'C341')
        d.setVar('OUTPUT_PANEL_CFG', 'OUTPUT_PANEL_4K')
        d.setVar('DMD_RESOLUTION', 'DMD_RES_WUXGA')
    elif platform == 'PLATFORM_H60_4K':
        d.setVar('LD_CTRL_DIR', './LD_Ctrl/LD_LPC54113')
        d.setVar('INCLUDE_PLATFORM_PATH', 'PLATFORM_Standard_4K')
        d.setVar('SCALER_TYPE', 'FPGA_F34')
        d.setVar('OUTPUT_PANEL_CFG', 'OUTPUT_PANEL_4K')
        d.setVar('DMD_RESOLUTION', 'DMD_RES_WUXGA')    
    elif platform == 'PLATFORM_R70K':
        d.setVar('LD_CTRL_DIR', './LD_Ctrl/LD_LPC54113')
        d.setVar('INCLUDE_PLATFORM_PATH', 'PLATFORM_Standard_4K')
        d.setVar('SCALER_TYPE', 'FPGA_F34')
        d.setVar('OUTPUT_PANEL_CFG', 'OUTPUT_PANEL_4K')
        d.setVar('DMD_RESOLUTION', 'DMD_RES_HD')    
    else:
        d.setVar('LD_CTRL_DIR', './LD_Ctrl/LD_LPC54113')
        d.setVar('INCLUDE_PLATFORM_PATH', 'PLATFORM_Standard')
        d.setVar('SCALER_TYPE', 'C821_C789')
        d.setVar('OUTPUT_PANEL_CFG', 'OUTPUT_PANEL_4K')
        d.setVar('DMD_RESOLUTION', 'DMD_RES_WUXGA')    

#   select Scaler/Warping Folder 
    ScalerType = d.getVar('SCALER_TYPE', True)
    if ScalerType == 'FPGA_F34':
        d.setVar('IMAGE_PROCESS_DIR', './ImageProc/proav')
        d.setVar('DATAPATH_DIR', './ImageProc/proav/DataPath')
    elif ScalerType == 'C821_C789':
        d.setVar('IMAGE_PROCESS_DIR', './ImageProc/iChips/C821_C789')
        d.setVar('DATAPATH_DIR', './ImageProc/iChips')
    elif ScalerType == 'C341':
        d.setVar('IMAGE_PROCESS_DIR', './ImageProc/iChips/C341')
        d.setVar('DATAPATH_DIR', './ImageProc/iChips')
}

do_compile() {
        bbplain "-----------------------------------------------------"
        bbplain "-----------------------------------------------------"
        bbplain "PLATFORM_DEF           : ${PLATFORM_DEF}"
        bbplain "CUSTOM_DEF             : ${CUSTOM_DEF}"
        bbplain "INCLUDE_PLATFORM_PATH  : ${INCLUDE_PLATFORM_PATH}"
        bbplain "LD_CTRL_DIR            : ${LD_CTRL_DIR}"
        bbplain "SCALER_TYPE            : ${SCALER_TYPE}"
        bbplain "IMAGE_PROCESS_DIR      : ${IMAGE_PROCESS_DIR}"
        bbplain "DATAPATH_DIR           : ${DATAPATH_DIR}"
        bbplain "OUTPUT_PANEL_CFG       : ${OUTPUT_PANEL_CFG}"
        bbplain "DMD_RESOLUTION         : ${DMD_RESOLUTION}"
        bbplain "-----------------------------------------------------"
        bbplain "-----------------------------------------------------"
        
        ${CC} -D${PLATFORM_DEF} -D${CUSTOM_DEF} -D${SCALER_TYPE} -D${OUTPUT_PANEL_CFG} -D${DMD_RESOLUTION} -DLow_Latency_All${CFLAGS} \
              -I${INCLUDE_PLATFORM_PATH}/TempDataMapTable -I${INCLUDE_PLATFORM_PATH} -I${CUSTOM_DEF} -I${LD_CTRL_DIR} \
              -I${IMAGE_PROCESS_DIR} -I${IMAGE_PROCESS_DIR}/driver/include -I${DATAPATH_DIR} -I./ -I./inc -I./ColorTable\
        `pkg-config --cflags dbus-glib-1 glib-2.0` ${LIBS} `pkg-config --libs dbus-glib-1 glib-2.0` \
        -ldbus-1 -ldbus-glib-1 -lglib-2.0 -lpng -lhpbulib \
        -lm -lpthread -lcrypt -lrt *.c ./IPC_DBUS/*.c \
        ${INCLUDE_PLATFORM_PATH}/*.c ${LD_CTRL_DIR}/*.c ${IMAGE_PROCESS_DIR}/*.c ${IMAGE_PROCESS_DIR}/driver/*.c ${DATAPATH_DIR}/*.c ${INCLUDE_PLATFORM_PATH}/TempDataMapTable/*.c \
        -o ${EXEC_APP}
}

do_install() {
        install -d ${D}${bindir}
        install -m 0755 ${EXEC_APP} ${D}${bindir}
}

#python () {    
#    p_def = d.getVar('PLATFORM_DEF', True)
#    c_def = d.getVar('CUSTOM_DEF', True)
#    bb.plain("\nPLATFORM_DEF : ",p_def)
#    bb.plain("CUSTOM_DEF : ",c_def)
#}