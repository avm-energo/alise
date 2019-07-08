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

#ifndef BLINKER_H
#define BLINKER_H

#define BLINK_1HZ_ONTIME    500 // ms
#define BLINK_1HZ_OFFTIME   500
#define BLINK_4HZ_ONTIME    125
#define BLINK_4HZ_OFFTIME   125
#define BLINK_03HZ_ONTIME   1500 // ms
#define BLINK_03HZ_OFFTIME  1500

#define GPIO_BLINK_LED      63  // p1_31
#define GPIO_RSTBUTTON      70  // p2_8
#define GPIO_PWR1           5   // p0_5
#define GPIO_PWR2           113 // p3_17

#define RST_TIMEOUT         4000 // ms

#include <chrono>
#include <map>
#include <ratio>

class Blinker
{
public:
    enum BlinkModes
    {
        BLINK_1HZ,
        BLINK_4HZ,
        BLINK_03HZ
    };
    
    enum LedModes
    {
        LEDMODE_ON = 1,
        LEDMODE_OFF = 0
    };
    
    Blinker();
    ~Blinker();
    
    void Run();
    void SetMode(int mode);
    
private:
    int BlinkMode;
    std::chrono::time_point<std::chrono::system_clock> LedEndTime, RstEndTime;
    int CurrentLedMode;
    bool ResetIsActive;

    std::map<int, std::chrono::duration<int, std::milli> > LedOnTimes = \
    {{ BLINK_1HZ, std::chrono::milliseconds(BLINK_1HZ_ONTIME) }, \
    { BLINK_4HZ, std::chrono::milliseconds(BLINK_4HZ_ONTIME) }, \
    { BLINK_03HZ, std::chrono::milliseconds(BLINK_03HZ_ONTIME) }};

    std::map<int, std::chrono::duration<int, std::milli> > LedOffTimes = \
    {{ BLINK_1HZ, std::chrono::milliseconds(BLINK_1HZ_OFFTIME) }, \
    { BLINK_4HZ, std::chrono::milliseconds(BLINK_4HZ_OFFTIME) }, \
    { BLINK_03HZ, std::chrono::milliseconds(BLINK_03HZ_OFFTIME) }};
    
    void SetLed(int gpio, int value);
    void GPIOExport(int gpio, int mode);
    bool GetResetStatus();
};

#endif // BLINKER_H
