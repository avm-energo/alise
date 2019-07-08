#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include "aliseserver.h"
#include "global.h"

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "--version") == 0)
        {
            printf("alise daemon v%s\n", ALISE_VERSION);
            exit(0);
        }
    }
    Global.FinishThreads = false;
    AliseServer *ST = new AliseServer();
    printf("Server initialized\n");
    ST->Start();
    return 0;
}
