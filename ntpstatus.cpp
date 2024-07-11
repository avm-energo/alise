/* This piece of code is stolen from ntpstate project (https://github.com/darkhelmet/ntpstat.git) */

#include "ntpstatus.h"

#include <QtDebug>
#include <sys/time.h>
#include <unistd.h>

NtpStatus::NtpStatus()
{
    /*----------------------------------------------------------------*/
    /* Compose the command message */

    memset(&ntpmsg, 0, sizeof(ntpmsg));
    ntpmsg.byte1 = B1VAL;
    ntpmsg.byte2 = B2VAL;
    ntpmsg.sequence = htons(1);

    inet_aton("127.0.0.1", &address);
    sock.sin_family = AF_INET;
    sock.sin_addr = address;
    sock.sin_port = htons(NTP_PORT);

    /* initialise timeout value */
    tv.tv_sec = 1;
    tv.tv_usec = 0;
}

int NtpStatus::getNtpStatus()
{
    /* initialise file descriptor set */
    FD_ZERO(&fds);

    /*---------------------------------------------------------------------*/
    /* Send the command message */
    if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        qDebug() << "unable to open socket";
        return ERROR;
    }

    if (connect(sd, (struct sockaddr *)&sock, sizeof(sock)) < 0)
    {
        qDebug() << "unable to connect to socket";
        return ERROR;
    }
    FD_SET(sd, &fds);

    if (send(sd, &ntpmsg, sizeof(ntpmsg), 0) < 0)
    {
        qDebug() << "unable to send command to NTP port";
        return ERROR;
    }
    /*----------------------------------------------------------------------*/
    /* Receive the reply message */
    n = select(sd + 1, &fds, (fd_set *)0, (fd_set *)0, &tv);

    if (n == 0)
    {
        qDebug() << "timeout";
        return ERROR;
    }
    if (n == -1)
    {
        qDebug() << "error on select";
        return ERROR;
    }
    if (recv(sd, &ntpmsg, sizeof(ntpmsg), 0) < 0)
    {
        qDebug() << "Unable to talk to NTP daemon. Is it running?";
        return ERROR;
    }

    close(sd);

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
        qDebug() << "return data appears to be invalid based on status word";
        return ERROR;
    }

    if (!(ntpmsg.byte2 | EMASK))
    {
        qDebug() << "error bit is set in reply";
        return ERROR;
    }

    if (!(ntpmsg.byte2 | MMASK))
    {
        qDebug() << "More bit unexpected in reply";
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
