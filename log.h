/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  <copyright holder> <email>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <locale>
#include <string>
#include <mutex>
#include <chrono>

#define LOG_MAX_SIZE	1048576
#define LOG_IDENTMSG	1000 // number of identical messages followed one by one

#define LOGLEVEL	LL_NFLOG

#define LOGERR(a)	Log::Msg(MT_ERR, a, __FILE__, __LINE__)

#define LL_CRLOG	0
#define LL_ERLOG	1
#define LL_WRLOG	2
#define LL_NFLOG	3

#define ALISELOGFILE    "log/sqlparse.log"
#define LOGFILEMAX	5

class Log
{
public:
  
  Log();
  ~Log();
  
  static std::mutex LMutex;
 
  void OpenLogFiles();
  void Msg(int type, const std::string &msg, const char* file, const int line);
  void Run();
  void CheckAndGz();
    
private:
  int LogLevel;
  std::fstream file;
};

#endif // LOG_H
