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
#include <fstream>
#include "global.h"
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
}

Blinker::~Blinker()
{
}

void Blinker::SetMode(int mode)
{
    BlinkMode = mode;
    LedEndTime = std::chrono::system_clock::now() + LedOnTimes[BlinkMode];
    CurrentLedMode = LEDMODE_ON;
}

void Blinker::Run()
{
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
            rstpushes++;
            printf("RST button has been pushed %d times\n", rstpushes);
        }
        
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
    fd = open("/sys/class/gpio/gpio%d/direction", mode);
//    sprintf(buf, "out");
    if (mode == O_WRONLY)
        write(fd, "out", 3);
    else
        write(fd, "in", 2);
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
