#ifndef _ICHIPS341MEMTABLE_H_
#define _ICHIPS341MEMTABLE_H_



///***  !!!  修改此檔案，必須同步複製到 CT 和 PF                !!! ***///

#define ALIGN_ADDR(addr, align) (((addr) + (align) - 1) & ~((align) - 1))           //H2PF_Simon_0110

////////////////////////////////////////////////////////////////////////////////////////////////
//  Scaler
////////////////////////////////////////////////////////////////////////////////////////////////

//4K
#define DEF_ISFLD0CH1_4K	            0x00000000  //PS_MEM_SC
#define DEF_ISFLD1CH1_4K	            0x025A8000  //4096x2410x4 = 39485440 = 0x025A8000
#define DEF_ISFLD2CH1_4K                0x04B50000
#define DEF_ISFLD3CH1_4K                0x070F8000
#define DEF_ISFLD4CH1_4K                0x096A0000

//2K
#define DEF_MVFLDCH1_2K_3D_1		    0x16cea000 //H30K_Doulas_0008//H30K_Doulas_0001
#define DEF_MVFLDCH1_2K_3D_2		    0x18cea000 //H30K_Doulas_0001
#define DEF_MVFLDCH2_2K_3D_1		    0x16cea000 //H30K_Doulas_0001
#define DEF_MVFLDCH2_2K_3D_2		    0x18cea000 //H30K_Doulas_0001

//4K
#define DEF_SCMWICH1_4K		            0x10    //PS_MWI_SC
#define DEF_IPMWICH1_4K		            0x23    //PS_MWI_IP
#define DEF_RTMWICH1_4K		            0x00    //PS_MWI_RT
#define DEF_IPYOFFCH1_4K                0x00CF  //PS_MEM_IPYOFF
#define DEF_MVFLDCH1_4K		            0x0C932000  //PS_MEM_MV

#define DEF_IPISFLD0CH1_2K_3D	        0x16000000//0x03223C00//0x0282DC00  //H30K_Doulas_0008//H30K_Doulas_0002//H30K_Doulas_0001
#define DEF_IPISFLD1CH1_2K_3D		    0x16339000//0x03223000//0x0282D000
#define DEF_IPISFLD0CH2_2K_3D		    0x18000000//0x03555400//0x02B5F400
#define DEF_IPISFLD1CH2_2K_3D           0x18339000//0x03554800//0x02B5E800

#define DEF_IPOSFLD0CH1_2K_3D      		DEF_IPISFLD0CH1_2K_3D  //H30K_Doulas_0002//H30K_Doulas_0001
#define DEF_IPOSFLD1CH1_2K_3D		    DEF_IPISFLD1CH1_2K_3D
#define DEF_IPOSFLD0CH2_2K_3D		    DEF_IPISFLD0CH2_2K_3D
#define DEF_IPOSFLD1CH2_2K_3D		    DEF_IPISFLD1CH2_2K_3D

//PS_MEM_IP_I   //DEF_IPISFLD0CH1  //B23_IPISFLD0CH1
//PS_MEM_IP_P   //DEF_IPISFLD0CH1
#define DEF_IP_I_ISFLD0CH1_4K	            0x0BC48000  //CH1CH2
#define DEF_IP_I_ISFLD1CH1_4K		        0x0BF81000
#define DEF_IP_I_ISFLD0CH2_4K      		    0x19748800  //CH1CH2
#define DEF_IP_I_ISFLD1CH2_4K		        0x19A81800

#define DEF_IP_P_ISFLD0CH1_4K	            0x0C35F000  //CH3CH4
#define DEF_IP_P_ISFLD1CH1_4K		        0x0BC48000
#define DEF_IP_P_ISFLD0CH2_4K	            0x19E5F800  //CH3CH4
#define DEF_IP_P_ISFLD1CH2_4K		        0x19748800

//4K
#define DEF_ISFLD0CH2_4K	            0x0DB00800
#define DEF_ISFLD1CH2_4K                0x100A8800
#define DEF_ISFLD2CH2_4K                0x12650800
#define DEF_ISFLD3CH2_4K  	            0x14BF8800
#define DEF_ISFLD4CH2_4K  	            0x171A0800

//4K
#define DEF_SCMWICH2_4K		            0x10    //PS_MWI_SC
#define DEF_IPMWICH2_4K		            0x23    //PS_MWI_IP
#define DEF_RTMWICH2_4K		            0x00    //PS_MWI_RT
#define DEF_IPYOFFCH2_4K                0x00CF  //PS_MEM_IPYOFF
#define DEF_MVFLDCH2_4K		            0x1A432800  //PS_MEM_MV

#define DEF_BITMAPAD_2K                 0x0CFA0000  //for Bitmap used size=4096x2160 0x870000 //A70LV_Larry_0001
#define DEF_FONTAD_2K                   0x0D810000
#define DEF_OSDMWI_2K                   0x0F
#define DEF_MPROTECT_2K                 0x8B63

#define DEF_LOGOSFL_ADDR                0x01500000  //G100_Owen_0051
#define DEF_LOGOSFL_SIZE                0x00980000
#define DEF_LOGODDR_SIZE                0x00974000
#define DEF_LOGOREG_ADDR                0x01E74200  //G100_Owen_0051


////////////////////////////////////////////////////////////////////////////////////////////////
//  Warping
////////////////////////////////////////////////////////////////////////////////////////////////

#define DEF_WPEGBDBDSAD0    0x25000000
#define DEF_WPEBIASSAD0     0x25870000
#define DEF_WPEGBDBDSAD1    0x260E0000
#define DEF_WPEBIASSAD1     0x26950000


#define OSD_BASIC_ADDR		0x06000000	//size=0x300000*2(2048x1200=0x258000; two layers), addr=0x06000000
#define OSD_TEMP_ADDR		0x06600000	//size=0x300000(2048x1200=0x258000; one layers), addr=OSD_BASIC_ADDR+0x300000*2
#define OSD_FONT_ADDR		0x06900000	//size=50k<0x100000, addr=OSD_BASIC_ADDR+0x300000*3
#define OSD_BMP_ADDR		0x06A00000	//size=100k<0x200000, addr=OSD_FONT_ADDR+0x100000
#define RAM_DATA_ADDR		0x06C00000  //0x06C00000	//size=0x300000(2048x1200=0x258000), addr=OSD_BMP_ADDR+0x200000
#define OSD_BIAS_ADDR		DEF_WPEBIASSAD0 //0x2B594800	//size=0x200000(1024x1200=0x12C000), addr=RAM_DATA_ADDR+0x300000
#define OSD_DBD_ADDR		DEF_WPEGBDBDSAD0 //0x2B524000	//size=0x300000(2560x1200=0x2EE000, for 10bit:1920/6*8=2560), addr=OSD_BIAS_ADDR+0x200000

//#define DEF_WPISFLD0CH1     0x16200000
//#define DEF_WPISFLD1CH1     0x0F000000
//#define DEF_WPISFLD2CH1     0x11600000
//#define DEF_WPISFLD3CH1     0x13C00000

#define DEF_WPISFLD0CH1     (DEF_WPISFLD1CH1 + 0x07200000)
#define DEF_WPISFLD1CH1     (0x1B000000)
#define DEF_WPISFLD2CH1     (DEF_WPISFLD1CH1 + 0x02600000)
#define DEF_WPISFLD3CH1     (DEF_WPISFLD1CH1 + 0x04C00000)

#define DEF_WPOSFLD0CH1     DEF_WPISFLD0CH1   //WPOSFLD 寫入 register 時，需先 shift right 15
#define DEF_WPOSFLD1CH1     DEF_WPISFLD1CH1
#define DEF_WPOSFLD2CH1     DEF_WPISFLD2CH1
#define DEF_WPOSFLD3CH1     DEF_WPISFLD3CH1

//===== Address =====//
#define DEF_OSD_PANEL_WIDTH         1920
#define DEF_OSD_PANEL_HEIGHT        1200
#define DEF_OSDSAD_LAYER_OFFSET     (DEF_OSD_PANEL_WIDTH*DEF_OSD_PANEL_HEIGHT)

#define DEF_OSDMWI			        0x0F
#define TWIST_DEF_BLEND_ADDRESS	    OSD_DBD_ADDR//0x05000000		//G100_Doulas_0027
#define TWIST_DEF_OSDSAD      	    OSD_BASIC_ADDR//0x06000000	//G100_Doulas_0027
#define DEF_BLEND_ADDRESS           OSD_DBD_ADDR//0x05000000		//G100_Doulas_0027

#define DEF_MEMORY_ADDRESS          0


////////////////////////////////////////////////////////////////////////////////////////////////
//  OSD
////////////////////////////////////////////////////////////////////////////////////////////////     //H2PF_Simon_0107
#define OSD_PANEL_MAX_WIDTH             (3840)
#define OSD_PANEL_MAX_HEIGHT            (2400)
#define OSD_PANEL_MAX_SIZE              (OSD_PANEL_MAX_WIDTH * OSD_PANEL_MAX_HEIGHT)  //0x8CA000
#define GET_PANEL_HW()                  dvC341_Get_Panel_H_Active()

//PS_PANEL_HW 為 3840 時， OSDSAD 加上 0x80 可節省頻寬
//PS_PANEL_HW 為 1920 時， OSDSAD 加上 0x40 可節省頻寬
//OSD1 Addr
#define DEF_OSDSAD_LAYER0_4K            (0x2B650000 + ((GET_PANEL_HW() == 3840) ? (0x80) : \
                                                       (GET_PANEL_HW() == 1920) ? (0x40) : \
                                                       (0x00)))                                                  //reserve 3840x2400 = 0x8CA000
#define DEF_OSDSAD_LAYER1_4K            (DEF_OSDSAD_LAYER0_4K + OSD_PANEL_MAX_SIZE)  //0x2BF1A000 + 0x80 or 0x40  //reserve 3840x2400 = 0x8CA000
//OSD2 Addr
#define DEF_OSDSAD_LAYER3_4K            (DEF_OSDSAD_LAYER1_4K + OSD_PANEL_MAX_SIZE)  //0x2C7E4000 + 0x80 or 0x40  //reserve 3840x2400 = 0x8CA000
#define DEF_OSDSAD_LAYER4_4K            (DEF_OSDSAD_LAYER3_4K + OSD_PANEL_MAX_SIZE)  //0x2D0AE000 + 0x80 or 0x40  //reserve 3840x2400 = 0x8CA000

//OSD Bitmap Addr
#define DEF_BITMAPAD_4K                 (DEF_OSDSAD_LAYER4_4K + OSD_PANEL_MAX_SIZE)  //0x2D978000 + 0x80 or 0x40  //for Bitmap Raw Data
#define OSD_BITMAP_WIDTH                (1920)
#define OSD_BITMAP_MAX_HEIGHT           (4800)
#define OSD_BITMAP_SIZE                 (OSD_BITMAP_WIDTH * OSD_BITMAP_MAX_HEIGHT)   //or BITMAP_RAW_DATA_SIZE

//OSD Font Addr
#define DEF_FONTAD_4K                   ALIGN_ADDR((DEF_BITMAPAD_4K + OSD_BITMAP_SIZE), 0x200)     //0x2E242000 + 0x200  //for Font Raw Data
#define OSD_FONT_MAX_SIZE               (0x300000)

//Edge Blending Font   //需要 align 0x200
#define DEF_GEO_FONTAD_4K               ALIGN_ADDR((DEF_FONTAD_4K + OSD_FONT_MAX_SIZE), 0x200)     //0x2E542200   //for Egde Blending Font Raw Data



#define DEF_OSDSAD2			            DEF_OSDSAD_LAYER3_4K
#define DEF_OSDMWI_4K                   0x0F
#define DEF_MPROTECT_4K                 ((DEF_WPEGBDBDSAD0 >> 16) - 2)  //H2PF_Simon_0116


////////////////////////////////////////////////////////////////////////////////////////////////
// Serial Flash
////////////////////////////////////////////////////////////////////////////////////////////////
#define DEF_SFL_OSD_FSAD_2K 			(0x00010000) // Font start address
#define DEF_SFL_OSD_PSAD_2K 			(0x00300000) // Bitmap start address
// logo replace addr = 0x00300000+0x00A00000=0x00D000000 => 0x01000000
#define DEF_SFL_OSD_2ND_LOGO    		(0x01000000) // logo replace size = 1920*1200 = 0x00232800 => 0x00240000
#define DEF_SFL_OSD_SERVICE_2ND_LOGO    (0x01240000) // service logo replace size 1920*1200



#endif  //_ICHIPS341MEMTABLE_H_

