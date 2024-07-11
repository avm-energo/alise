/* This piece of code is stolen from ntpstate project (https://github.com/darkhelmet/ntpstat.git) */

#include "ntpstatus.h"

#include <QtDebug>
#include <sys/time.h>
#include <unistd.h>

NtpStatus::NtpStatus()
{
    struct in_addr address;
    /*----------------------------------------------------------------*/
    /* Compose the command message */

    memset(&m_ntpmsg, 0, sizeof(NtpMsgStruct));
    m_ntpmsg.byte1 = B1VAL;
    m_ntpmsg.byte2 = B2VAL;
    m_ntpmsg.sequence = htons(1);

    inet_aton("127.0.0.1", &address);
    m_sock.sin_family = AF_INET;
    m_sock.sin_addr = address;
    m_sock.sin_port = htons(NTP_PORT);
}

int NtpStatus::getNtpStatus()
{
    int n; /* number returned from select call */
    fd_set fds;
    uint8_t byte1ok;
    uint8_t byte2ok;
    int sd; /* file descriptor for socket */
    struct timeval tv;

    /* initialise file descriptor set */
    FD_ZERO(&fds);

    /*---------------------------------------------------------------------*/
    /* Send the command message */
    if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        qDebug() << "Ntp: unable to open socket";
        return ERROR;
    }

    qDebug() << "Ntp: Socket has been opened";

    if (connect(sd, (struct sockaddr *)&m_sock, sizeof(m_sock)) < 0)
    {
        qDebug() << "Ntp: unable to connect to socket: " << m_sock.sin_addr.s_addr << ":" << m_sock.sin_port;
        return ERROR;
    }
    qDebug() << "Ntp: Socket connected: " << m_sock.sin_addr.s_addr << ":" << m_sock.sin_port;
    FD_SET(sd, &fds);

    if (send(sd, &m_ntpmsg, sizeof(m_ntpmsg), 0) < 0)
    {
        qDebug() << "Ntp: unable to send command to NTP port";
        return ERROR;
    }
    qDebug() << "Ntp: query has been sent";

    /* initialise timeout value */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

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
    if (recv(sd, &m_ntpmsg, sizeof(NtpMsgStruct), 0) < 0)
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
    byte1ok = ((m_ntpmsg.byte1 & 0x3F) == B1VAL);
    byte2ok = ((m_ntpmsg.byte2 & ~MMASK) == (B2VAL | RMASK));
    if (!(byte1ok && byte2ok))
    {
        qDebug() << "Ntp: return data appears to be invalid based on status word";
        return ERROR;
    }

    if (!(m_ntpmsg.byte2 | EMASK))
    {
        qDebug() << "Ntp: error bit is set in reply";
        return ERROR;
    }

    if (!(m_ntpmsg.byte2 | MMASK))
    {
        qDebug() << "Ntp: More bit unexpected in reply";
        return ERROR;
    }

    /* if the leap indicator (LI), which is the two most significant bits
       in status byte1, are both one, then the clock is not synchronised. */
    if ((m_ntpmsg.status1 >> 6) == 3)
    {
        qDebug() << "NTP is unsynchronised";
        return NO_SYNC;
    }
    else
    {
        clksrc = (m_ntpmsg.status1 & 0x3F);
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
