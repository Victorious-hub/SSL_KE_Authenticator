#ifndef INCLUDE_TCPSSLCLIENT_H_
#define INCLUDE_TCPSSLCLIENT_H_

#include "../socket/secure_socket.h"
#include "tcp_client.h"

class CTCPSSLClient : public ASecureSocket
{
public:
   explicit CTCPSSLClient(const LogFnCallback oLogger,
                          const OpenSSLProtocol eSSLVersion = OpenSSLProtocol::TLS,
                          const SettingsFlag eSettings = ALL_FLAGS);
   ~CTCPSSLClient() override;

   CTCPSSLClient(const CTCPSSLClient&) = delete;
   CTCPSSLClient& operator=(const CTCPSSLClient&) = delete;

   /* connect to a TCP SSL server */
   bool Connect(const std::string& strServer, const std::string& strPort);

   bool SetRcvTimeout(unsigned int timeout);
   bool SetSndTimeout(unsigned int timeout);

#ifndef WINDOWS
   bool SetRcvTimeout(struct timeval timeout);
   bool SetSndTimeout(struct timeval timeout);
#endif

   /* disconnect from the SSL TCP server */
   bool Disconnect();

   /* send data to a TCP SSL server */
   bool Send(const char* pData, const size_t uSize) const;
   bool Send(const std::string& strData) const;
   bool Send(const std::vector<char>& Data) const;

   /* receive data from a TCP SSL server */
   bool HasPending();
   int PendingBytes();

   int Receive(char* pData, const size_t uSize, bool bReadFully = true) const;

protected:
   CTCPClient  m_TCPClient;
   SSLSocket   m_SSLConnectSocket;

};

#endif