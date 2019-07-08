/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2018  anton <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <sys/reboot.h>
#include "global.h"
#include "log.h"
#include "blinker.h"

Blinker::Blinker()
{
    BlinkMode = BLINK_1HZ;
    GPIOExport(GPIO_BLINK_LED, O_WRONLY);
    GPIOExport(GPIO_PWR1, O_RDONLY);
    GPIOExport(GPIO_PWR2, O_RDONLY);
    GPIOExport(GPIO_RSTBUTTON, O_RDONLY);
    LedEndTime = std::chrono::system_clock::now() + LedOnTimes[BlinkMode];
    CurrentLedMode = LEDMODE_ON;
    ResetIsActive = false;
}

Blinker::~Blinker()
{
}

void Blinker::SetMode(int mode)
{
    BlinkMode = mode;
    LedEndTime = std::chrono::system_clock::now() + LedOnTimes[BlinkMode];
    CurrentLedMode = LEDMODE_ON;
    std::string tmps = "Set LED mode: " + std::to_string(mode);
    INF(tmps);
}

void Blinker::Run()
{
    std::string tmps;
    const std::string address_string = "address";
    int rstpushes;
    rstpushes = 0;
    while (!Global.FinishThreads)
    {
        std::chrono::time_point<std::chrono::system_clock> tt = std::chrono::system_clock::now();
        // blink
        if (LedEndTime < tt) // timeout reached
        {
            switch (CurrentLedMode)
            {
                case LEDMODE_OFF:
                    CurrentLedMode = LEDMODE_ON;
                    LedEndTime = tt + LedOffTimes[BlinkMode];
//                    INF("LED on");
                    break;
                case LEDMODE_ON:
                    CurrentLedMode = LEDMODE_OFF;
                    LedEndTime = tt + LedOnTimes[BlinkMode];
//                    INF("LED off");
                    break;
                default:
                    break;
            }
            SetLed(GPIO_BLINK_LED, CurrentLedMode);
        }
        // reset check
        if (!GetResetStatus())
        {
            if (!ResetIsActive)
            {
                ResetIsActive = true;
                RstEndTime = tt + std::chrono::milliseconds(RST_TIMEOUT);
            }
            else
            {
                if (tt > RstEndTime)
                {
                    ++rstpushes;
                    tmps = "RST button has been pushed " + std::to_string(rstpushes) + " times\n";
                    INF(tmps);
                    std::ifstream lfs(Global.IPFile.c_str());
                    if (!lfs.is_open())
                    {
                        tmps = "Error opening file" + Global.IPFile;
                        ERR(tmps);
                    }
                    else
                    { 
                        std::vector<std::string> tmpv;
                        char *line = new char[255];
                        int foundpos;
                        while (!lfs.eof())
                        {
                            lfs.getline(line, 255);
                            tmps = line;
                            if (tmps.empty())
                                continue;
                            foundpos = tmps.find(address_string);
                            if (foundpos != std::string::npos)
                            {
                                tmps = tmps.substr(0, foundpos+address_string.length());
                                tmps.push_back(' ');
                                tmps += Global.DefaultIPString;
                            }
                            tmpv.push_back(tmps);
                        }
                        lfs.close();
                        std::ofstream ofs(Global.IPFile.c_str());
                        for (std::vector<std::string>::const_iterator it=tmpv.begin(); it != tmpv.end(); ++it)
                            ofs << *it << '\n';
                        ofs.close();
                        printf("Going reboot now...");
                        sync();
                        reboot(RB_AUTOBOOT);
                    }
                }
            }
        }
        else
            ResetIsActive = false;
        usleep(100000);
    }
}

void Blinker::SetLed(int gpio, int value)
{
    int fd;
    char buf[255];
    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(buf, O_WRONLY);
    sprintf(buf, "%d", value);
    write(fd, buf, 1);
    close(fd);
}

void Blinker::GPIOExport(int gpio, int mode)
{
    int fd;
    char buf[255];
    fd = open("/sys/class/gpio/export", O_WRONLY);
    sprintf(buf, "%d", gpio);
    write(fd, buf, strlen(buf));
    close(fd);
    sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);
    fd = open(buf, O_WRONLY);
    if (mode == O_WRONLY)
        sprintf(buf, "out");
    else
        sprintf(buf, "in");
    write(fd, buf, strlen(buf));
    close(fd);
}

bool Blinker::GetResetStatus()
{
    bool val;
    std::string getval_str = "/sys/class/gpio/gpio" + std::to_string(GPIO_RSTBUTTON) + "/value";
    std::ifstream getvalgpio(getval_str.c_str());
    getvalgpio >> val;
    getvalgpio.close();
    return val;
}
