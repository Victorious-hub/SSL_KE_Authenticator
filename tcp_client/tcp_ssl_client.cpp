#include "tcp_ssl_client.h"

CTCPSSLClient::CTCPSSLClient(const LogFnCallback oLogger,
                             const OpenSSLProtocol eSSLVersion,
                             const SettingsFlag eSettings /*= ALL_FLAGS*/) :
   ASecureSocket(oLogger, eSSLVersion, eSettings),
   m_TCPClient(oLogger, eSettings)
{

}

bool CTCPSSLClient::SetRcvTimeout(unsigned int msec_timeout){
   return m_TCPClient.SetRcvTimeout(msec_timeout);
}

bool CTCPSSLClient::SetSndTimeout(unsigned int msec_timeout){
   return m_TCPClient.SetSndTimeout(msec_timeout);
}

#ifndef WINDOWS
bool CTCPSSLClient::SetRcvTimeout(struct timeval timeout) {
    return m_TCPClient.SetRcvTimeout(timeout);
}

bool CTCPSSLClient::SetSndTimeout(struct timeval timeout){
   return m_TCPClient.SetSndTimeout(timeout);
}
#endif

// Connect to server
bool CTCPSSLClient::Connect(const std::string& strServer, const std::string& strPort)
{
   std::cout<< "111" << std::endl;
   if (m_TCPClient.Connect(strServer, strPort))
   {
      m_SSLConnectSocket.m_SockFd = m_TCPClient.m_ConnectSocket;
      SetUpCtxClient(m_SSLConnectSocket);

      if (m_SSLConnectSocket.m_pCTXSSL == nullptr)
      {
         if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog("[TCPSSLClient][Error] SSL_CTX_new failed.");
         //ERR_print_errors_fp(stdout);
         return false;
      }

      if (!m_strSSLCertFile.empty())
      {
         if (SSL_CTX_use_certificate_file(m_SSLConnectSocket.m_pCTXSSL,
            m_strSSLCertFile.c_str(), SSL_FILETYPE_PEM) <= 0)
         {
            if (m_eSettingsFlags & ENABLE_LOG)
               m_oLog("[TCPSSLClient][Error] Loading cert file failed.");

            return false;
         }
      }
      /* Load trusted CA. Mandatory to verify server's certificate */
      if (!m_strCAFile.empty())
      {
         if (!SSL_CTX_load_verify_locations(m_SSLConnectSocket.m_pCTXSSL, m_strCAFile.c_str(), nullptr))
         {
            if (m_eSettingsFlags & ENABLE_LOG)
               m_oLog("[TCPSSLClient][Error] Loading CA file failed.");

            return false;
         }
         SSL_CTX_set_verify_depth(m_SSLConnectSocket.m_pCTXSSL, 1);
      }
   
      if (!m_strSSLKeyFile.empty())
      {
         if (SSL_CTX_use_PrivateKey_file(m_SSLConnectSocket.m_pCTXSSL,
            m_strSSLKeyFile.c_str(), SSL_FILETYPE_PEM) <= 0)
         {
            if (m_eSettingsFlags & ENABLE_LOG)
               m_oLog("[TCPSSLClient][Error] Loading key file failed.");
            //ERR_print_errors_fp(stdout);
            return false;
         }
      }

      /* create new SSL connection state */
      m_SSLConnectSocket.m_pSSL = SSL_new(m_SSLConnectSocket.m_pCTXSSL);
      SSL_set_fd(m_SSLConnectSocket.m_pSSL, m_SSLConnectSocket.m_SockFd);

      /* initiate the TLS/SSL handshake with an TLS/SSL server */
      int iResult = SSL_connect(m_SSLConnectSocket.m_pSSL);
      if (iResult > 0)
      {
         /* The data can now be transmitted securely over this connection. */
         if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog(StringFormat("[TCPSSLClient][Info] Connected with '%s' encryption.",
                             SSL_get_cipher(m_SSLConnectSocket.m_pSSL)));
         
         return true;
      }
      // under Windows it creates problems
      #ifdef LINUX
      ERR_print_errors_fp(stdout);
      #endif

      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog(StringFormat("[TCPSSLClient][Error] SSL_connect failed (Error=%d | %s)",
            iResult, GetSSLErrorString(SSL_get_error(m_SSLConnectSocket.m_pSSL, iResult))));

      return false;
   }

   if (m_eSettingsFlags & ENABLE_LOG)
      m_oLog("[TCPSSLClient][Error] Unable to establish a TCP connection with the server.");

   return false;
}

bool CTCPSSLClient::Send(const char* pData, const size_t uSize) const
{
   if (m_TCPClient.m_eStatus != CTCPClient::CONNECTED)
   {
      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog("[TCPSSLClient][Error] SSL send failed : not connected to an SSL server.");

      return false;
   }

   int total = 0;
   do
   {
      /* encrypt & send message */
      int nSent = SSL_write(m_SSLConnectSocket.m_pSSL, pData + total, uSize - total);
      if (nSent <= 0)
      {
         if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog(StringFormat("[TCPSSLClient][Error] SSL_write failed (Error=%d | %s)",
                  nSent, GetSSLErrorString(SSL_get_error(m_SSLConnectSocket.m_pSSL, nSent))));

         return false;
      }

      total += nSent;
   } while (total < uSize);
   
   return true;
}

bool CTCPSSLClient::Send(const std::string& strData) const
{
   return Send(strData.c_str(), strData.length());
}

bool CTCPSSLClient::Send(const std::vector<char>& Data) const
{
   return Send(Data.data(), Data.size());
}

bool CTCPSSLClient::HasPending()
{
   int pend;

   pend = SSL_has_pending(m_SSLConnectSocket.m_pSSL);

   return pend == 1;
}

int CTCPSSLClient::PendingBytes()
{
   int nPend;

   nPend = SSL_pending(m_SSLConnectSocket.m_pSSL);

   return nPend;
}

int CTCPSSLClient::Receive(char* pData, const size_t uSize, bool bReadFully /*= true*/) const
{
   if (m_TCPClient.m_eStatus != CTCPClient::CONNECTED)
   {
      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog("[TCPSSLClient][Error] SSL recv failed : not connected to a server.");

      return -1;
   }

   int total = 0;
   do
   {
      int nRecvd = SSL_read(m_SSLConnectSocket.m_pSSL, pData + total, uSize - total);

      if (nRecvd <= 0)
      {
         if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog(StringFormat("[TCPSSLClient][Error] SSL_read failed (Error=%d | %s)",
                  nRecvd, GetSSLErrorString(SSL_get_error(m_SSLConnectSocket.m_pSSL, nRecvd))));
         
         break;
      }

      total += nRecvd;

   } while (bReadFully && (total < uSize));
   
   return total;
}

bool CTCPSSLClient::Disconnect()
{
   if (m_TCPClient.m_eStatus != CTCPClient::CONNECTED)
      return true;

   // send close_notify message to notify peer of the SSL closure.
   ShutdownSSL(m_SSLConnectSocket);

   return m_TCPClient.Disconnect();
}

CTCPSSLClient::~CTCPSSLClient()
{
   if (m_TCPClient.m_eStatus == CTCPClient::CONNECTED)
   {
      Disconnect();
      m_TCPClient.Disconnect();
   }
}