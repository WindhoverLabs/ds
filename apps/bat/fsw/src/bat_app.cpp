/************************************************************************
** Includes
*************************************************************************/
#include <string.h>

#include "cfe.h"

#include "bat_app.h"
#include "bat_msg.h"
#include "bat_version.h"
#include <math.h>
#include "px4lib.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Instantiate the application object.                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
BAT oBAT;



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Default constructor.                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
BAT::BAT()
{

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Destructor constructor.                                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
BAT::~BAT()
{

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize event tables.                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 BAT::InitEvent()
{
    int32  iStatus=CFE_SUCCESS;

    /* Register the table with CFE */
    iStatus = CFE_EVS_Register(0, 0, CFE_EVS_BINARY_FILTER);
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("BAT - Failed to register with EVS (0x%08lX)\n", iStatus);
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize Message Pipes                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 BAT::InitPipe()
{
    int32  iStatus=CFE_SUCCESS;

    /* Init schedule pipe and subscribe to wakeup messages */
    iStatus = CFE_SB_CreatePipe(&SchPipeId,
    		BAT_SCH_PIPE_DEPTH,
			BAT_SCH_PIPE_NAME);
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = CFE_SB_SubscribeEx(BAT_WAKEUP_MID, SchPipeId, CFE_SB_Default_Qos, BAT_WAKEUP_MID_MAX_MSG_COUNT);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(BAT_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
            		"SCH Pipe failed to subscribe to BAT_WAKEUP_MID. (0x%08lX)",
                    iStatus);
            goto BAT_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(BAT_SEND_HK_MID, SchPipeId, CFE_SB_Default_Qos, BAT_SEND_HK_MID_MAX_MSG_COUNT);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(BAT_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
					 "SCH Pipe failed to subscribe to BAT_SEND_HK_MID. (0x%08X)",
					 (unsigned int)iStatus);
            goto BAT_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_SubscribeEx(PX4_ACTUATOR_CONTROLS_0_MID, SchPipeId, CFE_SB_Default_Qos, 1);
        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(BAT_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
                     "SCH Pipe failed to subscribe to PX4_ACTUATOR_CONTROLS_0_MID. (0x%08X)",
                     (unsigned int)iStatus);
            goto BAT_InitPipe_Exit_Tag;
        }
    }
    else
    {
        (void) CFE_EVS_SendEvent(BAT_PIPE_INIT_ERR_EID, CFE_EVS_ERROR,
			 "Failed to create SCH pipe (0x%08lX)",
			 iStatus);
        goto BAT_InitPipe_Exit_Tag;
    }

    /* Init command pipe and subscribe to command messages */
    iStatus = CFE_SB_CreatePipe(&CmdPipeId,
    		BAT_CMD_PIPE_DEPTH,
			BAT_CMD_PIPE_NAME);
    if (iStatus == CFE_SUCCESS)
    {
        /* Subscribe to command messages */
        iStatus = CFE_SB_Subscribe(BAT_CMD_MID, CmdPipeId);

        if (iStatus != CFE_SUCCESS)
        {
            (void) CFE_EVS_SendEvent(BAT_SUBSCRIBE_ERR_EID, CFE_EVS_ERROR,
				 "CMD Pipe failed to subscribe to BAT_CMD_MID. (0x%08lX)",
				 iStatus);
            goto BAT_InitPipe_Exit_Tag;
        }
    }
    else
    {
        (void) CFE_EVS_SendEvent(BAT_PIPE_INIT_ERR_EID, CFE_EVS_ERROR,
			 "Failed to create CMD pipe (0x%08lX)",
			 iStatus);
        goto BAT_InitPipe_Exit_Tag;
    }

BAT_InitPipe_Exit_Tag:
    return iStatus;
}
    

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize Global Variables                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BAT::InitData()
{
    /* Init housekeeping message. */
    CFE_SB_InitMsg(&HkTlm,
    		BAT_HK_TLM_MID, sizeof(HkTlm), TRUE);

    /* Init output messages */
    CFE_SB_InitMsg(&BatteryStatusMsg,
      		PX4_BATTERY_STATUS_MID, sizeof(PX4_BatteryStatusMsg_t), TRUE);
    BatteryStatusMsg.Timestamp = PX4LIB_GetPX4TimeUs();
    BatteryStatusMsg.CurrentFiltered = -1.0f;
    BatteryStatusMsg.VoltageFiltered = -1.0f;

	SampleTime.Seconds = 0;
	SampleTime.Subseconds = 0;

	RemainingVoltage = 1.0f;
	RemainingCapacity = 1.0f;
	Discharged = 0.0f;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* BAT initialization                                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 BAT::InitApp()
{
    int32  iStatus   = CFE_SUCCESS;
    int8   hasEvents = 0;

    iStatus = InitEvent();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("BAT - Failed to init events (0x%08lX)\n", iStatus);
        goto BAT_InitApp_Exit_Tag;
    }
    else
    {
        hasEvents = 1;
    }

    iStatus = InitPipe();
    if (iStatus != CFE_SUCCESS)
    {
        goto BAT_InitApp_Exit_Tag;
    }

    InitData();

    iStatus = InitConfigTbl();
    if (iStatus != CFE_SUCCESS)
    {
        goto BAT_InitApp_Exit_Tag;
    }

    iStatus = InitDevice();
    if (iStatus != CFE_SUCCESS)
    {
        goto BAT_InitApp_Exit_Tag;
    }

    iStatus = InitListenerTask();
    if (iStatus != CFE_SUCCESS)
    {
        goto BAT_InitApp_Exit_Tag;
    }

BAT_InitApp_Exit_Tag:
    if (iStatus == CFE_SUCCESS)
    {
        (void) CFE_EVS_SendEvent(BAT_INIT_INF_EID, CFE_EVS_INFORMATION,
                                 "Initialized.  Version %d.%d.%d.%d",
								 BAT_MAJOR_VERSION,
								 BAT_MINOR_VERSION,
								 BAT_REVISION,
								 BAT_MISSION_REV);
    }
    else
    {
        if (hasEvents == 1)
        {
            (void) CFE_ES_WriteToSysLog("BAT - Application failed to initialize\n");
        }
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Receive and Process Messages                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 BAT::RcvSchPipeMsg(int32 iBlocking)
{
    int32           iStatus=CFE_SUCCESS;
    CFE_SB_Msg_t*   MsgPtr=NULL;
    CFE_SB_MsgId_t  MsgId;

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(BAT_MAIN_TASK_PERF_ID);

    /* Wait for WakeUp messages from scheduler */
    iStatus = CFE_SB_RcvMsg(&MsgPtr, SchPipeId, iBlocking);

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(BAT_MAIN_TASK_PERF_ID);

    if (iStatus == CFE_SUCCESS)
    {
        MsgId = CFE_SB_GetMsgId(MsgPtr);
        switch (MsgId)
        {
            case BAT_WAKEUP_MID:
                PublishBatteryStatus();
                break;

            case BAT_SEND_HK_MID:
                ProcessCmdPipe();
                ReportHousekeeping();
                break;

            case PX4_ACTUATOR_ARMED_MID:
                memcpy(&CVT.ActuatorArmed, MsgPtr, sizeof(CVT.ActuatorArmed));
                break;

            case PX4_ACTUATOR_CONTROLS_0_MID:
                memcpy(&CVT.ActuatorControls0, MsgPtr, sizeof(CVT.ActuatorControls0));
                break;

            default:
                (void) CFE_EVS_SendEvent(BAT_MSGID_ERR_EID, CFE_EVS_ERROR,
                     "Recvd invalid SCH msgId (0x%04X)", MsgId);
        }
    }
    else if (iStatus == CFE_SB_NO_MESSAGE)
    {
        /* TODO: If there's no incoming message, you can do something here, or 
         * nothing.  Note, this section is dead code only if the iBlocking arg
         * is CFE_SB_PEND_FOREVER. */
        iStatus = CFE_SUCCESS;
    }
    else if (iStatus == CFE_SB_TIME_OUT)
    {
        /* TODO: If there's no incoming message within a specified time (via the
         * iBlocking arg, you can do something here, or nothing.  
         * Note, this section is dead code only if the iBlocking arg
         * is CFE_SB_PEND_FOREVER. */
        iStatus = CFE_SUCCESS;
    }
    else
    {
        (void) CFE_EVS_SendEvent(BAT_RCVMSG_ERR_EID, CFE_EVS_ERROR,
			  "SCH pipe read error (0x%08lX).", iStatus);
    }

    return iStatus;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process Incoming Commands                                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void BAT::ProcessCmdPipe()
{
    int32 iStatus = CFE_SUCCESS;
    CFE_SB_Msg_t*   CmdMsgPtr=NULL;
    CFE_SB_MsgId_t  CmdMsgId;

    /* Process command messages until the pipe is empty */
    while (1)
    {
        iStatus = CFE_SB_RcvMsg(&CmdMsgPtr, CmdPipeId, CFE_SB_POLL);
        if(iStatus == CFE_SUCCESS)
        {
            CmdMsgId = CFE_SB_GetMsgId(CmdMsgPtr);
            switch (CmdMsgId)
            {
                case BAT_CMD_MID:
                    ProcessAppCmds(CmdMsgPtr);
                    break;

                default:
                    /* Bump the command error counter for an unknown command.
                     * (This should only occur if it was subscribed to with this
                     *  pipe, but not handled in this switch-case.) */
                    HkTlm.usCmdErrCnt++;
                    (void) CFE_EVS_SendEvent(BAT_MSGID_ERR_EID, CFE_EVS_ERROR,
                                      "Recvd invalid CMD msgId (0x%04X)", (unsigned short)CmdMsgId);
                    break;
            }
        }
        else if (iStatus == CFE_SB_NO_MESSAGE)
        {
            break;
        }
        else
        {
            (void) CFE_EVS_SendEvent(BAT_RCVMSG_ERR_EID, CFE_EVS_ERROR,
                  "CMD pipe read error (0x%08lX)", iStatus);
            break;
        }
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process BAT Commands                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void BAT::ProcessAppCmds(CFE_SB_Msg_t* MsgPtr)
{
    uint32  uiCmdCode=0;

    if (MsgPtr != NULL)
    {
        uiCmdCode = CFE_SB_GetCmdCode(MsgPtr);
        switch (uiCmdCode)
        {
            case BAT_NOOP_CC:
                HkTlm.usCmdCnt++;
                (void) CFE_EVS_SendEvent(BAT_CMD_NOOP_EID, CFE_EVS_INFORMATION,
					"Recvd NOOP. Version %d.%d.%d.%d",
					BAT_MAJOR_VERSION,
					BAT_MINOR_VERSION,
					BAT_REVISION,
					BAT_MISSION_REV);
                break;

            case BAT_RESET_CC:
                HkTlm.usCmdCnt = 0;
                HkTlm.usCmdErrCnt = 0;
                break;

            default:
                HkTlm.usCmdErrCnt++;
                (void) CFE_EVS_SendEvent(BAT_CC_ERR_EID, CFE_EVS_ERROR,
                                  "Recvd invalid command code (%u)", (unsigned int)uiCmdCode);
                break;
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Send BAT Housekeeping                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void BAT::ReportHousekeeping()
{
    memcpy(&HkTlm.ActuatorArmed, &CVT.ActuatorArmed, sizeof(CVT.ActuatorArmed));
    memcpy(&HkTlm.ActuatorControls0, &CVT.ActuatorControls0, sizeof(CVT.ActuatorControls0));
    memcpy(&HkTlm.BatteryStatusMsg, &BatteryStatusMsg, sizeof(BatteryStatusMsg));
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&HkTlm);
    CFE_SB_SendMsg((CFE_SB_Msg_t*)&HkTlm);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Publish Output Data                                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BAT::SendBatteryStatusMsg()
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&BatteryStatusMsg);
    CFE_SB_SendMsg((CFE_SB_Msg_t*)&BatteryStatusMsg);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify Command Length                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
boolean BAT::VerifyCmdLength(CFE_SB_Msg_t* MsgPtr,
                           uint16 usExpectedLen)
{
    boolean bResult  = TRUE;
    uint16  usMsgLen = 0;

    if (MsgPtr != NULL)
    {
        usMsgLen = CFE_SB_GetTotalMsgLength(MsgPtr);

        if (usExpectedLen != usMsgLen)
        {
            bResult = FALSE;
            CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(MsgPtr);
            uint16 usCmdCode = CFE_SB_GetCmdCode(MsgPtr);

            (void) CFE_EVS_SendEvent(BAT_MSGLEN_ERR_EID, CFE_EVS_ERROR,
                              "Rcvd invalid msgLen: msgId=0x%08X, cmdCode=%d, "
                              "msgLen=%d, expectedLen=%d",
                              MsgId, usCmdCode, usMsgLen, usExpectedLen);
            HkTlm.usCmdErrCnt++;
        }
    }

    return bResult;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* BAT Application C style main entry point.                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
extern "C" void BAT_AppMain()
{
    oBAT.AppMain();
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* BAT Application C++ style main entry point.                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BAT::AppMain()
{
    /* Register the application with Executive Services */
    uiRunStatus = CFE_ES_APP_RUN;

    int32 iStatus = CFE_ES_RegisterApp();
    if (iStatus != CFE_SUCCESS)
    {
        (void) CFE_ES_WriteToSysLog("BAT - Failed to register the app (0x%08lX)\n", iStatus);
    }

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(BAT_MAIN_TASK_PERF_ID);

    /* Perform application initializations */
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = InitApp();
    }

    if (iStatus == CFE_SUCCESS)
    {
        /* Do not perform performance monitoring on startup sync */
        CFE_ES_PerfLogExit(BAT_MAIN_TASK_PERF_ID);
        CFE_ES_WaitForStartupSync(BAT_STARTUP_TIMEOUT_MSEC);
        CFE_ES_PerfLogEntry(BAT_MAIN_TASK_PERF_ID);
    }
    else
    {
        uiRunStatus = CFE_ES_APP_ERROR;
    }

    /* Application main loop */
    while (CFE_ES_RunLoop(&uiRunStatus) == TRUE)
    {
        RcvSchPipeMsg(BAT_SCH_PIPE_PEND_TIME);

        iStatus = AcquireConfigPointers();
        if(iStatus != CFE_SUCCESS)
        {
            /* We apparently tried to load a new table but failed.  Terminate the application. */
            uiRunStatus = CFE_ES_APP_ERROR;
        }
    }

    CloseDevice();

    StopChild();

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(BAT_MAIN_TASK_PERF_ID);

    /* Exit the application */
    CFE_ES_ExitApp(uiRunStatus);
}


void BAT::PublishBatteryStatus(void)
{
    OS_MutSemTake(Mutex);

    CFE_SB_SendMsg((CFE_SB_Msg_t*)&BatteryStatusMsg);

    OS_MutSemGive(Mutex);
}


bool  BAT::ChildContinueExec(void)
{
	bool result;

    OS_MutSemTake(Mutex);
    result = ChildContinueFlag;
    OS_MutSemGive(Mutex);

	return result;
}


void BAT::StopChild(void)
{
    OS_MutSemTake(Mutex);
    ChildContinueFlag = false;
    OS_MutSemGive(Mutex);
}


int32 BAT::InitListenerTask(void)
{
    int32 Status = CFE_SUCCESS;

    /* Create mutex for shared data */
    Status = OS_MutSemCreate(&Mutex, BAT_MUTEX_NAME, 0);
	if (Status != CFE_SUCCESS)
	{
		goto end_of_function;
	}

	ChildContinueFlag = true;

	Status= CFE_ES_CreateChildTask(&ListenerTaskID,
			BAT_LISTENER_TASK_NAME,
			BAT_ListenerTaskMain,
			NULL,
			BAT_LISTENER_TASK_STACK_SIZE,
			BAT_LISTENER_TASK_PRIORITY,
			0);
	if (Status != CFE_SUCCESS)
	{
		goto end_of_function;
	}

end_of_function:
	if (Status != CFE_SUCCESS)
	{
		CFE_EVS_SendEvent (BAT_LISTENER_CREATE_CHDTASK_ERR_EID,
						   CFE_EVS_ERROR,
						   "Listener child task failed.  CFE_ES_CreateChildTask returned: 0x%08lX",
						   Status);
	}

    return Status;
}

void BAT::ListenerTaskMain(void)
{
	int32 iStatus = CFE_SUCCESS;

	CFE_ES_RegisterChildTask();

	while(ChildContinueExec())
	{
		float voltage = 0.0f;
		float current = 0.0f;

		iStatus = ReadDevice(voltage, current);
		if(iStatus == CFE_SUCCESS)
		{
			CFE_TIME_SysTime_t msgTime;

		    OS_MutSemTake(Mutex);

		    voltage = voltage * ConfigTblPtr->VoltageScale;
		    current = current * ConfigTblPtr->CurrentScale;

		    BatteryStatusMsg.Voltage = voltage;
		    BatteryStatusMsg.VoltageFiltered = GetFilteredVoltage(voltage);
		    BatteryStatusMsg.Current = current;
		    BatteryStatusMsg.CurrentFiltered = GetFilteredCurrent(current);
		    BatteryStatusMsg.Discharged = GetDischarged(current);
		    BatteryStatusMsg.Remaining = GetRemaining(voltage, current,
		    		CVT.ActuatorControls0.Control[PX4_ACTUATOR_CONTROL_THROTTLE],
		    		CVT.ActuatorArmed.Armed);
		    BatteryStatusMsg.Scale = GetScale();
		    BatteryStatusMsg.CellCount = ConfigTblPtr->NumCells;
		    BatteryStatusMsg.Connected = true;
		    BatteryStatusMsg.Warning = GetWarningSeverity(BatteryStatusMsg.Remaining);

		    OS_MutSemGive(Mutex);
		}
	}

	CFE_ES_ExitChildTask();
}


float BAT::GetFilteredVoltage(float Voltage)
{
	float newVoltageFiltered = 0.0f;

	if(BatteryStatusMsg.VoltageFiltered < 0.0f)
	{
		newVoltageFiltered = Voltage;
	}
	else
	{
		newVoltageFiltered = BatteryStatusMsg.VoltageFiltered * 0.99f + Voltage * 0.01f;
	}

	return newVoltageFiltered;
}


float BAT::GetFilteredCurrent(float Current)
{
	float newCurrentFiltered = 0.0f;

	if(BatteryStatusMsg.CurrentFiltered < 0.0f)
	{
		newCurrentFiltered = Current;
	}
	else
	{
		newCurrentFiltered = BatteryStatusMsg.CurrentFiltered * 0.99f + Current * 0.01f;
	}

	return newCurrentFiltered;
}


float BAT::GetDischarged(float Current)
{
	CFE_TIME_SysTime_t zeroTime;
	CFE_TIME_SysTime_t msgTime;
	zeroTime.Seconds = 0;
	zeroTime.Subseconds = 0;

	if(Current < 0.0f)
	{
		/* Not a valid measurement.  Because the measurement was invalid we
		 * need to stop integration and re-init8ialize with the next valid
		 * measurement.
		 */
		SampleTime.Seconds = 0;
		SampleTime.Subseconds = 0;
	}

	/* Check to see if we have read a valid measurement yet.  If not, ignore
	 * the first measurement since we don't know dT yet.
	 */
	if(CFE_TIME_Compare(SampleTime, zeroTime) != CFE_TIME_EQUAL)
	{
		CFE_TIME_SysTime_t currentTime = CFE_TIME_GetMET();
		CFE_TIME_SysTime_t dSysTime = CFE_TIME_Subtract(currentTime, SampleTime);
		float dt = dSysTime.Seconds + (CFE_TIME_Sub2MicroSecs(dSysTime.Subseconds)* 0.000001);

		/* Integrate the current to calculate the new discharge, but calculate
		 * it using mAh units. */
		Discharged += Discharged + (Current * dt / 1000.0 / 3600.0f);

		/* Finally, set the message time to the current time. */
		SampleTime.Seconds = currentTime.Seconds;
		SampleTime.Subseconds = currentTime.Subseconds;
	}

	return Discharged;
}


float BAT::GetRemaining(float Voltage, float Current, float ThrottleNormalized, bool Armed)
{
	float remaining = 0.0f;
	float batVEmptyDynamic = 0.0f;
	float voltageRange = 0.0f;
	float rVoltage = 0.0f;
	float rVoltageFilt = 0.0f;
	float rCap = 0.0f;
	float rCapFilt = 0.0f;

	if(ConfigTblPtr->RInternal >= 0.0f)
	{
		batVEmptyDynamic = ConfigTblPtr->VEmpty - (Current * ConfigTblPtr->RInternal);
	}
	else
	{
		/* Assume 10% voltage drop of the full drop range with motors idle */
		float thr = (Armed) ? ((fabsf(ThrottleNormalized) + 0.1f) / 1.1f) : 0.0f;

		batVEmptyDynamic = ConfigTblPtr->VEmpty - (ConfigTblPtr->VLoadDrop * thr);
	}

	/* The range from full to empty is the same for batteries under load and
	 * without load, since the voltage drop applies to both the full and
	 * empty state.
	 */
	voltageRange = ConfigTblPtr->VFull - ConfigTblPtr->VEmpty;

	/* Remaining battery capacity based on voltage. */
	rVoltage = (Voltage - (ConfigTblPtr->NumCells * batVEmptyDynamic))
			/ (ConfigTblPtr->NumCells * voltageRange);
	rVoltageFilt = RemainingVoltage * 0.99f + rVoltage * 0.01f;
	RemainingVoltage = rVoltageFilt;

	/* Remaining battery capacity based on used current integrated time. */
	rCap = 1.0f - Discharged / ConfigTblPtr->Capacity;
	rCapFilt = RemainingCapacity * 0.99f + rCap * 0.01f;

	RemainingCapacity = rCapFilt;

	if(ConfigTblPtr->Capacity > 0.0f)
	{
		remaining = fminf(RemainingVoltage, RemainingCapacity);
	}
	else
	{
		remaining = RemainingVoltage;
	}

	return remaining;
}


PX4_BatteryWarningSeverity_t BAT::GetWarningSeverity(float Remaining)
{
	PX4_BatteryWarningSeverity_t severity = PX4_BATTERY_WARNING_NONE;

	if(Remaining < ConfigTblPtr->EmergencyThreshold)
	{
		severity = PX4_BATTERY_WARNING_EMERGENCY;
	}
	else if(Remaining < ConfigTblPtr->CriticalThreshold)
	{
		severity = PX4_BATTERY_WARNING_CRITICAL;
	}
	else if(Remaining < ConfigTblPtr->LowThreshold)
	{
		severity = PX4_BATTERY_WARNING_LOW;
	}

	return severity;
}



float BAT::GetScale(void)
{
	float scale = 0.0;
	float voltageRange = ConfigTblPtr->VFull - ConfigTblPtr->VEmpty;
	float batV = ConfigTblPtr->VEmpty + (voltageRange * RemainingVoltage);

	/* Reusing capacity calculation to get single cell voltage before drop. */
	scale = ConfigTblPtr->VFull / batV;

	if(scale > 1.3f)
	{
		/* Allow at most, 30% compensation */
		scale = 1.3f;
	}
	else if(scale < 1.0f)
	{
		/* Shouldn't ever be more than the power at full battery. */
		scale = 1.0f;
	}

	return scale;
}



extern "C" void BAT_ListenerTaskMain()
{
    oBAT.ListenerTaskMain();
}




/************************/
/*  End of File Comment */
/************************/
