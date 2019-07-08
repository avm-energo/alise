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
#include <sstream>
//#include <iomanip>
#include <iostream>
#include <ctime>
#include <queue>
// #include <codecvt>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <archive.h>
#include <archive_entry.h>
#include "global.h"
#include "log.h"

struct MsgStruct
{
  int type;
  std::string msg;
  std::string file;
  std::string line;
  std::chrono::system_clock::time_point now;
};
std::mutex Log::LMutex;
std::queue<MsgStruct> Queue;
MsgStruct OldMsg;
int IdentMsgCount;

Log::Log()
{
  LogLevel = LOGLEVEL;
}

Log::~Log()
{
  for (int i=0; i<LOGFILEMAX; ++i)
  {
    if (file.is_open())
      file.close();
  }
}

void Log::OpenLogFiles()
{
    file.open(Global.LogFilename, std::fstream::out | std::fstream::app);
    if (!file.is_open())
    {
      std::cout << "Log file can't be opened: ";
      std::cout << Global.LogFilename;
      std::cout << "\n";
      return;
    }
    else
    {
      std::cout << "Log file ";
      std::cout << Global.LogFilename;
      std::cout << " started\n";
    }
}
    
void Log::Run()
{
    const std::string MsgTypeStrings[] = {"CRITICAL", "ERROR", "WARNING", "INFO"};
    IdentMsgCount = 0;
    Msg(LL_NFLOG, "=== Log started ===", __FILE__, __LINE__);
    while (!Global.FinishThreads)
    {
        if (!Queue.empty())
        {
            std::lock_guard<std::mutex> lk(LMutex);
            MsgStruct ms = Queue.front();
            Queue.pop();
            if ((ms.file == OldMsg.file) && (ms.line == OldMsg.line) && (ms.msg == OldMsg.msg))
            {
                ++IdentMsgCount;
                if (IdentMsgCount > LOG_IDENTMSG)
                {
                    file << "last message repeated " << IdentMsgCount << " times\n";
                    IdentMsgCount = 0;
                    file.flush();
                    CheckAndGz();
                }
            }
            else
            {
                if (IdentMsgCount != 0)
                {
                    file << "last message repeated " << IdentMsgCount << " times\n";
                    IdentMsgCount = 0;
                    file.flush();
                    CheckAndGz();
                }
                OldMsg.file = ms.file;
                OldMsg.line = ms.line;
                OldMsg.msg = ms.msg;
                if (ms.type <= LogLevel) // level of logging is acceptable
                {
                    // writing thread number and datetime to log file
                    file << "[";

                    // http://stackoverflow.com/questions/12835577/how-to-convert-stdchronotime-point-to-calendar-datetime-string-with	  
                    std::chrono::milliseconds mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(ms.now.time_since_epoch());
                    std::size_t fraction = mseconds.count() % 1000;
                    time_t t = time(0);
                    struct tm *now = localtime(&t);
                    file << '[' << now->tm_mday << '-' << (now->tm_mon + 1) << '-' << (now->tm_year + 1900);
                    file << ' ' << now->tm_hour << ':' << now->tm_min << ':' << now->tm_sec << '.' << fraction << "][";
                    file << MsgTypeStrings[ms.type] << "] " << ms.file << ":" << ms.line << " " << ms.msg << "\n";
                    file.flush();
                    CheckAndGz();
                }
            }
        }
        usleep(10000);
    }
}

void Log::Msg(int type, const std::string &msg, const char* file, const int line)
{
  MsgStruct ms;
  ms.now = std::chrono::system_clock::now();
  ms.type = (type < 5) ? type : LL_NFLOG;
  ms.file = std::string(file);
  ms.line = std::to_string(line);
  ms.msg = msg;
  std::lock_guard<std::mutex> lk(LMutex);
  Queue.push(ms);
}

void Log::CheckAndGz()
{
  struct stat st;
  std::string GZippedLogFilename = ALISELOGFILE;
  GZippedLogFilename += ".tar.gz";
  if (stat(ALISELOGFILE,&st) == 0) // if the file exists
  {
    if (st.st_size >= LOG_MAX_SIZE)
    {
      // rotating
      int i;
      for (i=9; i>0; --i)
      {
        std::string tmpsold = GZippedLogFilename + "." + std::to_string(i-1);
        if (stat(tmpsold.c_str(), &st) == 0) // file exists
        {
            std::string tmpsnew = GZippedLogFilename + "." + std::to_string(i);
            rename(tmpsold.c_str(), tmpsnew.c_str());
        }
      }
      GZippedLogFilename += ".0";
      // gzip log file and clearing current one
      struct archive *a;
      struct archive_entry *entry;
      char buff[8192];
      int len;
      a = archive_write_new();
      archive_write_add_filter_gzip(a);
      archive_write_set_format_pax_restricted(a);
      archive_write_open_filename(a, GZippedLogFilename.c_str());
      stat(ALISELOGFILE, &st);
      entry = archive_entry_new(); // Note 2
      archive_entry_set_pathname(entry, ALISELOGFILE);
      archive_entry_set_size(entry, st.st_size);
      archive_entry_set_filetype(entry, AE_IFREG);
      archive_entry_set_perm(entry, 0660);
      archive_write_header(a, entry);
      int fd = open(ALISELOGFILE, O_RDONLY);
      len = read(fd, buff, sizeof(buff));
      while (len > 0)
      {
        archive_write_data(a, buff, len);
        len = read(fd, buff, sizeof(buff));
      }
      close(fd);
      archive_entry_free(entry);
      archive_write_close(a);
      archive_write_free(a);
      file.close();
      file.open(ALISELOGFILE, std::fstream::out | std::fstream::trunc);
    }
  }
}
