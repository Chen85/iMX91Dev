/********************************************

    A35G2 Barco G62 Project Settings

*********************************************/

#define GLOBAL_ALLOCATE

#include "ProjectSettings.h"

//Enter Factory Mode Key
eKEY_LIST EnterFactoryModeKey[4] = {keBACK,
                                    keUP,
                                    keLEFT,
                                    keDOWN};

//Enter Service Mode Key
eKEY_LIST EnterServiceModeKey[6] = {keSTANDBY,
                                    keUP,
                                    keRIGHT,
                                    keUP,
                                    keLEFT,
                                    keMENU};

//Enter Dealer Mode Key
eKEY_LIST EnterDealerModeKey[4] = {keSTANDBY,
                                   keUP,
                                   keDOWN,
                                   keMENU};

//Service Code
UINT8 ServiceCode[4] = "1590";

sHSG_SETTING sCE_TABLE_SETTING_DEFAULT[] =
{
    //0 eCM_PICTURE_SETTINGS_PRESENTATION
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 1 eCM_PICTURE_SETTINGS_VIDEO
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 2 eCM_PICTURE_SETTINGS_BRIGHT
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
     // 3 eCM_PICTURE_SETTINGS_ENHANCE
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 4 eCM_PICTURE_SETTINGS_REC709
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 5 eCM_PICTURE_SETTINGS_REAL
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 6 eCM_PICTURE_SETTINGS_DICOMSIM
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 8 eCM_PICTURE_SETTINGS_3D
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 9 eCM_PICTURE_SETTINGS_BLENDING
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
	//10 eCM_PICTURE_SETTINGS_USER
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },

    // 11 eCM_PICTURE_SETTINGS_HDR
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 12 eCM_PICTURE_SETTINGS_SRGB
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 14 eCM_PICTURE_SETTINGS_SUPER_RED
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 15 eCM_PICTURE_SETTINGS_3D_PASSIVE
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
    // 16 eCM_PICTURE_SETTINGS_HLG
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127
    }
};

VERIFY_SIZE_OF(sCE_TABLE_SETTING_DEFAULT, sizeof(sCE_TABLE_SETTING_DEFAULT[0])*eCM_PICTURE_SETTINGS_NUMBER);


#if defined(PLATFORM_H60_2K) //HICC2_Doulas_0033  //H60

sWALL_COLOR_STRUCT sWallColorValues[][eCM_WALL_COLOR_NUMBER] =
{ //HICC2_Casper_0016
    {   // eCM_PICTURE_SETTINGS_PRESENTATION
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_VIDEO
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_BRIGHT
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_ENHANCED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_REC709
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_REAL
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_DICOMSIM
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_2D_HIGH_SPEED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_3D
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_BLENDING
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_USER
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_HDR
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SRGB
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SUPPER_BRIGHT
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SUPER_RED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_3D_PASSIVE
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_HLG
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        {       100,             88,             92         },  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    }
};

sHSG_SETTING sHSG_TABLE_SETTING_MODEL2[] =	//A70Gen2_Doulas_0019//A70Gen2_Doulas_0006 M0dify
{
       //0 eCM_PICTURE_SETTINGS_PRESENTATION => eIFC_PRESENTATION
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 1 eCM_PICTURE_SETTINGS_VIDEO
        {
			127, 127, 127,
			127, 127, 127,
			 89, 127, 127,
			 41, 127, 127,
			217, 123, 127,
			 71, 125, 127,
			127, 127, 127
        },
       // 2 eCM_PICTURE_SETTINGS_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 38, 127, 127,
			229, 123, 127,
			 57, 125, 127,
			127, 127, 127
        },
       // 3 eCM_PICTURE_SETTINGS_ENHANCE
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 36, 127, 127,
			229, 123, 127,
			 57, 125, 127,
			127, 127, 127
        },
       // 4 eCM_PICTURE_SETTINGS_REC709
        {
			127, 127, 127,
			114, 124, 127,
			 89, 127, 127,
			 41, 127, 127,
			217, 123, 127,
			 71, 125, 127,
			127, 127, 127
        },
       // 5 eCM_PICTURE_SETTINGS_REAL
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 6 eCM_PICTURE_SETTINGS_DICOMSIM
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 36, 127, 127,
			229, 123, 127,
			 57, 125, 127,
			127, 127, 127
        },
       // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 38, 127, 127,
			229, 123, 127,
			 57, 125, 127,
			127, 127, 127
        },
       // 8 eCM_PICTURE_SETTINGS_3D
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 89, 127, 127,
			197, 125, 127,
			 57, 125, 127,
			127, 127, 127
        },
       // 9 eCM_PICTURE_SETTINGS_BLENDING
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 34, 124, 127,
			231, 123, 127,
			 42, 124, 127,
			127, 127, 127
        },
		//10 eCM_PICTURE_SETTINGS_USER
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 38, 127, 127,
			229, 123, 127,
			 57, 125, 127,
			127, 127, 127
        },

       // 11 eCM_PICTURE_SETTINGS_HDR
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 36, 127, 127,
			229, 123, 127,
			 57, 125, 127,
			127, 127, 127
        },
       // 12 eCM_PICTURE_SETTINGS_SRGB
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 14 eCM_PICTURE_SETTINGS_SUPER_RED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        // 15 eCM_PICTURE_SETTINGS_3D_PASSIVE
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
};

//A35G2_Coda_0114
sHSG_SETTING sHSG_TABLE_SETTING_MODEL1[] =	//A70Gen2_Doulas_0019//A70Gen2_Doulas_0006 Modify
{
       //0 eCM_PICTURE_SETTINGS_PRESENTATION
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 1 eCM_PICTURE_SETTINGS_VIDEO
        {
            127,  127,  127,
            127,  127,  127,
             89,  127,  127,
             59,  126,  127,
            207,  118,  127,
             60,  117,  127,
            127,  127,  127
        },
       // 2 eCM_PICTURE_SETTINGS_BRIGHT
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
             41, 123, 127,
            227, 119, 127,
             55, 117, 127,
            127, 127, 127
        },
       // 3 eCM_PICTURE_SETTINGS_ENHANCE
        {
			127, 127, 127,
			127, 127, 127,
			 89, 127, 127,
			 42, 122, 127,
			226, 118, 127,
			 53, 118, 127,
			127, 127, 127
        },
       // 4 eCM_PICTURE_SETTINGS_REC709
        {
            127, 127, 127,
            140, 124, 127,
             89, 127, 127,
             59, 126, 127,
            207, 118, 127,
             60, 117, 127,
            127, 127, 127
        },
       // 5 eCM_PICTURE_SETTINGS_REAL
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 6 eCM_PICTURE_SETTINGS_DICOMSIM
        {
			127, 127, 127,
			127, 127, 127,
			 89, 127, 127,
			 42, 122, 127,
			226, 118, 127,
			 53, 118, 127,
			127, 127, 127
        },
       // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 41, 123, 127,
			227, 119, 127,
			 55, 117, 127,
			127, 127, 127
        },
       // 8 eCM_PICTURE_SETTINGS_3D
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 9 eCM_PICTURE_SETTINGS_BLENDING
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
		//10 eCM_PICTURE_SETTINGS_USER =
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
             41, 123, 127,
            227, 119, 127,
             55, 117, 127,
            127, 127, 127
        },
       // 11 eCM_PICTURE_SETTINGS_HDR
        {
			127, 127, 127,
			127, 127, 127,
			 89, 127, 127,
			 42, 122, 127,
			226, 118, 127,
			 53, 118, 127,
			127, 127, 127
        },
       // 12 eCM_PICTURE_SETTINGS_SRGB
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 14 eCM_PICTURE_SETTINGS_SUPER_RED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        // 15 eCM_PICTURE_SETTINGS_3D_PASSIVE
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
};

sHSG_SETTING sHSG_TABLE_SETTING_MODEL0[] =	//HICC2_Doulas_0074//HICC2_Doulas_0072 1800//A70Gen2_Doulas_0019//A70Gen2_Doulas_0006 Modify
{
       //0 eCM_PICTURE_SETTINGS_PRESENTATION
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 1 eCM_PICTURE_SETTINGS_VIDEO
        {
            127, 127, 127,
            127, 127, 127,
             89, 127, 127,
             59, 126, 127,
            207, 118, 127,
             60, 117, 127,
            127, 127, 127
        },
       // 2 eCM_PICTURE_SETTINGS_BRIGHT
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
             41, 123, 127,
            227, 119, 127,
             55, 117, 127,
            127, 127, 127
        },
       // 3 eCM_PICTURE_SETTINGS_ENHANCE
        {
            127, 127, 127,
            127, 127, 127,
             89, 127, 127,
             42, 122, 127,
            226, 118, 127,
             53, 118, 127,
            127, 127, 127
        },
       // 4 eCM_PICTURE_SETTINGS_REC709
        {
            127,  127,  127,
            140,  124,  127,
             89,  127,  127,
             59,  126,  127,
            207,  118,  127,
             60,  117,  127,
            127,  127,  127
        },
       // 5 eCM_PICTURE_SETTINGS_REAL
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 6 eCM_PICTURE_SETTINGS_DICOMSIM
        {
            127, 127, 127,
            127, 127, 127,
             89, 127, 127,
             42, 122, 127,
            226, 118, 127,
             53, 118, 127,
            127, 127, 127
        },
       // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 41, 123, 127,
			227, 119, 127,
			 55, 117, 127,
			127, 127, 127
        },
       // 8 eCM_PICTURE_SETTINGS_3D
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 9 eCM_PICTURE_SETTINGS_BLENDING
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
		//10 eCM_PICTURE_SETTINGS_USER
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
             41, 123, 127,
            227, 119, 127,
             55, 117, 127,
            127, 127, 127
        },
       // 11 eCM_PICTURE_SETTINGS_HDR
        {
            127, 127, 127,
            127, 127, 127,
             89, 127, 127,
             42, 122, 127,
            226, 118, 127,
             53, 118, 127,
            127, 127, 127
        },
       // 12 eCM_PICTURE_SETTINGS_SRGB
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 14 eCM_PICTURE_SETTINGS_SUPER_RED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        // 15 eCM_PICTURE_SETTINGS_3D_PASSIVE
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
};


#elif defined(PLATFORM_A70LK)  //HICC2_Doulas_0100// A70LK   ///////////////////////////////////////////////////////////////////////////////////////////////////

sWALL_COLOR_STRUCT sWallColorValues[eCM_PICTURE_SETTINGS_NUMBER][eCM_WALL_COLOR_NUMBER] =
{ //HICC2_Casper_0016
    {   // eCM_PICTURE_SETTINGS_PRESENTATION
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_VIDEO
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_BRIGHT
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_ENHANCED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_REC709
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_REAL
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_DICOMSIM
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_2D_HIGH_SPEED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_3D
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_BLENDING
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_USER
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_HDR
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SRGB
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SUPPER_BRIGHT
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SUPER_RED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_HLG
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        {       100,             88,             92         },  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    }
};

//G100_Doulas_0080//A35G2_Coda_0124 //A35G2_BRC_Casper_0127 //A35G2_Jacky_0012
sHSG_SETTING sHSG_TABLE_SETTING_MODEL2[eCM_PICTURE_SETTINGS_NUMBER] =
{
       //0 eCM_PICTURE_SETTINGS_PRESENTATION => eIFC_PRESENTATION
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            183, 115, 127,
            54,  119, 127,
            127, 127, 127
        },
       // 1 eCM_PICTURE_SETTINGS_VIDEO => eIFC_CINEMA
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            179, 127, 127,
            66,  111, 127,
            78,  115, 127,
            127, 127, 127
        },
       // 2 eCM_PICTURE_SETTINGS_BRIGHT => eIFC_BRIGHT
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            183, 115, 127,
            54,  119, 127,
            127, 127, 127
        },
       // 3 eCM_PICTURE_SETTINGS_ENHANCE
        {
            127, 127, 137,
            127, 127, 137,
            127, 127, 137,
            127, 127, 127,
            186, 116, 127,
            54,  119, 127,
            127, 127, 127
        },
       // 4 eCM_PICTURE_SETTINGS_REC709
        {
            110, 127, 127,
            111, 119, 127,
            100, 129, 127,
            167, 127, 127,
            87,  110, 127,
            72,  116, 127,
            127, 127, 127
        },
       // 5 eCM_PICTURE_SETTINGS_REAL
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 6 eCM_PICTURE_SETTINGS_DICOMSIM => eIFC_DICOMSIM
        {
            127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED => eIFC_2D_HIGH_SPEED
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            104, 127, 127,
            186, 114, 127,
            59,  119, 127,
            127, 127, 127
        },
       // 8 eCM_PICTURE_SETTINGS_3D => eIFC_3D
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            120, 127, 127,
            190, 115, 127,
            53,  121, 127,
            127, 127, 127
        },
       // 9 eCM_PICTURE_SETTINGS_BLENDING => eIFC_BLENDING
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            104, 127, 127,
            186, 114, 127,
            59,  119, 127,
            127, 127, 127
        },
		//10 eCM_PICTURE_SETTINGS_USER => eIFC_USER
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            183, 115, 127,
            54,  119, 127,
            127, 127, 127
        },

       // 11 eCM_PICTURE_SETTINGS_HDR => eIFC_HDR
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            104, 127, 127,
            186, 114, 127,
            59,  119, 127,
            127, 127, 127
        },
       // 12 eCM_PICTURE_SETTINGS_SRGB => eIFC_sRGB
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 14 eCM_PICTURE_SETTINGS_SUPER_RED => eIFC_SUPER_RED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
};

//A35G2_Coda_0114 //A35G2_Jacky_0012 //A35G2_Alan_0016
sHSG_SETTING sHSG_TABLE_SETTING_MODEL1[eCM_PICTURE_SETTINGS_NUMBER] =
{
       //0 eCM_PICTURE_SETTINGS_PRESENTATION => eIFC_PRESENTATION
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            183, 115, 127,
            54,  119, 127,
            127, 127, 127
        },
       // 1 eCM_PICTURE_SETTINGS_VIDEO => eIFC_CINEMA
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            179, 127, 127,
            66,  111, 127,
            78,  115, 127,
            127, 127, 127
        },
       // 2 eCM_PICTURE_SETTINGS_BRIGHT => eIFC_BRIGHT
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            183, 115, 127,
            54,  119, 127,
            127, 127, 127
        },
       // 3 eCM_PICTURE_SETTINGS_ENHANCE
        {
            127, 127, 137,
            127, 127, 137,
            127, 127, 137,
            127, 127, 127,
            186, 116, 127,
            54,  119, 127,
            127, 127, 127
        },
       // 4 eCM_PICTURE_SETTINGS_REC709
        {
            110, 127, 127,
            111, 119, 127,
            100, 129, 127,
            167, 127, 127,
            87,  110, 127,
            72,  116, 127,
            127, 127, 127
        },
       // 5 eCM_PICTURE_SETTINGS_REAL
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 6 eCM_PICTURE_SETTINGS_DICOMSIM => eIFC_DICOMSIM
        {
            127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED => eIFC_2D_HIGH_SPEED
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            104, 127, 127,
            186, 114, 127,
            59,  119, 127,
            127, 127, 127
        },
       // 8 eCM_PICTURE_SETTINGS_3D => eIFC_3D
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            120, 127, 127,
            190, 115, 127,
            53,  121, 127,
            127, 127, 127
        },
       // 9 eCM_PICTURE_SETTINGS_BLENDING => eIFC_BLENDING
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            104, 127, 127,
            186, 114, 127,
            59,  119, 127,
            127, 127, 127
        },
		//10 eCM_PICTURE_SETTINGS_USER => eIFC_USER
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            183, 115, 127,
            54,  119, 127,
            127, 127, 127
        },

       // 11 eCM_PICTURE_SETTINGS_HDR => eIFC_HDR
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            104, 127, 127,
            186, 114, 127,
            59,  119, 127,
            127, 127, 127
        },
       // 12 eCM_PICTURE_SETTINGS_SRGB => eIFC_sRGB
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 14 eCM_PICTURE_SETTINGS_SUPER_RED => eIFC_SUPER_RED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
};

sHSG_SETTING sHSG_TABLE_SETTING_MODEL0[eCM_PICTURE_SETTINGS_NUMBER] =
{
       //0 eCM_PICTURE_SETTINGS_PRESENTATION => eIFC_PRESENTATION
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            183, 115, 127,
            54,  119, 127,
            127, 127, 127
        },
       // 1 eCM_PICTURE_SETTINGS_VIDEO => eIFC_CINEMA
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            179, 127, 127,
            66,  111, 127,
            78,  115, 127,
            127, 127, 127
        },
       // 2 eCM_PICTURE_SETTINGS_BRIGHT => eIFC_BRIGHT
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            183, 115, 127,
            54,  119, 127,
            127, 127, 127
        },
       // 3 eCM_PICTURE_SETTINGS_ENHANCE
        {
            127, 127, 137,
            127, 127, 137,
            127, 127, 137,
            127, 127, 127,
            186, 116, 127,
            54,  119, 127,
            127, 127, 127
        },
       // 4 eCM_PICTURE_SETTINGS_REC709
        {
            110, 127, 127,
            111, 119, 127,
            100, 129, 127,
            167, 127, 127,
            87,  110, 127,
            72,  116, 127,
            127, 127, 127
        },
       // 5 eCM_PICTURE_SETTINGS_REAL
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 6 eCM_PICTURE_SETTINGS_DICOMSIM => eIFC_DICOMSIM
        {
            127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED => eIFC_2D_HIGH_SPEED
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            104, 127, 127,
            186, 114, 127,
            59,  119, 127,
            127, 127, 127
        },
       // 8 eCM_PICTURE_SETTINGS_3D => eIFC_3D
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            120, 127, 127,
            190, 115, 127,
            53,  121, 127,
            127, 127, 127
        },
       // 9 eCM_PICTURE_SETTINGS_BLENDING => eIFC_BLENDING
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            104, 127, 127,
            186, 114, 127,
            59,  119, 127,
            127, 127, 127
        },
		//10 eCM_PICTURE_SETTINGS_USER => eIFC_USER
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            183, 115, 127,
            54,  119, 127,
            127, 127, 127
        },

       // 11 eCM_PICTURE_SETTINGS_HDR => eIFC_HDR
        {
            127, 127, 127,
            127, 127, 127,
            127, 127, 127,
            104, 127, 127,
            186, 114, 127,
            59,  119, 127,
            127, 127, 127
        },
       // 12 eCM_PICTURE_SETTINGS_SRGB => eIFC_sRGB
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 14 eCM_PICTURE_SETTINGS_SUPER_RED => eIFC_SUPER_RED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
};

sHSG_SETTING sCE_TABLE_SETTING_W16[eGUI_PICTURE_SETTINGS_NUMBER][eCOLOR_ENHANCEMENT_MAX_NUMBER] =   	//G100_Clare_0015
{
	//Order follow GUI
	//G100_Clare_0021, modify, >>>
    {   //0 eIFC_VIDEO
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //1 eIFC_BRIGHT
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //3 eIFC_ENHANCED
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //5 eIFC_RE709
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //6 eIFC_DICOMSIM
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //7 eIFC_BLENDING
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //10 eIFC_USER
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //4 eIFC_HDR
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //8 eIFC_3D
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //9 eIFC_2D_HIGH_SPEED
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    }
	//G100_Clare_0021, modify, <<<
};

sHSG_SETTING sCE_TABLE_SETTING_W20[eGUI_PICTURE_SETTINGS_NUMBER][eCOLOR_ENHANCEMENT_MAX_NUMBER] =	//G100_Clare_0014
{
	//Order follow GUI
	//G100_Clare_0021, modify, >>>
    {   //0 eIFC_VIDEO
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //1 eIFC_BRIGHT
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //3 eIFC_ENHANCED
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //5 eIFC_RE709
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //6 eIFC_DICOMSIM
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //7 eIFC_BLENDING
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //10 eIFC_USER
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //4 eIFC_HDR
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //8 eIFC_3D
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //9 eIFC_2D_HIGH_SPEED
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    }
	//G100_Clare_0021, modify, <<<
};

#elif defined(PLATFORM_H60_4K)  //HICC2_Doulas_0100    ///////////////////////////////////////////////////////////////////////////////////////////////////

sWALL_COLOR_STRUCT sWallColorValues[][eCM_WALL_COLOR_NUMBER] =
{ //HICC2_Casper_0016
    {   // eCM_PICTURE_SETTINGS_PRESENTATION
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_VIDEO
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_BRIGHT
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_ENHANCED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_REC709
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_REAL
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_DICOMSIM
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_2D_HIGH_SPEED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_3D
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_BLENDING
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_USER
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_HDR
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SRGB
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SUPPER_BRIGHT
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SUPER_RED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_3D_PASSIVE
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_HLG
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        {       100,             88,             92         },  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    }
};

//G100_Doulas_0080//A35G2_Coda_0124 //A35G2_BRC_Casper_0127 //A35G2_Jacky_0012
//#define MODULE_TYPE_ID0_PLATFORM      (MODULE_4K14_PLATFORM)
//#define MODULE_TYPE_ID1_PLATFORM      (MODULE_4K21_PLATFORM)
//#define MODULE_TYPE_ID2_PLATFORM      (MODULE_4K16_PLATFORM) //reserve

sHSG_SETTING sHSG_TABLE_SETTING_MODEL2[] =
{
   //0 eCM_PICTURE_SETTINGS_PRESENTATION => eIFC_PRESENTATION
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 1 eCM_PICTURE_SETTINGS_VIDEO => eIFC_CINEMA
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        179, 127, 127,
         66, 111, 127,
         78, 115, 127,
        127, 127, 127
    },
   // 2 eCM_PICTURE_SETTINGS_BRIGHT => eIFC_BRIGHT
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        183, 115, 127,
         54, 119, 127,
        127, 127, 127
    },
   // 3 eCM_PICTURE_SETTINGS_ENHANCE
    {
        127, 127, 137,
        127, 127, 137,
        127, 127, 137,
        127, 127, 127,
        186, 116, 127,
        54,  119, 127,
        127, 127, 127,
    },
   // 4 eCM_PICTURE_SETTINGS_REC709
    {
        110, 127, 127,
        111, 119, 127,
        100, 129, 127,
        167, 127, 127,
        87 , 110, 127,
        72 , 116, 127,
        127, 127, 127
    },
   // 5 eCM_PICTURE_SETTINGS_REAL
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 6 eCM_PICTURE_SETTINGS_DICOMSIM => eIFC_DICOMSIM
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED => eIFC_2D_HIGH_SPEED
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        104, 127, 127,
        186, 114, 127,
         59, 119, 127,
        127, 127, 127,
    },
   // 8 eCM_PICTURE_SETTINGS_3D => eIFC_3D
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        120, 127, 127,
        190, 115, 127,
         53, 121, 127,
        127, 127, 127
    },
   // 9 eCM_PICTURE_SETTINGS_BLENDING => eIFC_BLENDING
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
	//10 eCM_PICTURE_SETTINGS_USER => eIFC_USER
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		 64, 191, 127,
		 64, 114, 127,
		216, 114, 127,
		127, 127, 127
    },

   // 11 eCM_PICTURE_SETTINGS_HDR => eIFC_HDR
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		 64, 140, 127,
		 64, 114, 127,
		211, 114, 127,
		127, 127, 127
    },
   // 12 eCM_PICTURE_SETTINGS_SRGB => eIFC_sRGB
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 14 eCM_PICTURE_SETTINGS_SUPER_RED => eIFC_SUPER_RED
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 15 eCM_PICTURE_SETTINGS_3D_PASSIVE
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        120, 127, 127,
        190, 115, 127,
         53, 121, 127,
        127, 127, 127
    }
};

//A35G2_Coda_0114 //A35G2_Jacky_0012 //A35G2_Alan_0016
sHSG_SETTING sHSG_TABLE_SETTING_MODEL1[] =
{
   //0 eCM_PICTURE_SETTINGS_PRESENTATION
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 1 eCM_PICTURE_SETTINGS_VIDEO
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        92 , 127, 127,
        174, 112, 127,
        77 , 114, 127,
        127, 127, 127
    },
   // 2 eCM_PICTURE_SETTINGS_BRIGHT
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        57 , 114, 127,
        216, 114, 127,
        57 , 114, 127,
        127, 127, 127
    },
   // 3 eCM_PICTURE_SETTINGS_ENHANCE
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        60 , 127, 127,
        210, 116, 127,
        64 , 114, 127,
        127, 127, 127
    },
   // 4 eCM_PICTURE_SETTINGS_REC709
    {
        113, 127, 127,
        146, 124, 127,
        99 , 127, 127,
        92 , 127, 127,
        174, 112, 127,
        77 , 114, 127,
        127, 127, 127
    },
   // 5 eCM_PICTURE_SETTINGS_REAL
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 6 eCM_PICTURE_SETTINGS_DICOMSIM
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        60 , 127, 127,
        210, 116, 127,
        64 , 114, 127,
        127, 127, 127
    },
   // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        60 , 127, 127,
        208, 116, 127,
        64 , 114, 127,
        127, 127, 127
    },
   // 8 eCM_PICTURE_SETTINGS_3D
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        100, 127, 127,
        199, 118, 127,
        55 , 118, 127,
        127, 127, 127
    },
   // 9 eCM_PICTURE_SETTINGS_BLENDING
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        80 , 127, 127,
        197, 114, 127,
        64 , 118, 127,
        127, 127, 127
    },
	//10 eCM_PICTURE_SETTINGS_USER
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		 64, 191, 127,
		 64, 114, 127,
		216, 114, 127,
		127, 127, 127
    },

   // 11 eCM_PICTURE_SETTINGS_HDR
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		60 , 127, 127,
		210, 116, 127,
		64 , 114, 127,
		127, 127, 127
    },
   // 12 eCM_PICTURE_SETTINGS_SRGB
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 14 eCM_PICTURE_SETTINGS_SUPER_RED
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 15 eCM_PICTURE_SETTINGS_3D_PASSIVE
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        90 , 127, 127,
        196, 113, 127,
        57 , 118, 127,
        127, 127, 127
    }
};

sHSG_SETTING sHSG_TABLE_SETTING_MODEL0[] =
{
   //0 eCM_PICTURE_SETTINGS_PRESENTATION => eIFC_PRESENTATION
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 1 eCM_PICTURE_SETTINGS_VIDEO => eIFC_CINEMA
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        77 , 127, 127,
        186, 116, 127,
        73 , 120, 127,
        127, 127, 127
    },
   // 2 eCM_PICTURE_SETTINGS_BRIGHT => eIFC_BRIGHT
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        50 , 119, 127,
        217, 116, 127,
        59 , 119, 127,
        127, 127, 127
    },
   // 3 eCM_PICTURE_SETTINGS_ENHANCE
    {
        127, 127, 127,
        127, 127, 127,
        92 , 127, 137,
        59 , 126, 127,
        210, 119, 127,
        61 , 117, 127,
        127, 127, 127,
    },
   // 4 eCM_PICTURE_SETTINGS_REC709
    {
        113, 127, 127,
        146, 125, 127,
        89 , 129, 127,
        77 , 127, 127,
        186, 116, 127,
        73 , 120, 127,
        127, 127, 127
    },
   // 5 eCM_PICTURE_SETTINGS_REAL
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 6 eCM_PICTURE_SETTINGS_DICOMSIM => eIFC_DICOMSIM
    {
		127, 127, 127,
		127, 127, 127,
		92 , 127, 127,
		59 , 126, 127,
		210, 119, 127,
		61 , 117, 127,
		127, 127, 127
    },
   // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED => eIFC_2D_HIGH_SPEED
    {
        127, 127, 127,
        127, 127, 127,
        92 , 127, 127,
        59 , 126, 127,
        210, 119, 127,
        61 , 117, 127,
        127, 127, 127,
    },
   // 8 eCM_PICTURE_SETTINGS_3D => eIFC_3D
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        99 , 127, 127,
        192, 115, 127,
        59 , 121, 127,
        127, 127, 127
    },
   // 9 eCM_PICTURE_SETTINGS_BLENDING => eIFC_BLENDING
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		94 , 127, 127,
		190, 118, 127,
		61 , 122, 127,
		127, 127, 127
    },
	//10 eCM_PICTURE_SETTINGS_USER => eIFC_USER
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		 64, 191, 127,
		 64, 114, 127,
		216, 114, 127,
		127, 127, 127
    },

   // 11 eCM_PICTURE_SETTINGS_HDR => eIFC_HDR
    {
		127, 127, 127,
		127, 127, 127,
		92 , 127, 127,
		59 , 126, 127,
		210, 119, 127,
		61 , 117, 127,
		127, 127, 127
    },
   // 12 eCM_PICTURE_SETTINGS_SRGB => eIFC_sRGB
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 14 eCM_PICTURE_SETTINGS_SUPER_RED => eIFC_SUPER_RED
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 15 eCM_PICTURE_SETTINGS_3D_PASSIVE
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        103, 127, 127,
        195, 115, 127,
        54 , 121, 127,
        127, 127, 127
    }
};

#if 0
sHSG_SETTING sCE_TABLE_SETTING_W16[eGUI_PICTURE_SETTINGS_NUMBER][eCOLOR_ENHANCEMENT_MAX_NUMBER] =
{
	//Order follow GUI
	//G100_Clare_0021, modify, >>>
    {   //0 eIFC_VIDEO
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //1 eIFC_BRIGHT
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //3 eIFC_ENHANCED
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //5 eIFC_RE709
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //6 eIFC_DICOMSIM
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //7 eIFC_BLENDING
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //10 eIFC_USER
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //4 eIFC_HDR
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //8 eIFC_3D
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //9 eIFC_2D_HIGH_SPEED
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    }
	//G100_Clare_0021, modify, <<<
};

sHSG_SETTING sCE_TABLE_SETTING_W20[eGUI_PICTURE_SETTINGS_NUMBER][eCOLOR_ENHANCEMENT_MAX_NUMBER] =	//G100_Clare_0014
{
	//Order follow GUI
	//G100_Clare_0021, modify, >>>
    {   //0 eIFC_VIDEO
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //1 eIFC_BRIGHT
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //3 eIFC_ENHANCED
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //5 eIFC_RE709
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //6 eIFC_DICOMSIM
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //7 eIFC_BLENDING
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //10 eIFC_USER
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //4 eIFC_HDR
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //8 eIFC_3D
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //9 eIFC_2D_HIGH_SPEED
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    }
	//G100_Clare_0021, modify, <<<
};
#endif /* 0 */

#elif defined(PLATFORM_H30_4K)

sWALL_COLOR_STRUCT sWallColorValues[][eCM_WALL_COLOR_NUMBER] =
{ //HICC2_Casper_0016
    {   // eCM_PICTURE_SETTINGS_PRESENTATION
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_VIDEO
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_BRIGHT
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_ENHANCED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_REC709
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_REAL
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_DICOMSIM
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_2D_HIGH_SPEED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_3D
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_BLENDING
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_USER
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_HDR
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SRGB
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SUPPER_BRIGHT
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SUPER_RED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_3D_PASSIVE
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_HLG
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        {       100,             88,             92         },  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             93,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    }
};

//G100_Doulas_0080//A35G2_Coda_0124 //A35G2_BRC_Casper_0127 //A35G2_Jacky_0012
//#define MODULE_TYPE_ID0_PLATFORM      (MODULE_4K14_PLATFORM)
//#define MODULE_TYPE_ID1_PLATFORM      (MODULE_4K21_PLATFORM)
//#define MODULE_TYPE_ID2_PLATFORM      (MODULE_4K16_PLATFORM) //reserve

sHSG_SETTING sHSG_TABLE_SETTING_MODEL2[] =
{
   //0 eCM_PICTURE_SETTINGS_PRESENTATION => eIFC_PRESENTATION
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 1 eCM_PICTURE_SETTINGS_VIDEO => eIFC_CINEMA
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        179, 127, 127,
         66, 111, 127,
         78, 115, 127,
        127, 127, 127
    },
   // 2 eCM_PICTURE_SETTINGS_BRIGHT => eIFC_BRIGHT
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        183, 115, 127,
         54, 119, 127,
        127, 127, 127
    },
   // 3 eCM_PICTURE_SETTINGS_ENHANCE
    {
        127, 127, 137,
        127, 127, 137,
        127, 127, 137,
        127, 127, 127,
        186, 116, 127,
        54,  119, 127,
        127, 127, 127,
    },
   // 4 eCM_PICTURE_SETTINGS_REC709
    {
        110, 127, 127,
        111, 119, 127,
        100, 129, 127,
        167, 127, 127,
        87 , 110, 127,
        72 , 116, 127,
        127, 127, 127
    },
   // 5 eCM_PICTURE_SETTINGS_REAL
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 6 eCM_PICTURE_SETTINGS_DICOMSIM => eIFC_DICOMSIM
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED => eIFC_2D_HIGH_SPEED
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        104, 127, 127,
        186, 114, 127,
         59, 119, 127,
        127, 127, 127,
    },
   // 8 eCM_PICTURE_SETTINGS_3D => eIFC_3D
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        120, 127, 127,
        190, 115, 127,
         53, 121, 127,
        127, 127, 127
    },
   // 9 eCM_PICTURE_SETTINGS_BLENDING => eIFC_BLENDING
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
	//10 eCM_PICTURE_SETTINGS_USER => eIFC_USER
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		 64, 191, 127,
		 64, 114, 127,
		216, 114, 127,
		127, 127, 127
    },

   // 11 eCM_PICTURE_SETTINGS_HDR => eIFC_HDR
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		 64, 140, 127,
		 64, 114, 127,
		211, 114, 127,
		127, 127, 127
    },
   // 12 eCM_PICTURE_SETTINGS_SRGB => eIFC_sRGB
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 14 eCM_PICTURE_SETTINGS_SUPER_RED => eIFC_SUPER_RED
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 15 eCM_PICTURE_SETTINGS_3D_PASSIVE
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        120, 127, 127,
        190, 115, 127,
         53, 121, 127,
        127, 127, 127
    },
   // 16 eCM_PICTURE_SETTINGS_HLG
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
};

//A35G2_Coda_0114 //A35G2_Jacky_0012 //A35G2_Alan_0016
sHSG_SETTING sHSG_TABLE_SETTING_MODEL1[] =
{
   //0 eCM_PICTURE_SETTINGS_PRESENTATION
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 1 eCM_PICTURE_SETTINGS_VIDEO
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        164, 119, 127,
        182, 110, 127,
        72 , 114, 127,
        127, 127, 127
    },
   // 2 eCM_PICTURE_SETTINGS_BRIGHT
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 3 eCM_PICTURE_SETTINGS_ENHANCE
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        60 , 127, 127,
        210, 116, 127,
        64 , 114, 127,
        127, 127, 127
    },
   // 4 eCM_PICTURE_SETTINGS_REC709
    {
        110, 127, 127,
        111, 119, 127,
        100, 129, 127,
        167, 127, 127,
        87 , 110, 127,
        72 , 116, 127,
        127, 127, 127
    },
   // 5 eCM_PICTURE_SETTINGS_REAL
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 6 eCM_PICTURE_SETTINGS_DICOMSIM
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        60 , 127, 127,
        210, 116, 127,
        64 , 114, 127,
        127, 127, 127
    },
   // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        60 , 127, 127,
        208, 116, 127,
        64 , 114, 127,
        127, 127, 127
    },
   // 8 eCM_PICTURE_SETTINGS_3D
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		90 , 112, 127,
		190, 114, 127,
		58 , 121, 127,
		127, 127, 127
    },
   // 9 eCM_PICTURE_SETTINGS_BLENDING
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        80 , 127, 127,
        197, 114, 127,
        64 , 118, 127,
        127, 127, 127
    },
	//10 eCM_PICTURE_SETTINGS_USER
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		 64, 191, 127,
		 64, 114, 127,
		216, 114, 127,
		127, 127, 127
    },

   // 11 eCM_PICTURE_SETTINGS_HDR
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		80 , 127, 127,
		197, 114, 127,
		64 , 118, 127,
		127, 127, 127
    },
   // 12 eCM_PICTURE_SETTINGS_SRGB
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 14 eCM_PICTURE_SETTINGS_SUPER_RED
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 15 eCM_PICTURE_SETTINGS_3D_PASSIVE
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127
    },
   // 16 eCM_PICTURE_SETTINGS_HLG
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
};

sHSG_SETTING sHSG_TABLE_SETTING_MODEL0[] =
{
   //0 eCM_PICTURE_SETTINGS_PRESENTATION => eIFC_PRESENTATION
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 1 eCM_PICTURE_SETTINGS_VIDEO => eIFC_CINEMA
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        80 , 127, 127,
        175, 112, 127,
        77 , 114, 127,
        127, 127, 127
    },
   // 2 eCM_PICTURE_SETTINGS_BRIGHT => eIFC_BRIGHT
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 3 eCM_PICTURE_SETTINGS_ENHANCE
    {
        127, 127, 127,
        127, 127, 127,
        89 , 127, 137,
        55 , 127, 127,
        210, 116, 127,
        64,  116, 127,
        127, 127, 127,
    },
   // 4 eCM_PICTURE_SETTINGS_REC709
    {
        116, 127, 127,
        127, 122, 127,
        91 , 127, 127,
        55 , 127, 127,
        210, 116, 127,
        64 , 116, 127,
        127, 127, 127
    },
   // 5 eCM_PICTURE_SETTINGS_REAL
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 6 eCM_PICTURE_SETTINGS_DICOMSIM => eIFC_DICOMSIM
    {
		127, 127, 127,
		127, 127, 127,
		89 , 127, 127,
		55 , 127, 127,
		210, 116, 127,
		64 , 116, 127,
		127, 127, 127
    },
   // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED => eIFC_2D_HIGH_SPEED
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        58 , 165, 127,
        212, 114, 127,
        64 , 114, 127,
        127, 127, 127,
    },
   // 8 eCM_PICTURE_SETTINGS_3D => eIFC_3D
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		90 , 112, 127,
		190, 114, 127,
		58 , 121, 127,
		127, 127, 127
    },
   // 9 eCM_PICTURE_SETTINGS_BLENDING => eIFC_BLENDING
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		0  , 127, 127
    },
	//10 eCM_PICTURE_SETTINGS_USER => eIFC_USER
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		 64, 191, 127,
		 64, 114, 127,
		216, 114, 127,
		127, 127, 127
    },

   // 11 eCM_PICTURE_SETTINGS_HDR => eIFC_HDR
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		0  , 127, 127
    },
   // 12 eCM_PICTURE_SETTINGS_SRGB => eIFC_sRGB
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 14 eCM_PICTURE_SETTINGS_SUPER_RED => eIFC_SUPER_RED
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
   // 15 eCM_PICTURE_SETTINGS_3D_PASSIVE
    {
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        127, 127, 127,
        0  , 127, 127
    },
   // 16 eCM_PICTURE_SETTINGS_HLG
    {
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127,
		127, 127, 127
    },
};


#else
sWALL_COLOR_STRUCT sWallColorValues[eCM_PICTURE_SETTINGS_NUMBER][eCM_WALL_COLOR_NUMBER] =
{ //HICC2_Casper_0016
    {   // eCM_PICTURE_SETTINGS_PRESENTATION
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_VIDEO
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_BRIGHT
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_ENHANCED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_REC709
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_REAL
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_DICOMSIM
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_2D_HIGH_SPEED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_3D
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_BLENDING
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_USER
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_HDR
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SRGB
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SUPPER_BRIGHT
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_SUPER_RED
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        { 		100,			 88,			 92 		},  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    },
    {   // eCM_PICTURE_SETTINGS_HLG
        //  CSC R Gain,     CSC G Gain,     CSC B Gain      // 0~200
        {       100,            100,            100         },  // White
        {       100,             88,             92         },  // Gray130
        {        94,             94,            100         },  // Light Yellow
        {       100,            100,            100         },  // Off
        {        85,             85,             85         },  // Blackboard
        {       100,             88,             92         },  // Light Green
        {       100,             97,             95         },  // Light Blue
        {        90,            100,             97         },  // Pink
        {       100,             88,             92         },  // Gray
        {       100,            100,            100         },  // Auto
    }
};

//H2 wait review (eGUI_PICTURE_SETTINGS_NUMBER)
sHSG_SETTING sCE_TABLE_SETTING_W16[eGUI_PICTURE_SETTINGS_NUMBER][eCOLOR_ENHANCEMENT_MAX_NUMBER] =   	//G100_Clare_0015 //A35G2_BRC_Casper_0012
{
    {   //0 eIFC_PRESENTATION
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //1 eIFC_BRIGHT
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //2 eIFC_CINEMA
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //3 eIFC_HDR
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //5 eIFC_sRGB
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //6 eIFC_DICOMSIM
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //7 eIFC_BLENDING
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //8 eIFC_3D
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //9 eIFC_2D_HIGH_SPEED
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //10 eIFC_USER
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    }
};


//H2 wait review (eGUI_PICTURE_SETTINGS_NUMBER)
sHSG_SETTING sCE_TABLE_SETTING_W20[eGUI_PICTURE_SETTINGS_NUMBER][eCOLOR_ENHANCEMENT_MAX_NUMBER] =	//G100_Clare_0014 //A35G2_BRC_Casper_0012
{
    {   //0 eIFC_PRESENTATION
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //1 eIFC_BRIGHT
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //3 eIFC_CINEMA
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //4 eIFC_HDR
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //5 eIFC_sRGB
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //6 eIFC_DICOMSIM
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //7 eIFC_BLENDING
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //8 eIFC_3D
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }

    },
    {   //9 eIFC_2D_HIGH_SPEED
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    },
    {   //10 eIFC_USER
        {   // Off
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE1
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
        {   // CE2
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
    }
};

//////////////////////////////////////////////////////     HSG        ///////////////////////
sHSG_SETTING sHSG_TABLE_SETTING_MODEL2[eCM_PICTURE_SETTINGS_NUMBER] =
{
       //0 eCM_PICTURE_SETTINGS_PRESENTATION
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 34, 127, 127,
			231, 120, 127,
			 57, 119, 127,
			127, 127, 127
        },
       // 1 eCM_PICTURE_SETTINGS_VIDEO
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 53, 127, 127,
			210, 114, 127,
			 58, 118, 127,
			127, 127, 127
        },
       // 2 eCM_PICTURE_SETTINGS_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 38, 127, 127,
			229, 119, 127,
			 55, 120, 127,
			127, 127, 127
        },
       // 3 eCM_PICTURE_SETTINGS_ENHANCED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 37, 127, 127,
			231, 127, 127,
			 60, 127, 127,
			127, 127, 127
        },
       // 4 eCM_PICTURE_SETTINGS_REC709
        {
			119, 127, 127,
			130, 124, 127,
			 87, 127, 127,
			 50, 127, 127,
			212, 114, 127,
			 63, 117, 127,
			127, 127, 127
        },
       // 5 eCM_PICTURE_SETTINGS_REAL
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 6 eCM_PICTURE_SETTINGS_DICOMSIM
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 34, 127, 127,
			231, 120, 127,
			 57, 119, 127,
			127, 127, 127
        },
       // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 36, 127, 127,
			231, 120, 127,
			 57, 119, 127,
			127, 127, 127
        },
       // 8 eCM_PICTURE_SETTINGS_3D
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 90, 127, 127,
			195, 116, 127,
			 56, 120, 127,
			127, 127, 127
        },
       // 9 eCM_PICTURE_SETTINGS_BLENDING
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
		//10 eCM_PICTURE_SETTINGS_USER
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 38, 127, 127,
			229, 119, 127,
			 55, 120, 127,
			127, 127, 127
        },

       // 11 eCM_PICTURE_SETTINGS_HDR
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 34, 127, 127,
			231, 120, 127,
			 57, 119, 127,
			127, 127, 127
        },
       // 12 eCM_PICTURE_SETTINGS_SRGB
        {
			119, 127, 127,
			130, 124, 127,
			 87, 127, 127,
			 50, 127, 127,
			212, 114, 127,
			 63, 117, 127,
			127, 127, 127
        },
       // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 14 eCM_PICTURE_SETTINGS_SUPER_RED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
};

sHSG_SETTING sHSG_TABLE_SETTING_MODEL1[eCM_PICTURE_SETTINGS_NUMBER] =
{ //A35G2_BRC_Casper_0079 //A35G2_BRC_Casper_0085 //A35G2_BRC_Casper_0128 //A35G2_BRC_Casper_0129
       //0 eCM_PICTURE_SETTINGS_PRESENTATION
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 34, 127, 127,
			231, 120, 127,
			 57, 119, 127,
			127, 127, 127
        },
       // 1 eCM_PICTURE_SETTINGS_VIDEO
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 53, 127, 127,
			210, 114, 127,
			 58, 118, 127,
			127, 127, 127
        },
       // 2 eCM_PICTURE_SETTINGS_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 38, 127, 127,
			229, 119, 127,
			 55, 120, 127,
			127, 127, 127
        },
       // 3 eCM_PICTURE_SETTINGS_ENHANCED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 37, 127, 127,
			231, 127, 127,
			 60, 127, 127,
			127, 127, 127
        },
       // 4 eCM_PICTURE_SETTINGS_REC709
        {
			119, 127, 127,
			130, 124, 127,
			127, 127, 127,
			 50, 127, 127,
			212, 114, 127,
			 63, 117, 127,
			127, 127, 127
        },
       // 5 eCM_PICTURE_SETTINGS_REAL
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 6 eCM_PICTURE_SETTINGS_DICOMSIM
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 34, 127, 127,
			231, 120, 127,
			 57, 119, 127,
			127, 127, 127
        },
       // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 36, 127, 127,
			230, 117, 127,
			 58, 118, 127,
			127, 127, 127
        },
       // 8 eCM_PICTURE_SETTINGS_3D
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 90, 127, 127,
			195, 116, 127,
			 56, 120, 127,
			127, 127, 127
        },
       // 9 eCM_PICTURE_SETTINGS_BLENDING
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
		//10 eCM_PICTURE_SETTINGS_USER
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 38, 127, 127,
			229, 119, 127,
			 55, 120, 127,
			127, 127, 127
        },

       // 11 eCM_PICTURE_SETTINGS_HDR
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 34, 127, 127,
			231, 120, 127,
			 57, 119, 127,
			127, 127, 127
        },
       // 12 eCM_PICTURE_SETTINGS_SRGB
        {
			119, 127, 127,
			130, 124, 127,
			127, 127, 127,
			 50, 127, 127,
			212, 114, 127,
			 63, 117, 127,
			127, 127, 127
        },
       // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 14 eCM_PICTURE_SETTINGS_SUPER_RED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
};

sHSG_SETTING sHSG_TABLE_SETTING_MODEL0[eCM_PICTURE_SETTINGS_NUMBER] =
{ //A35G2_BRC_Casper_0022 //A35G2_BRC_Casper_0055
       //0 eCM_PICTURE_SETTINGS_PRESENTATION
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 33, 127, 127,
			230, 120, 127,
			 56, 121, 127,
			127, 127, 127
        },
       // 1 eCM_PICTURE_SETTINGS_VIDEO
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 57, 127, 127,
			208, 119, 127,
			 70, 119, 127,
			127, 127, 127
        },
       // 2 eCM_PICTURE_SETTINGS_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 33, 127, 127,
			230, 121, 127,
			 58, 120, 127,
			127, 127, 127
        },
       // 3 eCM_PICTURE_SETTINGS_ENHANCED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 33, 127, 127,
			230, 120, 127,
			 56, 121, 127,
			127, 127, 127
        },
       // 4 eCM_PICTURE_SETTINGS_REC709
        {
			127, 127, 127,
			147, 127, 127,
			 90, 127, 127,
			 57, 127, 127,
			208, 119, 127,
			 70, 119, 127,
			127, 127, 127
        },
       // 5 eCM_PICTURE_SETTINGS_REAL
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 6 eCM_PICTURE_SETTINGS_DICOMSIM
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 33, 127, 127,
			230, 120, 127,
			 56, 121, 127,
			127, 127, 127
        },
       // 7 eCM_PICTURE_SETTINGS_2DHIGHSPEED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 36, 127, 127,
			229, 121, 127,
			 57, 120, 127,
			127, 127, 127
        },
       // 8 eCM_PICTURE_SETTINGS_3D
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			100, 127, 127,
			192, 120, 127,
			 56, 121, 127,
			127, 127, 127
        },
       // 9 eCM_PICTURE_SETTINGS_BLENDING
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 33, 127, 127,
			230, 124, 127,
			 43, 122, 127,
			127, 127, 127
        },
		//10 eCM_PICTURE_SETTINGS_USER
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 33, 127, 127,
			230, 121, 127,
			 58, 120, 127,
			127, 127, 127
        },

       // 11 eCM_PICTURE_SETTINGS_HDR
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			 33, 127, 127,
			230, 120, 127,
			 56, 121, 127,
			127, 127, 127
        },
       // 12 eCM_PICTURE_SETTINGS_SRGB
        {
			127, 127, 127,
			147, 127, 127,
			 90, 127, 127,
			 57, 127, 127,
			208, 119, 127,
			 70, 119, 127,
			127, 127, 127
        },
       // 13 eCM_PICTURE_SETTINGS_SUPER_BRIGHT
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        },
       // 14 eCM_PICTURE_SETTINGS_SUPER_RED
        {
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127,
			127, 127, 127
        }
};
#endif


VERIFY_SIZE_OF(sWallColorValues, sizeof(sWallColorValues[0][0])*eCM_PICTURE_SETTINGS_NUMBER*eCM_WALL_COLOR_NUMBER);
VERIFY_SIZE_OF(sHSG_TABLE_SETTING_MODEL2, sizeof(sHSG_TABLE_SETTING_MODEL2[0])*eCM_PICTURE_SETTINGS_NUMBER);
VERIFY_SIZE_OF(sHSG_TABLE_SETTING_MODEL1, sizeof(sHSG_TABLE_SETTING_MODEL1[0])*eCM_PICTURE_SETTINGS_NUMBER);
VERIFY_SIZE_OF(sHSG_TABLE_SETTING_MODEL0, sizeof(sHSG_TABLE_SETTING_MODEL0[0])*eCM_PICTURE_SETTINGS_NUMBER);


#if defined(PLATFORM_H60_2K) //HICC2_Doulas_0033 //HICC2_Casper_0016
sCOLOR_SETTING sColorSetting[] =	//HICC2_Doulas_0100//A70Gen2_Doulas_0003 Modify
{
/*Presentation */  	{0x00, eCM_COLOR_TEMPERATURE_WARM,    	eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Video        */	{0x00, eCM_COLOR_TEMPERATURE_WARM, 	    eCM_GAMMA_FILM,          TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Bright       */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 	  	eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Enhanced     */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 	  	eCM_GAMMA_FILM,      	 TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*REC709       */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Real		   */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_FILM,          TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*DICOM SIM    */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_DICOM,         TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*2D high speed*/	{0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*3D           */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Blending     */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*User         */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*HDR SIM      */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_HDR_STANDARD,  TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*sRGB         */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Super Bright */  	{0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Super Red    */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*3D Passive   */   {0x00, eCM_COLOR_TEMPERATURE_WARM,      eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*HLG          */   {0x00, eCM_COLOR_TEMPERATURE_WARM,      eCM_GAMMA_HDR_STANDARD,  TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
};

#elif defined(PLATFORM_H60_4K) //HICC2_Doulas_0100 //HICC2_Casper_0016
sCOLOR_SETTING sColorSetting[] =  //HICC2_Doulas_0107
{
/*Presentation */  	{0x00, eCM_COLOR_TEMPERATURE_WARM,    	eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Video        */	{0x00, eCM_COLOR_TEMPERATURE_WARM, 	    eCM_GAMMA_FILM,          TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Bright       */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 	  	eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Enhanced     */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 	  	eCM_GAMMA_FILM,      	 TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*REC709       */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Real		   */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*DICOM SIM    */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_DICOM,         TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*2D high speed*/	{0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*3D           */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Blending     */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*User         */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*HDR SIM      */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_HDR_STANDARD,  TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*sRGB         */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Super Bright */  	{0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*Super Red    */   {0x00, eCM_COLOR_TEMPERATURE_WARM, 		eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*3D Passive   */   {0x00, eCM_COLOR_TEMPERATURE_WARM,      eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*HLG          */   {0x00, eCM_COLOR_TEMPERATURE_WARM,      eCM_GAMMA_HDR_STANDARD,  TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
};

#else //HICC2_Casper_0016
sCOLOR_SETTING sColorSetting[] =
{
/*Presentation */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_FILM,          TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*Video        */	{0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF}, //HICC2_AC_0016
/*Bright       */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*Enhanced     */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_VIDEO,         TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*REC709       */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*Real		   */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_FILM,          TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*DICOM SIM    */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_FILM,          TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*2D high speed*/	{0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*3D           */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*Blending     */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*User         */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*HDR SIM      */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_HDR_STANDARD,  TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*sRGB         */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*SuperBright  */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*Super Red    */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_OFF},
/*3D Passive   */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_2_2,           TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
/*HLG          */   {0x00, eCM_COLOR_TEMPERATURE_WARM, eCM_GAMMA_HDR_STANDARD,  TRUE, WHITE_PEAKING_DEFAULT_VALUE , 0x01, 0x00, SHARPNESS_DEFAULT_VALUE, BRIGHTNESS_DEFAULT_VALUE, CONTRAST_DEFAULT_VALUE, HUE_DEFAULT_VALUE, SATURATION_DEFAULT_VALUE, RED_GAIN_DEFAULT_VALUE, GREEN_GAIN_DEFAULT_VALUE, BLUE_GAIN_DEFAULT_VALUE, RED_OFFSET_DEFAULT_VALUE, GREEN_OFFSET_DEFAULT_VALUE, BLUE_OFFSET_DEFAULT_VALUE, eCM_WALL_COLOR_WHITE},
};
#endif

VERIFY_SIZE_OF(sColorSetting, sizeof(sColorSetting[0])*DISPLAY_MODE_MAX);


#if defined(PLATFORM_A70G2) || defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)

const char *pcPCBA_VER_Str[eHWVER_INVALID] = //STB54605
{
     /* 0  eHWVER_M04_VERA = 0 */ "A", //0
     /* 1  eHWVER_M04_VERB     */ "B", //260
     /* 2  eHWVER_M04_VERC     */ "B", //372
     /* 3  eHWVER_M04_VERD     */ "C", //816
     /* 4  eHWVER_M04_VERE     */ "C", //1084
     /* 5  eHWVER_M04_VERF     */ "Reserved", //1341
     /* 6  eHWVER_M04_VERG     */ "Reserved", //1657
     /* 7  eHWVER_M04_VERH     */ "Reserved", //1951
     /* 8  eHWVER_M04_VERI     */ "Reserved", //2217
     /* 9  eHWVER_M04_VERJ     */ "Reserved", //2483
     /* 9  eHWVER_M04_VERK     */ "Reserved", //2730
     /* 10 eHWVER_M04_VERL     */ "Reserved", //2988
     /* 11 eHWVER_M04_VERM     */ "Reserved", //3260
     /* 12 eHWVER_M04_VERN     */ "Reserved", //3513
     /* 13 eHWVER_M04_VERO     */ "Reserved", //3780
     /* 14 eHWVER_M04_VERP     */ "Reserved", //3885
};

#elif defined(PLATFORM_H60_2K)

const char *pcPCBA_VER_Str[eHWVER_INVALID] = //STB54605
{
     /* 0  eHWVER_M04_VERA = 0 */ "H602K PCBA REV:A", //0
     /* 1  eHWVER_M04_VERB     */ "H602K PCBA REV:B", //260
     /* 2  eHWVER_M04_VERC     */ "H602K PCBA REV:C", //372
     /* 3  eHWVER_M04_VERD     */ "H602K PCBA REV:D", //816
     /* 4  eHWVER_M04_VERE     */ "H602K PCBA REV:E", //1084
     /* 5  eHWVER_M04_VERF     */ "H602K PCBA REV:F", //1341
     /* 6  eHWVER_M04_VERG     */ "H602K PCBA REV:G", //1657
     /* 7  eHWVER_M04_VERH     */ "H602K PCBA REV:H", //1951
     /* 8  eHWVER_M04_VERI     */ "H602K PCBA REV:I", //2217
     /* 9  eHWVER_M04_VERJ     */ "H602K PCBA REV:J", //2483
     /* 9  eHWVER_M04_VERK     */ "H602K PCBA REV:K", //2730
     /* 10 eHWVER_M04_VERL     */ "H602K PCBA REV:L", //2988
     /* 11 eHWVER_M04_VERM     */ "H602K PCBA REV:M", //3260
     /* 12 eHWVER_M04_VERN     */ "H602K PCBA REV:N", //3513
     /* 13 eHWVER_M04_VERO     */ "H602K PCBA REV:O", //3780
     /* 14 eHWVER_M04_VERP     */ "H602K PCBA REV:P", //3885
};

#elif defined(PLATFORM_H30_FPGA_4K)

const char *pcPCBA_VER_Str[] = //STB54605
{
     /* 0  eHWVER_M04_VERA = 0 */ "H304K PCBA REV:A", //131
     /* 1  eHWVER_M04_VERB     */ "H304K PCBA REV:B", //439
     /* 2  eHWVER_M04_VERC     */ "H304K PCBA REV:C", //683
     /* 3  eHWVER_M04_VERD     */ "H304K PCBA REV:D", //945
     /* 4  eHWVER_M04_VERE     */ "H304K PCBA REV:E", //1231
     /* 5  eHWVER_M04_VERF     */ "H304K PCBA REV:F", //1470
     /* 6  eHWVER_M04_VERG     */ "H304K PCBA REV:G", //1779
     /* 7  eHWVER_M04_VERH     */ "H304K PCBA REV:H", //2023
     /* 8  eHWVER_M04_VERI     */ "H304K PCBA REV:I", //2267
     /* 9  eHWVER_M04_VERJ     */ "H304K PCBA REV:J", //2520
     /* 9  eHWVER_M04_VERK     */ "H304K PCBA REV:K", //2815
     /* 10 eHWVER_M04_VERL     */ "H304K PCBA REV:L", //3071
     /* 11 eHWVER_M04_VERM     */ "H304K PCBA REV:M", //3325
     /* 12 eHWVER_M04_VERN     */ "H304K PCBA REV:N", //3570
     /* 13 eHWVER_M04_VERO     */ "H304K PCBA REV:O", //3839
     /* 14 eHWVER_M04_VERP     */ "H304K PCBA REV:P", //4095
};
VERIFY_SIZE_OF(pcPCBA_VER_Str, sizeof(pcPCBA_VER_Str[0])*eHWVER_INVALID);

#elif defined(PLATFORM_H30_4K)

const char *pcPCBA_VER_Str[] = //STB54605
{
     /* 0  eHWVER_M04_VERA = 0 */ "A", //131
     /* 1  eHWVER_M04_VERB     */ "B", //439
     /* 2  eHWVER_M04_VERC     */ "C", //683
     /* 3  eHWVER_M04_VERD     */ "D", //945
     /* 4  eHWVER_M04_VERE     */ "E", //1231
     /* 5  eHWVER_M04_VERF     */ "F", //1470
     /* 6  eHWVER_M04_VERG     */ "G", //1779
     /* 7  eHWVER_M04_VERH     */ "H", //2023
     /* 8  eHWVER_M04_VERI     */ "I", //2267
     /* 9  eHWVER_M04_VERJ     */ "J", //2520
     /* 9  eHWVER_M04_VERK     */ "K", //2815
     /* 10 eHWVER_M04_VERL     */ "L", //3071
     /* 11 eHWVER_M04_VERM     */ "M", //3325
     /* 12 eHWVER_M04_VERN     */ "N", //3570
     /* 13 eHWVER_M04_VERO     */ "O", //3839
     /* 14 eHWVER_M04_VERP     */ "P", //4095
};
VERIFY_SIZE_OF(pcPCBA_VER_Str, sizeof(pcPCBA_VER_Str[0])*eHWVER_INVALID);


#else  //A35G2

const char *pcPCBA_VER_Str[eHWVER_INVALID] =
{
     /* 0  eHWVER_M04_VERA = 0 */ "G01 Ver.A(M04)", //0
     /* 1  eHWVER_M04_VERB     */ "G11 Ver.A(M04)", //260
     /* 2  eHWVER_M04_VERC     */ "G11 Ver.A(M04)", //372
     /* 3  eHWVER_M04_VERD     */ "G03 Ver.A(M04)", //816
     /* 4  eHWVER_M04_VERE     */ "G03 Ver.A(M04)", //1084
     /* 5  eHWVER_M04_VERF     */ "Ver.E(M04)", //1341
     /* 6  eHWVER_M04_VERG     */ "Ver.E(M04)", //1657
     /* 7  eHWVER_M04_VERH     */ "Ver.F(M04)", //1951
     /* 8  eHWVER_M04_VERI     */ "Ver.F(M04)", //2217
     /* 9  eHWVER_M04_VERJ     */ "Ver.G(M04)", //2730
     /* 10 eHWVER_M04_VERK     */ "Ver.G(M04)", //2988
     /* 11 eHWVER_M04_VERL     */ "G01 Ver.B(M04)", //3260
     /* 12 eHWVER_M04_VERM     */ "G01 Ver.B(M04)", //3513
     /* 13 eHWVER_M04_VERN     */ "G01 Ver.B(M04)", //3780

     /* 0  eHWVER_M08_VERA     */ "G01 Ver.A(M08)", //0
     /* 1  eHWVER_M08_VERB     */ "G11 Ver.A(M08)", //260
     /* 2  eHWVER_M08_VERC     */ "G11 Ver.A(M08)", //372
     /* 3  eHWVER_M08_VERD     */ "G03 Ver.A(M08)", //816
     /* 4  eHWVER_M08_VERE     */ "G03 Ver.A(M08)", //1084
     /* 5  eHWVER_M08_VERF     */ "Ver.E(M08)", //1341
     /* 6  eHWVER_M08_VERG     */ "Ver.E(M08)", //1657
     /* 7  eHWVER_M08_VERH     */ "Ver.F(M08)", //1951
     /* 8  eHWVER_M08_VERI     */ "Ver.F(M08)", //2217
     /* 9  eHWVER_M08_VERJ     */ "Ver.G(M08)", //2730
     /* 10 eHWVER_M08_VERK     */ "Ver.G(M08)", //2988
     /* 11 eHWVER_M08_VERL     */ "G01 Ver.B(M08)", //3260
     /* 12 eHWVER_M08_VERM     */ "G01 Ver.B(M08)", //3513
     /* 13 eHWVER_M08_VERN     */ "G01 Ver.B(M08)", //3780
};
#endif

#if defined(PLATFORM_H60_2K) || defined(PLATFORM_H60_4K)//#ifdef PLATFORM_H60_2K  //HICC2_Doulas_0101//HICC2_Doulas_0014 Modify   {2,1,2,3,4,5,6,7,8,9,10,11,2,2,2}; //CM map to DDP index
UINT8 FormatterPictureSettingMapping[] = {eCM_PICTURE_SETTINGS_PRESENTATION,
                                          eCM_PICTURE_SETTINGS_VIDEO,
                                          eCM_PICTURE_SETTINGS_BRIGHT,
                                          eCM_PICTURE_SETTINGS_ENHANCED,
                                          eCM_PICTURE_SETTINGS_REC709,
                                          eCM_PICTURE_SETTINGS_REAL,
                                          eCM_PICTURE_SETTINGS_DICOMSIM,
                                          eCM_PICTURE_SETTINGS_2DHIGHSPEED,
                                          eCM_PICTURE_SETTINGS_3D,
                                          eCM_PICTURE_SETTINGS_BLENDING,
                                          eCM_PICTURE_SETTINGS_USER,
                                          eCM_PICTURE_SETTINGS_HDR,
                                          eCM_PICTURE_SETTINGS_SRGB,
                                          eCM_PICTURE_SETTINGS_SUPER_BRIGHT,
                                          eCM_PICTURE_SETTINGS_SUPER_RED,
                                          eCM_PICTURE_SETTINGS_3D_PASSIVE,
                                          eCM_PICTURE_SETTINGS_HLG}; //CM map to DDP index
#elif defined (PLATFORM_H30_4K)        	//for Optoma H30K
UINT8 FormatterPictureSettingMapping[] = {eCM_PICTURE_SETTINGS_PRESENTATION,
                                          eCM_PICTURE_SETTINGS_VIDEO,
                                          eCM_PICTURE_SETTINGS_BRIGHT,
                                          eCM_PICTURE_SETTINGS_ENHANCED,
                                          eCM_PICTURE_SETTINGS_REC709,
                                          eCM_PICTURE_SETTINGS_REAL,
                                          eCM_PICTURE_SETTINGS_DICOMSIM,
                                          eCM_PICTURE_SETTINGS_2DHIGHSPEED,
                                          eCM_PICTURE_SETTINGS_3D,
                                          eCM_PICTURE_SETTINGS_BLENDING,
                                          eCM_PICTURE_SETTINGS_USER,
                                          eCM_PICTURE_SETTINGS_HDR,
                                          eCM_PICTURE_SETTINGS_SRGB,
                                          eCM_PICTURE_SETTINGS_SUPER_BRIGHT,
                                          eCM_PICTURE_SETTINGS_SUPER_RED,
                                          eCM_PICTURE_SETTINGS_3D_PASSIVE,
                                          eCM_PICTURE_SETTINGS_HLG}; //CM map to DDP index
#else
UINT8 FormatterPictureSettingMapping[] = {eCM_PICTURE_SETTINGS_PRESENTATION,
                                          eCM_PICTURE_SETTINGS_VIDEO,
                                          eCM_PICTURE_SETTINGS_BRIGHT,
                                          eCM_PICTURE_SETTINGS_ENHANCED,
                                          eCM_PICTURE_SETTINGS_REC709,
                                          eCM_PICTURE_SETTINGS_REAL,
                                          eCM_PICTURE_SETTINGS_DICOMSIM,
                                          eCM_PICTURE_SETTINGS_2DHIGHSPEED,
                                          eCM_PICTURE_SETTINGS_3D,
                                          eCM_PICTURE_SETTINGS_BLENDING,
                                          eCM_PICTURE_SETTINGS_USER,
                                          eCM_PICTURE_SETTINGS_HDR,
                                          eCM_PICTURE_SETTINGS_SRGB,
                                          eCM_PICTURE_SETTINGS_SUPER_BRIGHT,
                                          eCM_PICTURE_SETTINGS_SUPER_RED,
                                          eCM_PICTURE_SETTINGS_3D_PASSIVE,
                                          eCM_PICTURE_SETTINGS_HLG}; //CM map to DDP index
#endif

VERIFY_SIZE_OF(FormatterPictureSettingMapping, sizeof(FormatterPictureSettingMapping[0])*eCM_PICTURE_SETTINGS_NUMBER);

sGLOBAL_SETTINGS sGlobalCfg;

void GlobalSettings_Init(void)
{
    sGlobalCfg.sCLI_CFG.CLI_Type = eCLI_TYPE_GENERIC;
    sGlobalCfg.sFN_CFG.SupportSerialEcho = 1;
    sGlobalCfg.sFN_CFG.SupportPin = 1;
    sGlobalCfg.sFN_CFG.SupportSourceList = 1;
    sGlobalCfg.sDataCode_CFG.SMCU_Version_Type = eDATACODE_SMCU_VERSION_IS_LD_CONVERT_VERSION;
    sGlobalCfg.sDataCode_CFG.Language_RemoveFirstStartupFlag = 1;

    //sGlobalCfg.LD_Info_Num = Syscfg_Value_Get_Typeint(eLD_INFO_Num);
    //sGlobalCfg.BLD_Num = Syscfg_Value_Get_Typeint(eBLD_Num);
    //sGlobalCfg.RLD_Num = Syscfg_Value_Get_Typeint(eRLD_Num);
}

//default gamma 2.2
UINT8 Gamma_Mapping[] = {
    4, //00 eCM_GAMMA_FILM
    4, //01 eCM_GAMMA_VIDEO
    4, //02 eCM_GAMMA_GRAPHICS
    4, //03 eCM_GAMMA_STANDARD
    4, //04 eCM_GAMMA_3D
    4, //05 eCM_GAMMA_BLACKBOARD
    4, //06 eCM_GAMMA_DICOM
    4, //07 eCM_GAMMA_1_8
    4, //08 eCM_GAMMA_1_9
    4, //09 eCM_GAMMA_2_0
    4, //10 eCM_GAMMA_2_1
    4, //11 eCM_GAMMA_2_2
    4, //12 eCM_GAMMA_2_3
    4, //13 eCM_GAMMA_2_4
    4, //14 eCM_GAMMA_2_5
    4, //15 eCM_GAMMA_2_6
    4, //16 eCM_GAMMA_CRT
    4, //17 eCM_GAMMA_ENHANCED
    4, //18 eCM_GAMMA_HDR_STANDARD
    4, //19 eCM_GAMMA_HDR_Film_HC
    4, //20 eCM_GAMMA_HDR_Bright_HC
    4, //21 eCM_GAMMA_HDR_Detail_HC
    4, //22 eCM_GAMMA_HDR_Standard_Optoma
    4, //23 eCM_GAMMA_HDR_Film_Optoma
    4, //24 eCM_GAMMA_HDR_Bright_Optoma
    4, //25 eCM_GAMMA_HDR_Detail_Optoma
    4, //26 eCM_GAMMA_HLG_STANDARD
    4, //27 eCM_GAMMA_HLG_Film_HC
    4, //28 eCM_GAMMA_HLG_Bright_HC
    4, //29 eCM_GAMMA_HLG_Detail_HC
    4, //30 eCM_GAMMA_1_6
    4, //31 eCM_GAMMA_1_7
    4, //32 eCM_GAMMA_2_7
    4, //33 eCM_GAMMA_2_8
    4, //34 eCM_GAMMA_2_9
    4, //35 eCM_GAMMA_3_0
    4, //36 eCM_GAMMA_3_1
    4, //37 eCM_GAMMA_3_2
    4, //38 eCM_GAMMA_3_3
};

VERIFY_SIZE_OF(Gamma_Mapping, sizeof(Gamma_Mapping[0])*eCM_GAMMA_NUMBER);


