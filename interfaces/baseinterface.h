#ifndef BASEINTERFACE_H
#define BASEINTERFACE_H

#include <QTimer>
#include <gen/datamanager/typesproxy.h>
#include <gen/datatypes.h>
#include <gen/error.h>
#include <gen/logclass.h>
#include <gen/stdfunc.h>

enum INTERVAL
{
    RECONNECT = 3000,
    WAIT = 15000
};

class BasePort;
struct ConnectStruct;

namespace Interface
{

namespace Regs
{
    constexpr quint16 bsiExtStartReg = 40;
    constexpr quint16 timeReg = 4600;
    constexpr quint16 bsiStartReg = 1;
    constexpr quint16 bsiCountRegs = 15;
}

enum State
{
    Connect,
    Reconnect,
    Disconnect,
    Run
};

enum Direction
{
    NoDirection,
    ToDevice,
    FromDevice
};

enum Commands
{
    C_ReqTime,         // *
    C_WriteTime,       // *
    C_WriteUserValues, // *
    C_ReqBlkData,      // *
    C_Reboot           // *
};

struct CommandStruct
{
    Commands command;
    QVariant arg1; // reqFile, writeFile: id, reqStartup: sigAddr, WriteTime: time, WriteCommand: command
    QVariant arg2; // reqFile: format, reqStartup: sigCount, WriteFile: &bytearray, WriteCommand: value
};

class BaseInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)

public:
    /// BaseInterface has its own memory manager
    /// because it can be created and deleted
    /// multiple times in runtime
    using InterfacePointer = UniquePointer<BaseInterface>;

    explicit BaseInterface(QObject *parent = nullptr);
    ~BaseInterface() {};

    /// Pointer to current interface
    static BaseInterface *iface()
    {
        return m_iface.get();
    }

    /// Creator for interface
    static void setIface(InterfacePointer iface)
    {
        m_iface = std::move(iface);
    }

    virtual bool start(const ConnectStruct &) = 0;

    // helper methods
    State state();
    void setState(const State &state);
    void close();

    // commands to send
    void reqTime();
    void writeTime(quint32 time);
    void writeTime(const timespec &time);
    void writeCommand(Commands cmd, QVariant value = 0);
    void setToQueue(CommandStruct &cmd);

signals:
    void connected();
    void disconnected();
    void reconnect();
    void nativeEvent(void *const message);
    void stateChanged(State m_state);
    void wakeUpParser() const;

private:
    bool m_busy, m_timeout;
    QByteArray m_byteArrayResult;
    bool m_responseResult;
    QTimer *m_timeoutTimer;
    static InterfacePointer m_iface;
    State m_state;
    QMutex _stateMutex;
    UniquePointer<DataTypesProxy> proxyBS, proxyGRS;
#ifdef __linux__
    UniquePointer<DataTypesProxy> proxyTS;
#endif
    void ProxyInit();

protected:
    BasePort *ifacePort;

private slots:
    void resultReady(const QVariant &msg);
    void responseReceived(const QVariant &msg);
    void timeout();
};

}

Q_DECLARE_METATYPE(Interface::State)
Q_DECLARE_METATYPE(Interface::CommandStruct)

#endif // BASEINTERFACE_H
