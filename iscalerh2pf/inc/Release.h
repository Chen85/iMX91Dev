// HICC 2.0 Platform
#define VER_MAJOR   (39)
#define VER_MINOR   (65)


#define CUSTOMER_ID_CODE Syscfg_Value_Get_Typeint(eCustomerID)



/*----------------------------------------------------*/
//Christie
//Vx.y.z

//x: 0 & 1.   EVT & DVT 為0, PVT & MP後為1. (不分DVT1, DVT2等stage次數, 此皆視為DVT階段且x數值為0)

//y: 0-99, 該階段FW release的次數

//z: 中光電流水號(end user端僅會看到0之數字)

//ex 1: T100 9/21 release之FW(PVT approval後第一次release版本)

//ex 2: T100 11/5預計再次release FW for 9/21版本fix (客戶端視為PVT 2, PVT approval後第二次release版本)

//a V1.1.0

/*----------------------------------------------------*/


//Projector Code
#define PROJECTOR_CODE_T100         (0x01)
#define PROJECTOR_CODE_A70          (0x02)
#define PROJECTOR_CODE_B35LC        (0x03)
#define PROJECTOR_CODE_B35LB_PLUS   (0x04)
#define PROJECTOR_CODE_A65          (0x05)
#define PROJECTOR_CODE_R70          (0x06)
#define PROJECTOR_CODE_T50          (0x07)
#define PROJECTOR_CODE_X35          (0x08)

//Customer Code
#define CUSTOMER_CODE_CHRISTIE      (0x01)
#define CUSTOMER_CODE_OPTOMA        (0x02)
#define CUSTOMER_CODE_BASIL         (0x03)
#define CUSTOMER_CODE_DONVIEW	    (0x04)
#define CUSTOMER_CODE_EIKI          (0x05)
#define CUSTOMER_CODE_RICOH	    	(0x06)
#define CUSTOMER_CODE_KRINDA 	    (0x07)
#define CUSTOMER_CODE_WINCOMN 	    (0x08)
#define CUSTOMER_CODE_AVIC	    	(0x09)

#define PROJECTOR_IDENTIFY  (PROJECTOR_CODE_X35 << 24) | (CUSTOMER_ID_CODE << 16)

