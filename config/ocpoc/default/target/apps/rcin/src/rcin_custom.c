/****************************************************************************
*
*   Copyright (c) 2017 Windhover Labs, L.L.C. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
* 3. Neither the name Windhover Labs nor the names of its 
*    contributors may be used to endorse or promote products derived 
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/
/************************************************************************
** Pragmas
*************************************************************************/

/************************************************************************
** Includes
*************************************************************************/
#include "rcin_sbus.h"
#include "rcin_events.h"

/************************************************************************
** Local Defines
*************************************************************************/

/************************************************************************
** Local Structure Declarations
*************************************************************************/

typedef enum {

/** \brief <tt> 'RCIN - ' </tt>
**  \event <tt> 'RCIN - ' </tt>
**  
**  \par Type: ERROR
**
**  \par Cause:
**
**  This event message is issued when a device resource encounters an 
**  error.
**
*/
    RCIN_DEVICE_ERR_EID = RCIN_EVT_CNT,

/** \brief Number of custom events 
**
**  \par Limits:
**       int32
*/
    RCIN_CUSTOM_EVT_CNT

} RCIN_CustomEventIds_t;

/************************************************************************
** External Global Variables
*************************************************************************/

/************************************************************************
** Global Variables
*************************************************************************/
RCIN_AppCustomData_t RCIN_AppCustomData;
/************************************************************************
** Local Variables
*************************************************************************/

/************************************************************************
** Local Function Definitions
*************************************************************************/
int32 RCIN_Ioctl(int fh, int request, void *arg)
{
    int32 returnCode = 0;
    uint32 i = 0;

    for (i=0; i < RCIN_MAX_RETRY_ATTEMPTS; i++)
    {
        returnCode = ioctl(fh, request, arg);
            
        if (-1 == returnCode && EINTR == errno)
        {
            usleep(RCIN_MAX_RETRY_SLEEP_USEC);
        }
        else
        {
            break;
        }
    }

    return returnCode;
}


int32 RCIN_Read(int fd, void *buf, size_t count)
{
    int32 returnSize = 0;
    uint32 i = 0;
    
    for (i=0; i < RCIN_MAX_RETRY_ATTEMPTS; i++)
    {
        returnSize = read(fd, buf, count);
        /* Interrupted */
        if (-1 == returnSize && EINTR == errno)
        {
            CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
            "RCIN device read EINTR");
            usleep(RCIN_MAX_RETRY_SLEEP_USEC);
        }
        else
        {
            break;
        }
    }

    return returnSize;
}


void RCIN_Custom_InitData(void)
{
    /* Set all struct zero values */
    bzero(&RCIN_AppCustomData, sizeof(RCIN_AppCustomData));
    strncpy(RCIN_AppCustomData.DevName, RCIN_DEVICE_PATH, RCIN_MAX_DEVICE_PATH);
    /* Set all non-zero values */
    RCIN_AppCustomData.ContinueFlag          = TRUE;
    RCIN_AppCustomData.Priority              = RCIN_STREAMING_TASK_PRIORITY;
    RCIN_AppCustomData.StreamingTask         = RCIN_Stream_Task;
}


boolean RCIN_Custom_Init(void)
{
    boolean returnBool = TRUE;
    int32 returnCode = 0;
    
    /* Open */
    RCIN_AppCustomData.DeviceFd = open(RCIN_DEVICE_PATH, O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (RCIN_AppCustomData.DeviceFd < 0) 
    {
        CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
            "RCIN custom device open errno: %i", errno);
        returnBool = FALSE;
        goto end_of_function;
    }
    else
    {
        RCIN_AppCustomData.Status = RCIN_CUSTOM_INITIALIZED;
    }

    /* Create mutex for shared data */
    returnCode = OS_MutSemCreate(&RCIN_AppCustomData.Mutex, RCIN_MUTEX_NAME, 0);
    if (returnCode != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
            "RCIN mutex create failed in custom init");
        returnBool = FALSE;
        goto end_of_function;
    }

    /* Get configuration */
    returnCode = RCIN_Ioctl(RCIN_AppCustomData.DeviceFd, TCGETS2, 
            &RCIN_AppCustomData.TerminalConfig);
    if (0 != returnCode)
    {
        CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
            "RCIN custom get terminal config failed errno: %i", errno);
        goto end_of_function;
    }
    
    /* Setting serial port, 8E2, non-blocking. 100Kbps */
    /* https://linux.die.net/man/3/termios */
    RCIN_AppCustomData.TerminalConfig.c_iflag &= ~(IGNBRK | BRKINT | 
            PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    RCIN_AppCustomData.TerminalConfig.c_oflag &= ~OPOST;
    RCIN_AppCustomData.TerminalConfig.c_lflag &= ~(ECHO | ECHONL | 
            ICANON | ISIG | IEXTEN);

    RCIN_AppCustomData.TerminalConfig.c_cflag &= ~(CSIZE | CRTSCTS | 
            PARODD | CBAUD);
    RCIN_AppCustomData.TerminalConfig.c_iflag |= (INPCK | IGNPAR);
    /* Use BOTHER to specify speed directly in c_[io]speed member */
    RCIN_AppCustomData.TerminalConfig.c_cflag |= (CS8 | CSTOPB | CLOCAL 
            | PARENB | BOTHER | CREAD);
    RCIN_AppCustomData.TerminalConfig.c_ispeed = RCIN_SERIAL_INPUT_SPEED;
    RCIN_AppCustomData.TerminalConfig.c_ospeed = RCIN_SERIAL_OUTPUT_SPEED;
    RCIN_AppCustomData.TerminalConfig.c_cc[VMIN] = RCIN_SERIAL_VMIN_SETTING;
    RCIN_AppCustomData.TerminalConfig.c_cc[VTIME] = RCIN_SERIAL_VTIME_SETTING;
    
    returnCode = RCIN_Ioctl(RCIN_AppCustomData.DeviceFd, TCSETS2, 
            &RCIN_AppCustomData.TerminalConfig);
    if (0 != returnCode)
    {
        CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
            "RCIN custom set terminal config failed errno: %i", errno);
        goto end_of_function;
    }
    
    /* Create streaming task */
    /* Set loop flag to continue forever */
    RCIN_AppCustomData.ContinueFlag = TRUE;
    
        /* Create the streaming task */
    returnCode = CFE_ES_CreateChildTask(
        &RCIN_AppCustomData.ChildTaskID,
        RCIN_STREAMING_TASK_NAME,
        RCIN_AppCustomData.StreamingTask,
        0,
        CFE_ES_DEFAULT_STACK_SIZE,
        RCIN_AppCustomData.Priority,
        0);
        
    if(CFE_SUCCESS != returnCode)
    {
        RCIN_AppCustomData.ContinueFlag = FALSE;
        returnBool = FALSE;
        CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
            "RCIN create streaming task failed");
        goto end_of_function;
    }
    else
    {
        RCIN_AppCustomData.Status = RCIN_CUSTOM_ENABLED;
    }

end_of_function:
    return returnBool;
}


void RCIN_Stream_Task(void)
{
    int32 returnCode = 0;
    int32 timeouts = 0;
    uint32 maxFd = 0;
    uint32 retryAttempts = 0;
    fd_set fds;
    
    struct timeval timeValue;
    uint32 iStatus = -1;
    
    iStatus = CFE_ES_RegisterChildTask();
    
    if (iStatus == CFE_SUCCESS)
    {
        while (RCIN_AppCustomData.ContinueFlag == TRUE)
        {
            maxFd = 0;
            returnCode = 0;
            
            /* Select modifies the timeout value with time left until 
             * the timeout would expire so timeValue needs to be set
             * every loop iteration
             */
            timeValue.tv_sec = RCIN_BUFFER_FILL_TIMEOUT_SEC;
            timeValue.tv_usec = RCIN_BUFFER_FILL_TIMEOUT_USEC;

            /* Initialize the set */
            FD_ZERO(&fds);

            FD_SET(RCIN_AppCustomData.DeviceFd, &fds);
            
            /* Get the greatest fd value for select() */
            maxFd = RCIN_AppCustomData.DeviceFd; 

            //CFE_ES_PerfLogEntry(RCIN_DEVICE_GET_PERF_ID);
            /* Wait for RC data */
            returnCode = select(maxFd + 1, &fds, 0, 0, &timeValue);
            //CFE_ES_PerfLogExit(RCIN_DEVICE_GET_PERF_ID);

            /* select() wasn't successful */
            if (-1 == returnCode)
            {
                /* select was interrupted, try again */
                if (EINTR == errno)
                {
                    if (retryAttempts == RCIN_MAX_RETRY_ATTEMPTS)
                    {
                        goto end_of_function;
                    }
                    retryAttempts++;
                    usleep(RCIN_MAX_RETRY_SLEEP_USEC);
                    CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "RCIN select was interrupted");
                    continue;
                }
                else
                {
                    /* select returned an error other than EINTR */
                    CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
                        "RCIN stream failed select() returned %i", errno);
                    goto end_of_function;
                }
            }
            /* select timed out */
            if (0 == returnCode)
            {
                OS_MutSemTake(RCIN_AppCustomData.Mutex);
                RCIN_AppCustomData.Status = RCIN_CUSTOM_NOTSTREAMING;
                OS_MutSemGive(RCIN_AppCustomData.Mutex); 
                continue;
            } 
            /* select() returned and data is ready to be read */
            if(returnCode > 0)
            {
                OS_MutSemTake(RCIN_AppCustomData.Mutex);
                RCIN_AppCustomData.Status = RCIN_CUSTOM_STREAMING;
                RCIN_Custom_Read();
                OS_MutSemGive(RCIN_AppCustomData.Mutex); 

            }    
        } /* end while loop */
    } /* end if status == success */

end_of_function:

    /* Streaming task is exiting so set app flag to initialized */
    RCIN_AppCustomData.Status = RCIN_CUSTOM_ENABLED;
    CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
        "RCIN streaming task exited with return code %li task status (0x%08lX)",
        returnCode, iStatus);

    /* The child task was successfully created so exit from it */
    if (iStatus == CFE_SUCCESS)
    {
        CFE_ES_ExitChildTask();
    }
}


void RCIN_Custom_Read(void)
{
    int bytesRead = 0;
    uint8 sbusData[RCIN_SERIAL_READ_SIZE] = {
                                        0x0f, 0x01, 0x04, 0x20, 0x00,
                                        0xff, 0x07, 0x40, 0x00, 0x02,
                                        0x10, 0x80, 0x2c, 0x64, 0x21,
                                        0x0b, 0x59, 0x08, 0x40, 0x00,
                                        0x02, 0x10, 0x80, 0x00, 0x00 };

    boolean syncSuccess = FALSE;

    /* Timestamp */
    RCIN_AppCustomData.Measure.Timestamp = RCIN_Custom_Get_Time();

    /* Read */
    bytesRead = RCIN_Read(RCIN_AppCustomData.DeviceFd, &sbusData, sizeof(sbusData));
    
    OS_printf("RCIN bytesRead %d\n", bytesRead);

    /* If a partial read occurs need to resync */
    //if (bytesRead > 0 && bytesRead < RCIN_SERIAL_READ_SIZE) 
    //{
        //CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
            //"RCIN out of sync partial read, resyncing");
        //syncSuccess = RCIN_Custom_Sync(&sbusData, bytesRead);
        //if(TRUE == syncSuccess)
        //{
            //CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
                //"RCIN back in sync from partial read");
        //}
        //else
        //{
            //goto end_of_function;
        //}
    //}
    //else if (RCIN_SERIAL_READ_SIZE == bytesRead)
    if (RCIN_SERIAL_READ_SIZE == bytesRead)
    {
        /* Validate SBUS header and end byte */
        if (0x0f != sbusData[0] && 0x00 != sbusData[24]) 
        {
            CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
                "RCIN device out of sync error");
            //syncSuccess = RCIN_Custom_Sync(&sbusData, bytesRead);
            if(TRUE == syncSuccess)
            {
                CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
                    "RCIN back in sync");
            }
        }
    }
    else
    {
        CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
                "RCIN device read error");
    }

    RCIN_AppCustomData.Measure.Values[0] = (uint16)(((sbusData[1] | sbusData[2] << 8) 
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[1] = (uint16)(((sbusData[2] >> 3 | sbusData[3] << 5)
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[2] = (uint16)(((sbusData[3] >> 6 | sbusData[4] << 2
            | sbusData[5] << 10) & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f)
            + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[3] = (uint16)(((sbusData[5] >> 1 | sbusData[6] << 7)
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[4] = (uint16)(((sbusData[6] >> 4 | sbusData[7] << 4)
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[5] = (uint16)(((sbusData[7] >> 7 | sbusData[8] << 1
            | sbusData[9] << 9) & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f)
            + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[6] = (uint16)(((sbusData[9] >> 2 | sbusData[10] << 6)
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[7] = (uint16)(((sbusData[10] >> 5 | sbusData[11] << 3)
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET; // & the other 8 + 2 channels if you need them
    RCIN_AppCustomData.Measure.Values[8] = (uint16)(((sbusData[12] | sbusData[13] << 8)
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[9] = (uint16)(((sbusData[13] >> 3 | sbusData[14] << 5)
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[10] = (uint16)(((sbusData[14] >> 6 | sbusData[15] << 2
            | sbusData[16] << 10) & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f)
            + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[11] = (uint16)(((sbusData[16] >> 1 | sbusData[17] << 7)
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[12] = (uint16)(((sbusData[17] >> 4 | sbusData[18] << 4)
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[13] = (uint16)(((sbusData[18] >> 7 | sbusData[19] << 1
            | sbusData[20] << 9) & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f)
            + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[14] = (uint16)(((sbusData[20] >> 2 | sbusData[21] << 6)
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET;
    RCIN_AppCustomData.Measure.Values[15] = (uint16)(((sbusData[21] >> 5 | sbusData[22] << 3)
            & 0x07FF) * RCIN_SBUS_SCALE_FACTOR + .5f) + RCIN_SBUS_SCALE_OFFSET;

    /* Channel count */
    RCIN_AppCustomData.Measure.ChannelCount = RCIN_SBUS_CHANNEL_COUNT;
    RCIN_AppCustomData.Measure.RSSI = 100;
    // For now handle outside measure function call.
    //Measure->RcLostFrameCount = errorCount;
    RCIN_AppCustomData.Measure.RcTotalFrameCount = 1;
    RCIN_AppCustomData.Measure.RcPpmFrameLength = 0;
    RCIN_AppCustomData.Measure.RcFailsafe = (sbusData[23] & (1 << 3)) ? TRUE : FALSE;
    RCIN_AppCustomData.Measure.RcLost = (sbusData[23] & (1 << 2)) ? TRUE : FALSE;
    RCIN_AppCustomData.Measure.InputSource = PX4_RC_INPUT_SOURCE_PX4IO_SBUS;
}


//boolean RCIN_Custom_Sync(uint8 *data, int size)
//{
    //uint32 i = 0;
    //int bytesRemaining = 0;
    //uint8 sbusTemp[RCIN_SERIAL_READ_SIZE] = {0};
    //boolean headerFound = FALSE;
    //boolean returnBool = TRUE;
    
    //if(0 == data)
    //{
        //CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
                    //"RCIN_Custom_Sync null pointer");
        //returnBool = FALSE;
        //goto end_of_function;
    //}
    
     ///* Find the SBUS header */
    //for(i = 0; i < size - 1; i++)
    //{
        //if(0x0f == data[i])
        //{
            ///* Determine how many bytes remain */
            //bytesRemaining = RCIN_SERIAL_READ_SIZE - (i + 1);
            ///* Read bytes remaining to sync */
            //bytesRemaining = RCIN_Read(RCIN_AppCustomData.DeviceFd, &sbusTemp, bytesRemaining);
            //headerFound = TRUE;
            //break;
        //}
        ///* For now try again on the next read */
        //if (FALSE == headerFound)
        //{
            //CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
                    //"RCIN_Custom_Sync header not found");
            //returnBool = FALSE;
            //goto end_of_function;
        //}
    //}
    //if (RCIN_SERIAL_READ_SIZE == byteRemaining + i + 1)
    //{
        ///* shift messaage */
        //memcpy(data, data[i], (i + 1));
        //memcpy(data[i+1], sbusTemp, bytesRemaining);
    //}
//end_of_function:
    //return returnBool;
//}


boolean RCIN_Custom_Measure(PX4_InputRcMsg_t *Measure)
{
    boolean returnBool = TRUE;

    /* Null pointer check */
    if (0 == Measure)
    {
        CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
            "RCIN device read null pointer error");
        returnBool = FALSE;
        goto end_of_function;
    }

    OS_MutSemTake(RCIN_AppCustomData.Mutex);
    if(RCIN_CUSTOM_STREAMING == RCIN_AppCustomData.Status)
    {
        memcpy(Measure, &RCIN_AppCustomData.Measure, sizeof(PX4_InputRcMsg_t));
    }
    else
    {
        returnBool = FALSE;
    }
    OS_MutSemGive(RCIN_AppCustomData.Mutex);

end_of_function:
    return returnBool;
}


CFE_TIME_SysTime_t RCIN_Custom_Get_Time(void)
{
    struct timespec ts;
    int returnCode = 0;
    CFE_TIME_SysTime_t Timestamp = {0, 0};

    returnCode = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (-1 == returnCode)
    {
        CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
            "RCIN clock_gettime errno: %i", errno);
        goto end_of_function;
    }

end_of_function:
    return Timestamp;
}


boolean RCIN_Custom_Uninit(void)
{
    boolean returnBool = TRUE;
    int returnCode = 0;

    /* Delete the child task */
    CFE_ES_DeleteChildTask(RCIN_AppCustomData.ChildTaskID);
    /* Set streaming task loop flag to stop */
    RCIN_AppCustomData.ContinueFlag = FALSE;
    /* Set app state to initialized */
    RCIN_AppCustomData.Status = RCIN_CUSTOM_INITIALIZED;
    
    returnCode = close(RCIN_AppCustomData.DeviceFd);
    if (-1 == returnCode) 
    {
        CFE_EVS_SendEvent(RCIN_DEVICE_ERR_EID, CFE_EVS_ERROR,
            "RCIN Device close errno: %i", errno);
        returnBool = FALSE;
    }
    else
    {
        RCIN_AppCustomData.Status = RCIN_CUSTOM_UNINITIALIZED;
    }
    return returnBool;
}


boolean RCIN_Custom_Max_Events_Not_Reached(int32 ind)
{
    if ((ind < CFE_EVS_MAX_EVENT_FILTERS) && (ind > 0))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


int32 RCIN_Custom_Init_EventFilters(int32 ind, CFE_EVS_BinFilter_t *EventTbl)
{
    int32 customEventCount = ind;
    
    /* Null check */
    if(0 == EventTbl)
    {
        customEventCount = -1;
        goto end_of_function;
    }

    if(TRUE == RCIN_Custom_Max_Events_Not_Reached(customEventCount))
    {
        EventTbl[  customEventCount].EventID = RCIN_DEVICE_ERR_EID;
        EventTbl[customEventCount++].Mask    = CFE_EVS_FIRST_16_STOP;
    }
    else
    {
        customEventCount = -1;
        goto end_of_function;
    }
    
end_of_function:

    return customEventCount;
}
