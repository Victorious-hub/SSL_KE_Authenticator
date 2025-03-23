#include "tcp_client.h"

CTCPClient::CTCPClient(const LogFnCallback oLogger,
                       const SettingsFlag eSettings /*= ALL_FLAGS*/) :
   ASocket(oLogger, eSettings),
   m_eStatus(DISCONNECTED),
   m_pResultAddrInfo(nullptr),
   m_ConnectSocket(INVALID_SOCKET)
{

}

bool CTCPClient::SetRcvTimeout(unsigned int msec_timeout) 
{
#ifndef WINDOWS
	struct timeval t = ASocket::TimevalFromMsec(msec_timeout);

	return this->SetRcvTimeout(t);
#else
    int iErr;
    iErr = setsockopt(m_ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&msec_timeout, sizeof(struct timeval));
    if (iErr < 0) {
        if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog("[TCPServer][Error] CTCPClient::SetRcvTimeout : Socket error in SO_RCVTIMEO call to setsockopt.");

        return false;
    }

    return true;
#endif
}

#ifndef WINDOWS
bool CTCPClient::SetRcvTimeout(struct timeval timeout) 
{
   int iErr;

	iErr = setsockopt(m_ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(struct timeval));
	if (iErr < 0) 
   {
		if (m_eSettingsFlags & ENABLE_LOG)
			m_oLog("[TCPServer][Error] CTCPClient::SetRcvTimeout : Socket error in SO_RCVTIMEO call to setsockopt.");

		return false;
	}

	return true;
}
#endif

// Method for setting send timeout. Can be called after Connect
bool CTCPClient::SetSndTimeout(unsigned int msec_timeout) 
{
#ifndef WINDOWS
	struct timeval t = ASocket::TimevalFromMsec(msec_timeout);

	return this->SetSndTimeout(t);
#else
    int iErr;
    iErr = setsockopt(m_ConnectSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&msec_timeout, sizeof(struct timeval));
    if (iErr < 0) {
        if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog("[TCPServer][Error] CTCPClient::SetSndTimeout : Socket error in SO_SNDTIMEO call to setsockopt.");

        return false;
    }

    return true;
#endif
}

#ifndef WINDOWS
bool CTCPClient::SetSndTimeout(struct timeval timeout) 
{
	int iErr;

	iErr = setsockopt(m_ConnectSocket, SOL_SOCKET, SO_SNDTIMEO, (char*) &timeout, sizeof(struct timeval));
	if (iErr < 0) 
   {
		if (m_eSettingsFlags & ENABLE_LOG)
			m_oLog("[TCPServer][Error] CTCPClient::SetSndTimeout : Socket error in SO_SNDTIMEO call to setsockopt.");

		return false;
	}

	return true;
}
#endif

// Connexion au serveur
bool CTCPClient::Connect(const std::string& strServer, const std::string& strPort)
{
   if (m_eStatus == CONNECTED)
   {
      Disconnect();
      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog("[TCPClient][Warning] Opening a new connexion. The last one was automatically closed.");
   }

   memset(&m_HintsAddrInfo, 0, sizeof m_HintsAddrInfo);
   m_HintsAddrInfo.ai_family = AF_INET; // AF_INET or AF_INET6 to force version or use AF_UNSPEC
   m_HintsAddrInfo.ai_socktype = SOCK_STREAM;

   int iAddrInfoRet = getaddrinfo(strServer.c_str(), strPort.c_str(), &m_HintsAddrInfo, &m_pResultAddrInfo);
   if (iAddrInfoRet != 0)
   {
      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog(StringFormat("[TCPClient][Error] getaddrinfo failed : %s", gai_strerror(iAddrInfoRet)));

      if (m_pResultAddrInfo != nullptr)
      {
         freeaddrinfo(m_pResultAddrInfo);
         m_pResultAddrInfo = nullptr;
      }

      return false;
   }

   /* getaddrinfo() returns a list of address structures.
    * Try each address until we successfully connect(2).
    * If socket(2) (or connect(2)) fails, we (close the socket
    * and) try the next address. */
   struct addrinfo* pResPtr = m_pResultAddrInfo;
   for (pResPtr = m_pResultAddrInfo; pResPtr != nullptr; pResPtr = pResPtr->ai_next)
   {
      // create socket
      m_ConnectSocket = socket(pResPtr->ai_family, pResPtr->ai_socktype, pResPtr->ai_protocol);
      if (m_ConnectSocket < 0) // or == -1
         continue;

      // connexion to the server
      int iConRet = connect(m_ConnectSocket, pResPtr->ai_addr, pResPtr->ai_addrlen);
      if (iConRet >= 0) // or != -1
      {
         /* Success */
         m_eStatus = CONNECTED;
         
         if (m_pResultAddrInfo != nullptr)
         {
            freeaddrinfo(m_pResultAddrInfo);
            m_pResultAddrInfo = nullptr;
         }

         return true;
      }

      close(m_ConnectSocket);
   }

   if (m_pResultAddrInfo != nullptr)
   {
      freeaddrinfo(m_pResultAddrInfo); /* No longer needed */
      m_pResultAddrInfo = nullptr;
   }

   /* No address succeeded */
   if (m_eSettingsFlags & ENABLE_LOG)
      m_oLog("[TCPClient][Error] no such host.");

   return false;
}

bool CTCPClient::Send(const char* pData, const size_t uSize) const
{
   if (!pData || !uSize)
      return false;

   if (m_eStatus != CONNECTED)
   {
      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog("[TCPClient][Error] send failed : not connected to a server.");
      
      return false;
   }

   int total = 0;
   do
   {
      const int flags = 0;
      int nSent;

      nSent = send(m_ConnectSocket, pData + total, uSize - total, flags);

      if (nSent < 0)
      {
         if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog("[TCPClient][Error] Socket error in call to send.");

         return false;
      }
      total += nSent;
   } while(total < uSize);
   
   return true;
}

bool CTCPClient::Send(const std::string& strData) const
{
   return Send(strData.c_str(), strData.length());
}

bool CTCPClient::Send(const std::vector<char>& Data) const
{
   return Send(Data.data(), Data.size());
}

/* ret > 0   : bytes received
 * ret == 0  : connection closed
 * ret < 0   : recv failed
 */
int CTCPClient::Receive(char* pData, const size_t uSize, bool bReadFully /*= true*/) const
{
   if (!pData || !uSize)
      return -2;

   if (m_eStatus != CONNECTED)
   {
      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog("[TCPClient][Error] recv failed : not connected to a server.");

      return -1;
   }

   int total = 0;
   do
   {
      int nRecvd = recv(m_ConnectSocket, pData + total, uSize - total, 0);

      if (nRecvd == 0)
      {
         // peer shut down
         break;
      }
      
      total += nRecvd;

   } while (bReadFully && (total < uSize));
   
   return total;
}

bool CTCPClient::Disconnect()
{
   if (m_eStatus != CONNECTED)
      return true;

   m_eStatus = DISCONNECTED;
   close(m_ConnectSocket);
   m_ConnectSocket = INVALID_SOCKET;

   return true;
}

CTCPClient::~CTCPClient()
{
   if (m_eStatus == CONNECTED)
      Disconnect();
}