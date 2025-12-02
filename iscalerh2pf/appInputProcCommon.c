#include "appInputProc.h"
#include "appInputProcCommon.h"

INT8 palInputProc_ACU_Target_Select_KeyCheck(UINT32 eKey)
{
    if( palDataMgr_ACU_Target_Status_Get() == ets_ON )
    {
        UINT8 ucTS = palDataMgr_ACU_Target_Select_Tmp_Get();

        LOG_MSG(db_APP_INPUTKEY, "ACU Target Select Key (Ts = %d)\r\n", ucTS);

        switch(eKey)
        {
            //case keMENU: //有和PAD確認行為 Larry
            case kePOWERON:
            case keSTANDBY:
            case keBACK:
                #if 0
                palGeo_TestPatternAcuExitBlackBorderHandle();
                palDataMgr_ACU_Target_Status_Set(ets_OFF); //A65_OPTOMA_Julie_0082
                palDataMgr_ACU_DisableBlend_Set(ets_OFF, TRUE);// R70K_Bruce_0052
                #endif
                //appGui_MenuTimeout_Refresh();
                return TRUE;

            case keUP:      // Y --
                if( ( ucTS % 10 ) > ACU_TARGET_Y_MIN )
                {
                    ucTS --;
                    palDataMgr_ACU_Target_Select_Tmp_Set(ucTS);
                }
                return FALSE;

            case keLEFT:    // X --
                if( ( ucTS / 10 ) > ACU_TARGET_X_MIN )
                {
                    ucTS -= 10;
                    palDataMgr_ACU_Target_Select_Tmp_Set(ucTS);
                }
                return FALSE;

            case keENTER:
                palDataMgr_Data_Access(edcACU_TARGET_SEL, edaWRITE_RAM_ONLY_WITH_ACTION, &ucTS);
                #if 0
                palGeo_TestPatternAcuExitBlackBorderHandle();
                palDataMgr_ACU_Target_Status_Set(ets_OFF); //A65_OPTOMA_Julie_0082
                palDataMgr_ACU_DisableBlend_Set(ets_OFF, TRUE);// R70K_Bruce_0052
                //appGui_MenuTimeout_Refresh();
                #endif
                return TRUE;

            case keRIGHT:   // X ++
                if( ( ucTS / 10 ) < ACU_TARGET_X_MAX )
                {
                    ucTS += 10;
                    palDataMgr_ACU_Target_Select_Tmp_Set(ucTS);
                }
                return FALSE;

            case keDOWN:    // Y ++
                if( ( ucTS % 10 ) < ACU_TARGET_Y_MAX )
                {
                    ucTS ++;
                    palDataMgr_ACU_Target_Select_Tmp_Set(ucTS);
                }
                return FALSE;

            case keMENU: //HICC2_Doulas_0122
                #if 0
                palGeo_TestPatternAcuExitBlackBorderHandle();
                palDataMgr_ACU_Target_Status_Set(ets_OFF);
                palDataMgr_ACU_DisableBlend_Set(ets_OFF, TRUE);
                #endif
                return TRUE;

            default:
                return FALSE;   //need block key
        }

    }

    return FALSE;  //block key

}


