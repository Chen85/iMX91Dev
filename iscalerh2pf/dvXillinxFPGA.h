// ===============================================================================
// FILE NAME: dvXillinxFPGA.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2018/06/22, Doulas Create
// --------------------
// ===============================================================================


#ifndef DVMOTOR_DRIVER_H
#define DVMOTOR_DRIVER_H
//#include "CommonDef.h"
#include "Common.h"

//A35G2_Simon_0065 move
typedef enum
{
// Common Command

    eXFPGA_CMD_BL_VER              = 0x03,   // REG_reversion low byte
    eXFPGA_CMD_BH_VER              = 0x04,   // REG_reversion High byte

    eXFPGA_CMD_LVDS_VID_MAP_SEL    = 0x14,   // REG_lvds_vid_map_sel
/************************************************************************
 *  Address:    0x14                                                    *
 *  Name:       REG_lvds_vid_map_sel                                    *
 *  Description:    [7:3]   TBD                                         *
 *                  [2:0]   111: JEIDA 10-bit                           *
 *                          110: JEIDA 10-bit                           *
 *                          101: JEIDA 8-bit                            *
 *                          100: JEIDA 6-bit                            *
 *                          011: VESA 10-bit                            *
 *                          010: VESA 10-bit                            *
 *                          001: VESA 8-bit                             *
 *                          000: VESA 6-bit                             *
 ************************************************************************/

    eXFPGA_CMD_DDP_VID_MAP_SWAP     = 0x15,   // REG_ddp_vid_map_swap
/************************************************************************
 *  Reg Address:    0x15                                                *
 *  Name:       REG_ddp_vid_map_swap                                    *
 *  Description:    [7:1]   TBD                                         *
 *                  [0]     0: master bus is odd pixel,                 *
 *                             slave bus is even pixel                  *
 ************************************************************************/

    eXFPGA_CMD_DRP_SADDR            = 0x16,   // REG_drp_saddr
/************************************************************************
 *  Reg Address:    0x16                                                *
 *  Name:       REG_drp_saddr                                           *
 *  Description:    [7:1]   TBD                                         *
 *                  [1:0]   2'b00, 2'b11 if lvds_clk > 102.5MHz         *
 *                          2'b01 if 51MHz < lvds_clk <= 102.5MHz       *
 *                          2'b10 if 51MHz >= lvds_clk                  *
 ************************************************************************/

    eXFPGA_CMD_DRP_STATUS           = 0x17,   // REG_drp_status
/************************************************************************
 *  Reg Address:    0x17                                                *
 *  Name:       REG_drp_status                                          *
 *  Description:    [7:1]   TBD                                         *
 *                  [0]     1: drp_config_lock signal                   *
 ************************************************************************/

    eXFPGA_CMD_AUTO_DET             = 0x18,   // REG_auto_det
/************************************************************************
 *  Reg Address:    0x18                                                *
 *  Name:       REG_auto_det                                            *
 *  Description:    [7:1]   TBD                                         *
 *                  [0]     1: enable auto detect lvds bit rate         *
 ************************************************************************/

    eXFPGA_CMD_LVDS_RST             = 0x19,   // REG_lvds_rst
/************************************************************************
 *  Reg Address:    0x19                                                *
 *  Name:       REG_lvds_rst                                            *
 *  Description:    [7:1]   TBD                                         *
 *                  [0]     1: reset LVDS channel and MMCM              *
 ************************************************************************/

    eXFPGA_CMD_PHASE_UNLOCK         = 0x20,   // ph_unlocked               //A70LV_Doulas_0262
/************************************************************************
 *  Reg Address:    0x20                                                *
 *  Name:       ph_unlocked                                             *
 *  Description:    [7:1]   TBD                                         *
 *                  [0]     1: lvds clock phase unlocked                *
 ************************************************************************/

    eXFPGA_CMD_LVDS_COVER             = 0x21,   // REG_ph_recover            //A70LV_Doulas_0262
/************************************************************************
 *  Reg Address:    0x21                                                *
 *  Name:       REG_ph_recover                                            *
 *  Description:    [7:1]   TBD                                         *
 *                  [0]     0: enable lvds channel slave retry         *
 *                  [0]     1: keep functional circuit of Xilinx design         *
 ************************************************************************/

	//G100_Doulas_0001 Add
	eXFPGA_CMD_LVDS_TOL_VAL  	= 0x22,  	//tol_val ,tolerance value, means how many times that current using clock rate is continuously different with detect clock rate then it would unlock clock rate, need to do lvds_rst before apply new value
    eXFPGA_CMD_LVDS_LOCK_VAL  	= 0x23,  	//lock_val ,lock value, means how many times that current using clock rate is the same with last detect clock rate then it would lock clock rate, need to do lvds_rst before apply new value
    eXFPGA_CMD_LVDS_ERR_MULT  	= 0x24,  	//err_mult ,define error = err_mult value * 0.5%, need to do lvds_rst before apply new value
    eXFPGA_CMD_LVDS_25			= 0x25,		//ddp_det_ctrl
    										//bit0 : Enable main DDP sync unstable detection
    										//bit1 : Enable sub DDP sync unstable detection
    										//bit2 : Enable auto reset when detecting unstable main DDP sync
    										//bit3 : Enable auto reset when detecting unstable sub DDP sync
 	eXFPGA_CMD_LVDS_26			= 0x26,		//ddp_det_cycle
	eXFPGA_CMD_LVDS_27			= 0x27,		//ddp_det_flag
	eXFPGA_CMD_LVDS_28			= 0x28,		//mmcm_sw_rst
	eXFPGA_CMD_LVDS_29			= 0x29,		//mmcm_sw_rst
	eXFPGA_CMD_LVDS_2A			= 0x2A,		//meas_out, Measure output
                                            //00h: DDP main unstable counter
                                            //01h: DDP sub unstable counter
                                            //02h: measrued main sync stable cycles [7:0]
                                            //03h: measrued main sync stable cycles [15:8]
                                            //04h: measrued main sync stable cycles [16:16]
                                            //05h: measrued main sync stable cycles [29:24]
                                            //06h: measrued main sub stable cycles [7:0]
                                            //07h: measrued main sub stable cycles [15:8]
                                            //08h: measrued main sub stable cycles [16:16]
                                            //09h: measrued main sub stable cycles [29:24]
                                            //0Ah: clockrate stabilizer lock cnt

    eXFPGA_CMD_OSD_CTRL             = 0x30, //[0]   OSD pattern enable
                                            //[1]   Block table all clear
    eXFPGA_CMD_OSD_BLKTB_WR         = 0x31, //[0]   Block table entry write (auto clear by hardware)
    eXFPGA_CMD_OSD_BLKTB_UPDT       = 0x32, //[0]   Block table entry display update
                                            //[1]   Block table entry origin update
                                            //[2]   Block table entry size update
                                            //[3]   Block table entry layer update
    eXFPGA_CMD_OSD_BLKTB_IDX0       = 0x33, //[7:0] Block table entry index[7:0]
    eXFPGA_CMD_OSD_BLKTB_IDX1       = 0x34, //[0]   Block table entry index[8]
    eXFPGA_CMD_OSD_BLKTB_DSPEN      = 0x35, //[0]   Block table entry display enable
    eXFPGA_CMD_OSD_BLKTB_ORGX0      = 0x36, //[7:0] Block table entry origin X[7:0]
    eXFPGA_CMD_OSD_BLKTB_ORGX1      = 0x37, //[2:0] Block table entry origin X[10:8]
    eXFPGA_CMD_OSD_BLKTB_ORGY0      = 0x38, //[7:0] Block table entry origin Y[7:0]
    eXFPGA_CMD_OSD_BLKTB_ORGY1      = 0x39, //[2:0] Block table entry origin Y[10:8]
    eXFPGA_CMD_OSD_BLKTB_WID0       = 0x3A, //[7:0] Block table entry width[7:0]
    eXFPGA_CMD_OSD_BLKTB_WID1       = 0x3B, //[0]   Block table entry width[8]
    eXFPGA_CMD_OSD_BLKTB_HEIGHT0    = 0x3C, //[7:0] Block table entry height[7:0]
    eXFPGA_CMD_OSD_BLKTB_HEIGHT1    = 0x3D, //[2:0] Block table entry height[10:8]
    eXFPGA_CMD_OSD_BLKTB_LAYER      = 0x3E, //[3:0] Block table entry layer (1~8)
    eXFPGA_CMD_OSD_LYRTB_WR         = 0x40, //[0]   Layer table write (auto clear by hardware)
    eXFPGA_CMD_OSD_LYRTB_IDX        = 0x41, //[3:0] Layer table index (1~8, the larger numer is the higher layer)
    eXFPGA_CMD_OSD_LYRTB_R0         = 0x42, //[7:0] Layer table color Red[7:0]
    eXFPGA_CMD_OSD_LYRTB_R1         = 0x43, //[1:0] Layer table color Red[9:8]
    eXFPGA_CMD_OSD_LYRTB_G0         = 0x44, //[7:0] Layer table color Green[7:0]
    eXFPGA_CMD_OSD_LYRTB_G1         = 0x45, //[1:0] Layer table color Green[9:8]
    eXFPGA_CMD_OSD_LYRTB_B0         = 0x46, //[7:0] Layer table color Blue[7:0]
    eXFPGA_CMD_OSD_LYRTB_B1         = 0x47, //[1:0] Layer table color Blue[9:8]

	eXFPGA_CMD_LVDS_50  		= 0x50,
	eXFPGA_CMD_LVDS_51  		= 0x51,  	//relock_cnt
	eXFPGA_CMD_LVDS_52  		= 0x52,  	//retry_cnt
	eXFPGA_CMD_LVDS_53  		= 0x53,
	//G100_Doulas_0001 end

    eXFPGA_CMD_LOCK_REGS            = 0xF0,   // REG_lock_regs
/************************************************************************
 *  Reg Address:    0xF0                                                *
 *  Name:       REG_lock_regs                                           *
 *  Description:    [7:1]   TBD                                         *
 *                  [0]     1: enable internal registers writable       *
 ************************************************************************/

    eXFPGA_CMD_NUMBERS,
} eXFPGA_CMD;


eRESULT dvXillinxFPGARegWrite(UINT8 reg, UINT16 size, UINT8 *data);
eRESULT dvXillinxFPGARegRead(UINT8 reg, UINT16 size, UINT8 *data);


#endif /* DVMOTOR_DRIVER_H */


