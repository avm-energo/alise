/* This piece of code is stolen from ntpstate project (https://github.com/darkhelmet/ntpstat.git) */

#include "ntpstatus.h"

#include <QtDebug>
#include <sys/time.h>
#include <unistd.h>

NtpStatus::NtpStatus()
{
}

int NtpStatus::getNtpStatus()
{
    struct sockaddr_in sock;
    struct in_addr address;
    int sd; /* file descriptor for socket */
    fd_set fds;
    struct timeval tv;
    int n; /* number returned from select call */
    unsigned char byte1ok;
    unsigned char byte2ok;
    NtpMsgStruct ntpmsg;
    int8_t buff[PAYLOADSIZE]; /* temporary buffer holding payload string */

    uint8_t clksrc;

    /* initialise timeout value */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    /* initialise file descriptor set */
    FD_ZERO(&fds);

    inet_aton("127.0.0.1", &address);
    sock.sin_family = AF_INET;
    sock.sin_addr = address;
    sock.sin_port = htons(NTP_PORT);

    /*----------------------------------------------------------------*/
    /* Compose the command message */

    memset(&ntpmsg, 0, sizeof(ntpmsg));
    ntpmsg.byte1 = B1VAL;
    ntpmsg.byte2 = B2VAL;
    ntpmsg.sequence = htons(1);

    /*---------------------------------------------------------------------*/
    /* Send the command message */
    if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        qDebug() << "Ntp: unable to open socket";
        return ERROR;
    }

    qDebug() << "Ntp: Socket has been opened";

    if (connect(sd, (struct sockaddr *)&sock, sizeof(sock)) < 0)
    {
        qDebug() << "Ntp: unable to connect to socket: " << sock.sin_addr.s_addr << ":" << sock.sin_port;
        return ERROR;
    }
    qDebug() << "Ntp: Socket connected: " << sock.sin_addr.s_addr << ":" << sock.sin_port;
    FD_SET(sd, &fds);

    if (send(sd, &ntpmsg, sizeof(ntpmsg), 0) < 0)
    {
        qDebug() << "Ntp: unable to send command to NTP port";
        return ERROR;
    }
    qDebug() << "Ntp: query has been sent";

    /*----------------------------------------------------------------------*/
    /* Receive the reply message */
    n = select(sd + 1, &fds, (fd_set *)0, (fd_set *)0, &tv);

    if (n == 0)
    {
        qDebug() << "Ntp: select timeout";
        return ERROR;
    }
    if (n == -1)
    {
        qDebug() << "Ntp: error on select";
        return ERROR;
    }
    if (recv(sd, &ntpmsg, sizeof(NtpMsgStruct), 0) < 0)
    {
        qDebug() << "Ntp: Unable to talk to NTP daemon. Is it running?";
        return ERROR;
    }
    qDebug() << "Ntp: Data has been received";

    close(sd);

    qDebug() << "Ntp: Socket has been closed";
    /*----------------------------------------------------------------------*/
    /* Interpret the received NTP control message */
    // printf("NTP mode 6 message\n");
    // printf("cmd = %0x%0x\n",ntpmsg.byte1,ntpmsg.byte2);
    // printf("status = %0x%0x\n",ntpmsg.status1,ntpmsg.status2);
    // printf("AssocID = %0x\n",ntpmsg.AssocID);
    // printf("Offset = %0x\n",ntpmsg.Offset);
    // printf("%s\n\n",ntpmsg.payload);
    /* For the reply message to be valid, the first byte should be as sent,
       and the second byte should be the same, with the response bit set */
    byte1ok = ((ntpmsg.byte1 & 0x3F) == B1VAL);
    byte2ok = ((ntpmsg.byte2 & ~MMASK) == (B2VAL | RMASK));
    if (!(byte1ok && byte2ok))
    {
        qDebug() << "Ntp: return data appears to be invalid based on status word";
        return ERROR;
    }

    if (!(ntpmsg.byte2 | EMASK))
    {
        qDebug() << "Ntp: error bit is set in reply";
        return ERROR;
    }

    if (!(ntpmsg.byte2 | MMASK))
    {
        qDebug() << "Ntp: More bit unexpected in reply";
        return ERROR;
    }

    /* if the leap indicator (LI), which is the two most significant bits
       in status byte1, are both one, then the clock is not synchronised. */
    if ((ntpmsg.status1 >> 6) == 3)
    {
        qDebug() << "NTP is unsynchronised";
        return NO_SYNC;
    }
    else
    {
        clksrc = (ntpmsg.status1 & 0x3F);
        if (clksrc == LOCALNETSRC)
        {
            qDebug() << "NTP is synchronised to local source";
            return SYNC_LOCAL;
        }
        else if ((clksrc == NTPSRC) || (clksrc == LOCALNTPSRC))
        {
            qDebug() << "NTP is synchronised to ext source [" << clksrc << "]";
            return SYNC_EXT;
        }
        else
        {
            qDebug() << "NTP is synchronised to unknown source";
            return SYNC_EXT;
        }
    }
    return ERROR;
}
