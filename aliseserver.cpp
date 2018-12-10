/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2018  <copyright holder> <email>
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

#include <fstream>
#include <sstream>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "global.h"
#include "log.h"
#include "connectionthread.h"
#include "aliseserver.h"

AliseServer::AliseServer()
{
    FillConfigmap();
    std::string homedir, defhomedir; // каталог пользователя
    char *tmpc;
    tmpc = static_cast<char *>(malloc(HOMEDIR_MAX_SIZE));
    if ((tmpc = getenv("HOME")) == NULL)
        tmpc = getpwuid(getuid())->pw_dir;
    defhomedir = tmpc;
    defhomedir += "/.supik/";
    homedir = GetValueFromConfigmap("homedir", defhomedir);
    printf("Home directory is: %s", homedir.c_str());
    // сохраняем домашнюю директорию пользователя
    struct stat st;
    Global.HomeDirectory = homedir;
    if (stat(homedir.c_str(),&st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            printf("Home directory found...\n");
        }
        else
        {
            printf("Home directory not found!");
            printf("%s", homedir.c_str());
            exit(1);
        }
    }
    else
    {
        int res = mkdir(homedir.c_str(), 0x01b0);
        if (res)
        {
            printf("Name not found!");
            printf("%s", homedir.c_str());
            exit(1);
        }
    }
}

AliseServer::~AliseServer()
{
  Global.FinishThreads = true;
  printf("Alise finished");
}

void Signal_Handler(int sig)
{
  std::string str("Caught signal ");
  str.append(std::to_string(sig));
  printf("%s", str.c_str());
  switch(sig)
  {
    case SIGHUP:
      break;
    case SIGTERM:
      Global.FinishThreads = true;
      printf("Supik terminated");
      exit(0);
      break;
  }
}

void AliseServer::Start()
{
  int lfp;
  pid_t pid;
  struct sigaction sa;
  char str[10];
  
  if (getppid() == 1) // already a daemon (parent process id = 1 (init))
    return;
  
  printf("Server started\n");
  pid = fork();

  switch(pid)
  {
    case 0:
      if (setsid() == -1)
      {
        printf("Failed to daemonize");
        break;
      }
      chdir("/");
      umask(0);
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
      lfp=open(LOCK_FILE,O_RDWR|O_CREAT,0640);
      if (lfp<0)
      {
        printf("Can't open lockfile for writing\n");
        exit(1); /* can not open */
      }
      if (lockf(lfp,F_TLOCK,0)<0)
      {
        printf("Can't lock, another instance of alise is running?\n");
        exit(0); /* can not lock */
      }
      /* first instance continues */
      sprintf(str,"%d\n",getpid());
      write(lfp,str,strlen(str)); /* record pid to lockfile */
      signal(SIGCHLD,SIG_IGN); /* ignore child */
      signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
      signal(SIGTTOU,SIG_IGN);
      signal(SIGTTIN,SIG_IGN);
      signal(SIGHUP,Signal_Handler); /* catch hangup signal */
      signal(SIGTERM,Signal_Handler); /* catch kill signal */

//      printf("MainLoop");
      MainLoop();
      printf("Exited normally");
      exit(0);
      break;
    case -1:
      printf("fork() error");
      break;
      
    default:
      std::string str("Main process has started with pid=");
      str.append(std::to_string(pid));
      printf("%s\n", str.c_str());
      exit(0);
      break;
  }
}

void AliseServer::MainLoop()
{
  int sockfd, fd;
  struct sockaddr_in sa;
  socklen_t n;
//  int keepalive_enabled = 1;
  int keepalive_time = 180; // seconds of silence to send keepalive packet
  int keepalive_count = 3; // number of retries
  int keepalive_interval = 30; // seconds to wait between retries

  Log log;
  std::thread thr(&Log::Run, &log);
  thr.detach();

  fd = 0;
  sockfd = socket(PF_INET, SOCK_STREAM, 0);

  if(sockfd < 0)
  {
    SDERR("Failed to open socket");
    exit(1);
  }
  else
  {
    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(ALISE_PORT);

    // according to http://www.microhowto.info/howto/listen_on_a_tcp_port_with_connections_in_the_time_wait_state.html
    
    int reuseaddr=1;
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuseaddr,sizeof(reuseaddr))==-1)
    {
      SDERR(strerror(errno));
      exit(1);
    }
    
    if(bind(sockfd, (struct sockaddr *) &sa, sizeof(sa)) < 0)
      SDERR("Failed to bind");
    else
    {
      if(!listen(sockfd, 5))
      {
        ConnectionThread ct;
        // set server start time for status command
        std::thread thr(&ConnectionThread::DoWork, &ct);
        thr.detach();
        while(!Global.FinishThreads)
        {
        n = sizeof(sa);
        if((fd = accept(sockfd, (struct sockaddr *) &sa, &n)) < 0)
        {
            SDERR("Failed to accept");
            break;
        }
        std::string str("Connection #");
        str.append(std::to_string(fd));
        str.append(" accepted from: ");
        str.append(inet_ntoa(sa.sin_addr));
        SDINF(str);
        if ((fcntl(fd, F_SETFL, O_NONBLOCK)) < 0)
        {
            SDERR("Failed to enter non-blocking mode");
            exit(1);
        }
	  int n=1;
	  if (setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(void*)&n,sizeof(n))<0)
	  {
	    SDERR("Failed to set keep alive socket");
	    exit(1);
	  }
// from http://coryklein.com/tcp/2015/11/25/custom-configuration-of-tcp-socket-keep-alive-timeouts.html
/*	  setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&keepalive_time, sizeof (keepalive_time));
	  setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepalive_count, sizeof (keepalive_count));
	  setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepalive_interval, sizeof (keepalive_interval)); */
	  // start data exchange with new fd
//	  ct.AddNewCn(fd,sa.sin_addr.s_addr,sa.sin_port);
	}
      }
    }
  }
  SDINF("Exited from main loop");
}

void AliseServer::FillConfigmap()
{
  // from http://stackoverflow.com/questions/6892754/creating-a-simple-configuration-file-and-parser-in-c
    std::ifstream configfile("/usr/local/etc/alise.conf");
    std::string line;
    while (std::getline(configfile, line))
    {
        std::istringstream configline(line);
        std::string key;
        if (std::getline(configline, key, '='))
        {
            std::string value;
            if (std::getline(configline, value))
                configmap[key] = value;
        }
    }
}

std::string AliseServer::GetValueFromConfigmap(const std::string& key, const std::string& defaultvalue)
{
     std::map<std::string, std::string>::iterator it;
     it = configmap.find(key);
     if (it != configmap.end()) // if the key was found in config file
         return it->second;
     else
         return defaultvalue;
}
