#pragma once

#include <QObject>
#include <arpa/inet.h>
#include <sys/socket.h>

#define NTP_PORT 123

/* ------------------------------------------------------------------------*/
/* value of Byte1 and Byte2 in the ntpmsg       */
#define B1VAL 0x16 /* VN = 2, Mode = 6 */
#define B2VAL 2
// Response = 0; ( this makes the packet a command )
// Error    = 0;
// More     = 0;
// Op Code  = 2 (read variables command)
#define RMASK 0x80 /* bit mask for the response bit in Status Byte2 */
#define EMASK 0x40 /* bit mask for the error bit in Status Byte2 */
#define MMASK 0x20 /* bit mask for the more bit in Status Byte2 */
#define LOCALNETSRC 5
#define NTPSRC 6
#define LOCALNTPSRC 0
/*-------------------------------------------------------------------------*/
#define PAYLOADSIZE 468 /* size in bytes of the message payload string */

class NtpStatus
{
public:
    enum NtpStatusEnum
    {
        NO_SYNC = 1,
        SYNC_EXT = 2,
        SYNC_LOCAL = 3,
        ERROR = 4
    };

    NtpStatus();
    int getNtpStatus();

private:

    struct NtpMsgStruct
    {                  /* RFC-1305 NTP control message format */
        uint8_t byte1; /* Version Number: bits 3 - 5; Mode: bits 0 - 2; */
        uint8_t byte2; /* Response: bit 7;
                               Error: bit 6;
                               More: bit 5;
                               Op code: bits 0 - 4 */
        uint16_t sequence;
        uint8_t status1; /* LI and clock source */
        uint8_t status2; /* count and code */
        uint16_t AssocID;
        uint16_t Offset;
        uint16_t Count;
        int8_t payload[PAYLOADSIZE];
        int8_t authenticator[96];
    };
};
