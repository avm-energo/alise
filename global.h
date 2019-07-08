#ifndef __GLOBALCLASS__
#define __GLOBALCLASS__

#define ERR(a)	Global.BLog.Msg(LL_ERLOG, a, __FILE__, __LINE__)
#define WAR(a)	Global.BLog.Msg(LL_WRLOG, a, __FILE__, __LINE__)
#define INF(a)	Global.BLog.Msg(LL_NFLOG, a, __FILE__, __LINE__)

#define ALISE_VERSION       "1.0.0"
#define LOCK_FILE           "/run/alise.lock"
#define ALISE_PORT          6978
#define HOMEDIR_MAX_SIZE	255

#include <string>
#include "log.h"

class GlobalClass
{
public:
    std::string HomeDirectory, LogFilename;
    bool FinishThreads;
    std::string DefaultIPString;
    std::string IPFile; // /etc/network/interfaces.d/eth1
    Log BLog;

    GlobalClass();
};

extern GlobalClass Global;

#endif
