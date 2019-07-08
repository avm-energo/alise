#include <thread>
#include "global.h"

GlobalClass Global;

GlobalClass::GlobalClass()
{
    HomeDirectory.clear();
    FinishThreads = false;
}

