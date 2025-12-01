#include <stdio.h>
#include <string.h>
#include "CommonData.h"

//Model Name Table
const char ModelNameString[][eMN_INVALID][MODEL_NAME_MAX_STR_LEN] =
{
    //CustomCode    //Model Name        //Service Mode Name  //China Region Name //TWN Region Name   //Fota Name     //Neutral Name

	//T100
    {"01010100",     "4K10-HS",          "4K10-HS",          "4K10-HS",          "4K10-HS",          "4K10-HS",      "4K10-HS"},
    {"01010200",     "4K7-HS",           "4K7-HS",           "4K7-HS",           "4K7-HS",         	 "4K7-HS",       "4K7-HS"},
    {"01020100",     "ZK1050",           "ZK1050",           "ZK1050",           "ZK1050",           "ZK1050",       "ZK1050"},
    {"01020200",     "ZK750",            "ZK750",            "ZK750",            "ZK750",          	 "ZK750",      	 "ZK750"},

    //A35G2
    {"08010100",     "DWU880-GS",        "DWU880-GS",        "DWU9200-GS",       "DWU880A-GS",       "DWU880-GS",    "DWU880-GS"},
    {"08010200",     "DWU1100-GS",       "DWU1100-GS",       "DWU1200-GS",       "DWU1100A-GS",      "DWU1100-GS",   "DWU1100-GS"},
    {"08010300",     "DWU1400-GS",       "DWU1400-GS",       "DWU1460-GS",       "DWU1400A-GS",      "DWU1400-GS",   "DWU1400-GS"},
    {"08020100",     "DAZU88ZZ",         "DAZU88ZZ",         "DAZU88ZZ",         "DAZU88ZZ",         "ZU800",        "General 2190"},  //not use
    {"08020200",     "DAZUGGZZ",         "DAZUGGZZ",         "DAZUGGZZ",         "DAZUGGZZ",         "ZU1100",       "General 2191"},
    {"08020300",     "DAZUGSZZ",         "DAZUGSZZ",         "DAZUGSZZ",         "DAZUGSZZ",         "ZU1300",       "General 2192"},
    {"08030100",     "G62-W9",           "G62-W9",           "G62-W9",           "G62-W9",           "G62-W9",       "G62-W9"},
    {"08030200",     "G62-W11",          "G62-W11",          "G62-W11",          "G62-W11",          "G62-W11",      "G62-W11"},
    {"08030300",     "G62-W14",          "G62-W14",          "G62-W14",          "G62-W14",          "G62-W14",      "G62-W14"},

    //A70LK
	{"09010100",     "4K13-HS",          "4K13-HS",          "4K13-HS",          "4K13A-HS",         "4K13-HS",      "4K13-HS"},
    {"09010200",     "4K16-HS",          "4K16-HS",          "4K16-HS",          "4K16A-HS",         "4K16-HS",      "4K16-HS"},
    {"09010300",     "4K22-HS",          "4K22-HS",          "4K22-HS",          "4K22A-HS",         "4K22-HS",      "4K22-HS"},

	//A70G2
    {"0B010100",     "DWU15-HS",         "DWU15-HS",         "DWU1560-HS",       "DWU15A-HS",        "DWU15-HS",     "DWU15-HS"},
    {"0B010200",     "DWU19-HS",         "DWU19-HS",         "DWU1960-HS",       "DWU19A-HS",        "DWU19-HS",     "DWU19-HS"},
    {"0B010300",     "DWU23-HS",         "DWU23-HS",         "DWU2360-HS",       "DWU23A-HS",        "DWU23-HS",     "DWU23-HS"},

	//H60_2K
    {"0D010100",     "DWU1800-JS",       "DWU1800-JS",       "DWU1800-JS",       "DWU1800A-JS",      "DWU1800-JS",   "DWU1800-JS"},
    {"0D010200",     "DWU2400-JS",       "DWU2400-JS",       "DWU2400-JS",       "DWU2400A-JS",      "DWU2400-JS",   "DWU2400-JS"},

	//H60_4K
    {"0E010100",     "4K1600-JS",        "4K1600-JS",        "4K1600-JS",        "4K1600A-JS",       "4K1600-JS",    "4K1600-JS"},
    {"0E010200",     "4K2100-JS",        "4K2100-JS",        "4K2100-JS",        "4K2100A-JS",       "4K2100-JS",    "4K2100-JS"},
    {"0E010300",     "4K1600-JS",        "4K1600-JS",        "4K1600-JS",        "4K1600A-JS",       "4K1600-JS",    "4K1600-JS"},
    {"0E010400",     "4K2100-JS",        "4K2100-JS",        "4K2100-JS",        "4K2100A-JS",       "4K2100-JS",    "4K2100-JS"},

	//H30_4K_Christie
    {"0F010100",     "4K1000-KS",        "4K1000-KS",        "4K1000-KS",        "4K1000A-KS",       "4K1000-KS",    "4K1000-KS"},
    {"0F010200",     "4K1400-KS",        "4K1400-KS",        "4K1400-KS",        "4K1400A-KS",       "4K1400-KS",    "4K1400-KS"},

    //H30_4K_Optoma
    {"0F020100",     "DAZKGGNZ",         "DAZKGGNZ",          "DAZKGGNZ",         "DAZKGGNZ",         "ZK1120",      "General 2491"},
    {"0F020200",     "DAZKGSNZ",         "DAZKGSNZ",          "DAZKGSNZ",         "DAZKGSNZ",         "ZK1320",      "General 2492"},

	//R70K_Christie
    {"0C010100",     "4K860-iS",         "4K860-iS",         "4K860-iS",         "4K860A-iS",        "4K860-iS",     "4K860-iS"},
    {"0C010200",     "4K960-iS",         "4K960-iS",         "4K960-iS",         "4K960A-iS",        "4K960-iS",     "4K960-iS"},

    //R70K_Fujifilm
    {"0C100100",     "FP-ZUH6000",       "FP-ZUH6000",      "FP-ZUH6000",       "FP-ZUH6000",       "FP-ZUH6000",   "FP-ZUH6000"},
    {"0C100200",     "FP-ZUH6000",       "FP-ZUH6000",      "FP-ZUH6000",       "FP-ZUH6000",       "FP-ZUH6000",   "FP-ZUH6000"},

    //H30K_Fujifilm
    {"0F100100",     "FP-ZUH10000",      "FP-ZUH10000",     "FP-ZUH10000",      "FP-ZUH10000",      "FP-ZUH10000",  "FP-ZUH10000"}, //mockup
    {"0F100200",     "FP-ZUH12000",      "FP-ZUH12000",     "FP-ZUH12000",      "FP-ZUH12000",      "FP-ZUH12000",  "FP-ZUH12000"},

    //H30_4K_SNC
    {"0F110100",     "General 001",      "General 001",    	"General 001",     	"General 001",     	"General 001", 	"General 001"},

};

#define MODEL_NAME_TABLE_SIZE (sizeof(ModelNameString)/eMN_INVALID/MODEL_NAME_MAX_STR_LEN)

UINT32 CommonData_ModelNameTable_SizeGet(void)
{
    return (UINT32)(MODEL_NAME_TABLE_SIZE);
}

UINT32 CommonData_ModelNameTable_IndexGet(char *ProjectorID)
{
    for(UINT32 index=0; index<MODEL_NAME_TABLE_SIZE; index++)
    {
        if(strncmp(ProjectorID, ModelNameString[index][eMN_PROJECTOR_ID], strlen(ModelNameString[index][eMN_PROJECTOR_ID])) == 0)
        {
            //printf("\nModel Name Table Index %d\n\n", index);
            return index;
        }
    }

    return MODEL_NAME_TABLE_INDEX_NOT_FOUND;  //not found
}




