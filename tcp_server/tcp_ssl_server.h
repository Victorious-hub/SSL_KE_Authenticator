#ifndef INCLUDE_TCPSSLSERVER_H_
#define INCLUDE_TCPSSLSERVER_H_

#include "../socket/secure_socket.h"
#include "tcp_server.h"


class CTCPSSLServer : public ASecureSocket
{
public:
   explicit CTCPSSLServer(const LogFnCallback oLogger,
                          const std::string& strPort,
                          const OpenSSLProtocol eSSLVersion = OpenSSLProtocol::TLS,
                          const SettingsFlag eSettings = ALL_FLAGS)
                          /*throw (EResolveError)*/;

   ~CTCPSSLServer() override;

   CTCPSSLServer(const CTCPSSLServer&) = delete;
   CTCPSSLServer& operator=(const CTCPSSLServer&) = delete;

   bool Listen(SSLSocket& ClientSocket, size_t msec = ACCEPT_WAIT_INF_DELAY);

   bool SetRcvTimeout(SSLSocket& ClientSocket, unsigned int msec_timeout);
   bool SetSndTimeout(SSLSocket& ClientSocket, unsigned int timeout);
   
#ifndef WINDOWS
   bool SetRcvTimeout(SSLSocket& ClientSocket, struct timeval timeout);
   bool SetSndTimeout(SSLSocket& ClientSocket, struct timeval timeout);
#endif

   bool HasPending(const SSLSocket& ClientSocket);
   int PendingBytes(const SSLSocket& ClientSocket);
   int Receive(const SSLSocket& ClientSocket, char* pData,
               const size_t uSize, bool bReadFully = true) const;

   bool Send(const SSLSocket& ClientSocket, const char* pData, const size_t uSize) const;
   bool Send(const SSLSocket& ClientSocket, const std::string& strData) const;
   bool Send(const SSLSocket& ClientSocket, const std::vector<char>& Data) const;

   bool Disconnect(SSLSocket& ClientSocket) const;

protected:
   CTCPServer m_TCPServer;

};

#endif