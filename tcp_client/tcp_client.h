#ifndef INCLUDE_TCPCLIENT_H_
#define INCLUDE_TCPCLIENT_H_

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

class CTCPSSLClient;

class CTCPClient : public ASocket
{
   friend class CTCPSSLClient;

public:
   explicit CTCPClient(const LogFnCallback oLogger, const SettingsFlag eSettings = ALL_FLAGS);
   ~CTCPClient() override;

   CTCPClient(const CTCPClient&) = delete;
   CTCPClient& operator=(const CTCPClient&) = delete;

	// Session
   bool Connect(const std::string& strServer, const std::string& strPort); // connect to a TCP server
   bool Disconnect(); // disconnect from the TCP server
   bool Send(const char* pData, const size_t uSize) const; // send data to a TCP server
   bool Send(const std::string& strData) const;
   bool Send(const std::vector<char>& Data) const;
   int  Receive(char* pData, const size_t uSize, bool bReadFully = true) const;

   // To disable timeout, set msec_timeout to 0.
   bool SetRcvTimeout(unsigned int msec_timeout);
   bool SetSndTimeout(unsigned int msec_timeout);

#ifndef WINDOWS
   bool SetRcvTimeout(struct timeval Timeout);
   bool SetSndTimeout(struct timeval Timeout);
#endif

   bool IsConnected() const { return m_eStatus == CONNECTED; }

   Socket GetSocketDescriptor() const { return m_ConnectSocket; }

protected:
   enum SocketStatus
   {
      CONNECTED,
      DISCONNECTED
   };

   SocketStatus m_eStatus;
   Socket m_ConnectSocket;

   struct addrinfo* m_pResultAddrInfo;
   struct addrinfo  m_HintsAddrInfo;
   Logger logger;
};

#endif