#ifndef INCLUDE_TCPSERVER_H_
#define INCLUDE_TCPSERVER_H_

#include <algorithm>
#include <cstddef>   // size_t
#include <cstdlib>
#include <cstring>   // strerror, strlen, memcpy, strcpy
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "../socket/socket.h"
#include "../utils/logger.h"

class CTCPServer : public ASocket
{
public:
   explicit CTCPServer(const LogFnCallback oLogger,
                       const std::string& strPort,
                       const SettingsFlag eSettings = ALL_FLAGS);
   
   ~CTCPServer() override;

   // copy constructor and assignment operator are disabled
   CTCPServer(const CTCPServer&) = delete;
   CTCPServer& operator=(const CTCPServer&) = delete;

   /* returns the socket of the accepted client, the waiting period can be set */
   bool Listen(Socket& ClientSocket, size_t msec = ACCEPT_WAIT_INF_DELAY);

   int Receive(const Socket ClientSocket,
               char* pData,
               const size_t uSize,
               bool bReadFully = true) const;

   bool Send(const Socket ClientSocket, const char* pData, const size_t uSize) const;
   bool Send(const Socket ClientSocket, const std::string& strData) const;
   bool Send(const Socket ClientSocket, const std::vector<char>& Data) const;

   bool Disconnect(const Socket ClientSocket) const;

   bool SetRcvTimeout(ASocket::Socket& ClientSocket, unsigned int msec_timeout);
   bool SetSndTimeout(ASocket::Socket& ClientSocket, unsigned int msec_timeout);
   
#ifndef  WINDOWS
   bool SetRcvTimeout(ASocket::Socket& ClientSocket, struct timeval Timeout);
   bool SetSndTimeout(ASocket::Socket& ClientSocket, struct timeval Timeout);
#endif

protected:
   Socket m_ListenSocket;
   std::string m_strPort;
   struct sockaddr_in m_ServAddr;
   Logger logger;
};

#endif