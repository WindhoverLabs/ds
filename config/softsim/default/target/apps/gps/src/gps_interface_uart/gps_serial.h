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

#ifndef GPS_SERIAL_H
#define GPS_SERIAL_H
/************************************************************************
** Includes
*************************************************************************/
#include "gps_custom.h"
#include "../gps_custom_shared.h"
#include "cfe.h"

#ifdef __cplusplus
extern "C" {
#endif
/************************************************************************
** Local Defines
*************************************************************************/
/** \brief GPS device path. */
#define GPS_SERIAL_DEVICE_PATH                    "/dev/ttyS3"

/** \brief IO speed.
**
**  \par Description:
**       The serial IO speed 38400 baud.
*/
#define GPS_SERIAL_IO_SPEED                       38400

/** \brief Wait time (ms) before read.
**
**  \par Description:
**       ms, wait before reading to save read() calls.
*/
#define GPS_WAIT_BEFORE_READ                      (20)

/** \brief GPS packet timeout (ms).
**
**  \par Description:
**       ms, timeout to receive packet.
*/
#define GPS_PACKET_TIMEOUT                        (20)

/** \brief GPS ack timeout.
**
**  \par Description:
**       ms, timeout to wait for ack.
*/
#define GPS_ACK_TIMEOUT                           (200)


/** \brief GPS payload scanner buffer size.
**
**  \par Description:
**       GPS payload scanner length.
*/
//#define GPS_SCANNER_BUFFER_LENGTH              (1024)

/* Message Classes */
/** \brief Configuration input class.
**
**  \par Description:
**       UBX class ID configuration.
*/
#define GPS_MESSAGE_CLASS_CFG                     (0x06)

/* Message IDs */
/** \brief Message ID port configuration.
**
**  \par Description:
**       Message ID for port configuration.
*/
#define GPS_MESSAGE_ID_CFG_PRT                    (0x00)

/* TX CFG-PRT message contents */
/** \brief UART 1 port number.
**
**  \par Description:
**       Port interface number.
*/
#define GPS_TX_CFG_PRT_PORTID                     (0x01)

/** \brief Mode bitfield configuration.
**
**  \par Description:
**       0b0000100011010000: 8N1
*/
#define GPS_TX_CFG_PRT_MODE                       (0x000008D0)

/** \brief Input protocol configuration.
**
**  \par Description:
**       UBX in.
*/
#define GPS_TX_CFG_PRT_INPROTOMASK_GPS            (0x01)

/** \brief Output protocol configuration.
**
**  \par Description:
**       UBX out.
*/
#define GPS_TX_CFG_PRT_OUTPROTOMASK_GPS           (0x01)

/** \brief USB port number.
**
**  \par Description:
**       Port interface number.
*/
#define GPS_TX_CFG_PRT_PORTID_USB                 (0x03)

/* UBX header contents */
/** \brief Header symbol 1.
**
**  \par Description:
**       UBX header symbol 1.
*/
#define GPS_HEADER_SYNC1_VALUE                    (0xb5)

/** \brief Header symbol 2.
**
**  \par Description:
**       UBX header symbol 2.
*/
#define GPS_HEADER_SYNC2_VALUE                    (0x62)

/* Message Classes & IDs */
#define GPS_MESSAGE_CFG_PRT          ((GPS_MESSAGE_CLASS_CFG) | \
                                      GPS_MESSAGE_ID_CFG_PRT << 8)

/** \brief Retry attemps for interrupted calls.
**
**  \par Limits:
**       None.
*/
#define GPS_MAX_RETRY_ATTEMPTS                     (2)

/** \brief Sleep time micro seconds for interrupted calls.
**
**  \par Limits:
**       None.
*/
#define GPS_MAX_RETRY_SLEEP_USEC                   (10)

/************************************************************************
** Structure Declarations
*************************************************************************/

/**
 * \brief GPS port configuration payload message.
 */
typedef struct 
{
    /*! Port Identifier Number */
    uint8       portID;
    /*! Reserved */
    uint8       reserved1;
    /*! TX ready PIN configuration */
    uint16      txReady;
    /*! A bit mask describing the UART mode */
    uint32      mode;
    /*! Baud rate in bits/second */
    uint32      baudRate;
    /*! A mask describing which input protocols are active. */
    uint16      inProtoMask;
    /*! A mask describing which output protocols are active. */
    uint16      outProtoMask;
    /*! Flags bit mask */
    uint16      flags;
    /*! Reserved */
    uint16      reserved2;
} GPS_Payload_TX_CFG_PRT_t;

/**
 * \brief GPS UBX protocol header.
 */
typedef struct 
{
    /*! Sync symbol 1 */
    uint8       sync1;
    /*! Sync symbol 1 */
    uint8       sync2;
    /*! Message */
    uint16      msg;
    /*! Message length */
    uint16      length;
} GPS_Header_t;

/**
 * \brief GPS UBX protocol checksum.
 */
typedef struct 
{
    /*! checksum check A*/
    uint8       ck_a;
    /*! checksum check B*/
    uint8       ck_b;
} GPS_Checksum_t;


/**
 * \brief GPS device status
 */
typedef enum
{
    /*! GPS status uninitialized */
    GPS_CUSTOM_UNINITIALIZED  = 0,
    /*! GPS status initialized */
    GPS_CUSTOM_INITIALIZED   = 1
} GPS_Custom_Status_t;


typedef enum 
{
    GPS_ACK_IDLE = 0,
    GPS_ACK_WAITING,
    GPS_ACK_GOT_ACK,
    GPS_ACK_GOT_NAK
} GPS_Ack_State_t;


typedef struct
{
    /*! Device file descriptor */
    int                          DeviceFd;
    /*! The current device status */
    GPS_Custom_Status_t          Status;
    /*! The current baud */
    uint32                       Baud;
    /*! The current parser status */
    GPS_ParserStatus_t           ParserStatus;
    
    GPS_Ack_State_t              AckState;
    uint16                       AckWaitingMsg;
    boolean                      AckWaitingRcvd;
} GPS_AppCustomData_t;


/************************************************************************
** External Global Variables
*************************************************************************/
extern GPS_AppCustomData_t GPS_AppCustomData;

/************************************************************************
** Function Prototypes
*************************************************************************/
/************************************************************************/
/** \brief Negotiate GPS baud rate. 
**
**  \par Description
**       This function tries different baud rates to initially establish
**       communication and attempts to set the baud rate specified.
**
**  \par Assumptions, External Events, and Notes:
**       A device must be initialized before this function is called.
**
**  \param [out]    BaudRateSet    The baud rate that was set.
**
**  \param [in]     Baud           The baud rate to attempt to set.                             
**
**  \returns    boolean, TRUE for success, FALSE for failure.
**
*************************************************************************/
boolean GPS_Custom_Negotiate_Baud(uint32 *BaudRateSet, const uint32 Baud);

/************************************************************************/
/** \brief Set baud rate. 
**
**  \par Description
**       This function attempts to set the baud rate for the opened
**       device.
**
**  \par Assumptions, External Events, and Notes:
**       A device must be initialized before this function is called.
**
**  \param [in]     Baud           The baud rate to attempt to set.
**
**  \returns    boolean, TRUE for success, FALSE for failure.
**
*************************************************************************/
boolean GPS_Custom_Set_Baud(const uint32 Baud);


/************************************************************************/
/** \brief Sends a message to the GPS device.
**
**  \par Description
**       This function attempts to send a message to the GPS device.
**
**  \par Assumptions, External Events, and Notes:
**       A device must be initialized and baudrate configured before
**       this function is called.
**
**  \param [in]    msg          The message id.
**
**  \param [in]    payload      The message payload.
**
**  \param [in]    length       The message length.
**
**  \returns    boolean, TRUE for success, FALSE for failure.
**
*************************************************************************/
boolean GPS_Custom_SendMessage(const uint16 msg, const uint8 *payload, const uint16 length);


/************************************************************************/
/** \brief Set a checksum.
**
**  \par Description
**       This function calculates and populates the checksum structure.
**
**  \par Assumptions, External Events, and Notes:
**       None.
**
**  \param [in]    buffer       The buffer to use for checksum 
**                              calculation.
**
**  \param [in]    length       The buffer length.
**
**  \param [out]   checksum     The checksum structure to populate with 
**                              the calculated checksum.
**
**  \returns    boolean, TRUE for success, FALSE for failure.
**
*************************************************************************/
boolean GPS_Custom_SetChecksum(const uint8 *buffer, const uint16 length, GPS_Checksum_t *checksum);


/************************************************************************/
/** \brief Receive data from the GPS device.
**
**  \par Description
**       This receives data by calling select and read.
**
**  \par Assumptions, External Events, and Notes:
**       None.
**
**  \param [out]    buffer      The output buffer.
**
**  \param [in]    length       The buffer length to attempt to read.
**
**  \param [in]    Timeout      The timeout for select.
**
**  \returns       int32,       returns bytes read if successful, -1 or
**                              for failure. Select or read will set
**                              errno.
**
*************************************************************************/
int32 GPS_Custom_Receive(uint8 *Buffer, uint32 Length, uint32 Timeout);


/************************************************************************/
/** \brief Call select and wait for data from the GPS device.
**
**  \par Description
**       This function calls select with the timeout specified.
**
**  \par Assumptions, External Events, and Notes:
**       None.
**
**  \param [in]    TimeoutSec    The timeout in seconds.
**
**  \param [in]    TimeoutUSec   The timeout in microseconds.
**
**  \returns       int32,        returns greater than 0 for success, 0
**                               for timeout, negative value for error.
**
*************************************************************************/
int32 GPS_Custom_Select(const uint32 TimeoutSec, const uint32 TimeoutUSec);

/************************************************************************/
/** \brief Wait for an acknowledgement.
**
**  \par Description
**       This function waits for a specified period for a message.
**
**  \par Assumptions, External Events, and Notes:
**       None.
**
**  \param [in]    msg           The message to wait for an ack for.
**
**  \param [in]    timeout       The timeout to wait.
**
**  \returns       boolean, TRUE for success, FALSE for failure.
**
*************************************************************************/
boolean GPS_Custom_WaitForAck(const uint16 msg, const uint32 timeout);


/************************************************************************/
/** \brief Get time in from GPS_Custom_Get_Time in uint64 format.
**
**  \par Description
**       This function is a wrapper for GPS_Custom_Get_Time.
**
**  \par Assumptions, External Events, and Notes:
**       None.
**
**  \returns       uint64, monotonic time for success, 0 for failure.
**
*************************************************************************/
uint64 GPS_Custom_Get_Time_Uint64(void);

#ifdef __cplusplus
}
#endif 

#endif /* GPS_SERIAL_H */
