#ifndef __GLOBALCLASS__
#define __GLOBALCLASS__

#define SDERR(a)	log.Msg(LL_ERLOG, a, __FILE__, __LINE__)
#define SDWAR(a)	log.Msg(LL_WRLOG, a, __FILE__, __LINE__)
#define SDINF(a)	log.Msg(LL_NFLOG, a, __FILE__, __LINE__)

#define ALISE_VERSION       "1.0.0"
#define LOCK_FILE           "/run/alise.lock"
#define ALISE_PORT          6978
#define HOMEDIR_MAX_SIZE	255

#include <string>

class GlobalClass
{
public:
    std::string HomeDirectory;
    bool FinishThreads;

    GlobalClass();
};

extern GlobalClass Global;

#endif
