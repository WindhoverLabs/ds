/*************************************************************************
** Includes
*************************************************************************/
#include "lc_custom.h"
#include "lc_tbldefs.h"
#include "lc_events.h"
#include "lc_mission_cfg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initiate an RTS request                                         */
/*                                                                 */
/* NOTE: For complete prolog information, see 'lc_custom.h'        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */   
void LC_ExecuteRTS(uint16 RTSId)
{
    /***************************************************************
    ** This is a local declaration of the command message structure
    ** to initiate an RTS and has been placed here to allow the 
    ** the LC application to be built without including headers from
    ** any other applications (like Stored Commanding). 
    ** A mission may choose to remove this and use a message
    ** structure declared elsewhere instead.
    **
    ** This also applies to the LC_RTS_REQ_MID and LC_RTS_REQ_CC
    ** constants (see lc_mission_cfg.h).
    */
    typedef struct {
        uint8          CmdHeader[CFE_SB_CMD_HDR_SIZE];

        uint16         RTSId;               

    } LC_RTSRequest_t;
    
    LC_RTSRequest_t RTSRequest;
    /**************************************************************/
    
    CFE_SB_InitMsg((CFE_SB_Msg_t *) ((uint32) &RTSRequest),
            LC_RTS_REQ_MID, sizeof(LC_RTSRequest_t), TRUE);

    CFE_SB_SetCmdCode((CFE_SB_Msg_t *) ((uint32) &RTSRequest), 
                                               LC_RTS_REQ_CC);
        
    RTSRequest.RTSId = RTSId;
        
    CFE_SB_SendMsg((CFE_SB_Msg_t *) ((uint32) &RTSRequest));
    
    return;
    
} /* end LC_ExecuteRTS */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Mission specific custom function entry point                    */
/*                                                                 */
/* NOTE: For complete prolog information, see 'lc_custom.h'        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */   
uint8 LC_CustomFunction(uint16          WatchIndex,
                        uint32          ProcessedWPData,
                        CFE_SB_MsgPtr_t MessagePtr,
                        uint32          WDTCustomFuncArg)
{
    uint8  EvalResult = LC_WATCH_FALSE;
    
    /*
    ** This function is the entry point for all watchpoints where
    ** the OperatorID in the watchpoint definition table entry
    ** is set to LC_OPER_CUSTOM.
    ** 
    ** For this reason The first step would normally be to
    ** switch on the WatchIndex to figure out what watchpoint got
    ** us here. As an alternate, a mission may choose to use the
    ** WDTCustomFuncArg for this instead.
    */
    switch (WatchIndex)
    {
        case 0x0000:
        case 0x0001:
        default:
            CFE_EVS_SendEvent(LC_CFCALL_ERR_EID, CFE_EVS_ERROR,
                              "Unexpected LC_CustomFunction call: WP = %d",
                              WatchIndex);
            break;
        
    } /* end WatchIndex switch */
    
    return (EvalResult);
    
} /* end LC_CustomFunction */

/************************/
/*  End of File Comment */
/************************/
