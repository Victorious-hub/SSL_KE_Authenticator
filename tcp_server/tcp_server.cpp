#include "tcp_server.h"

CTCPServer::CTCPServer(
	const LogFnCallback oLogger,
	const std::string& strPort,
	const SettingsFlag eSettings /*= ALL_FLAGS*/): ASocket(oLogger, eSettings), m_ListenSocket(INVALID_SOCKET), m_strPort(strPort) 
{
	bzero((char*) &m_ServAddr, sizeof(m_ServAddr));
	int iPort = atoi(strPort.c_str());
	m_ServAddr.sin_family = AF_INET;

	m_ServAddr.sin_addr.s_addr = INADDR_ANY;
	m_ServAddr.sin_port = htons(iPort);
}

// Method for setting receive timeout. Can be called after Listen, using the previously created ClientSocket
bool CTCPServer::SetRcvTimeout(ASocket::Socket& ClientSocket, unsigned int msec_timeout) 
{
	int iErr;

	iErr = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&msec_timeout, sizeof(struct timeval));
	if (iErr < 0) 
	{
		if (m_eSettingsFlags & ENABLE_LOG)
			m_oLog("[TCPServer][Error] CTCPServer::SetRcvTimeout : Socket error in SO_RCVTIMEO call to setsockopt.");
		return false;
	}

	return true;
}

// Method for setting send timeout. Can be called after Listen, using the previously created ClientSocket
bool CTCPServer::SetSndTimeout(ASocket::Socket& ClientSocket, unsigned int msec_timeout) 
{
	int iErr;

	iErr = setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&msec_timeout, sizeof(struct timeval));
	if (iErr < 0) 
	{
		if (m_eSettingsFlags & ENABLE_LOG)
			m_oLog("[TCPServer][Error] CTCPServer::SetSndTimeout : Socket error in SO_SNDTIMEO call to setsockopt.");

		return false;
	}

	return true;
}

#ifndef WINDOWS
bool CTCPServer::SetRcvTimeout(ASocket::Socket& ClientSocket, struct timeval Timeout) 
{
	int iErr;

	iErr = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&Timeout, sizeof(struct timeval));
	if (iErr < 0) 
	{
		if (m_eSettingsFlags & ENABLE_LOG)
			m_oLog("[TCPServer][Error] CTCPServer::SetRcvTimeout : Socket error in SO_RCVTIMEO call to setsockopt.");

		return false;
	}

	return true;
}

bool CTCPServer::SetSndTimeout(ASocket::Socket& ClientSocket, struct timeval Timeout) 
{
	int iErr;

	iErr = setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*) &Timeout, sizeof(struct timeval));
	if (iErr < 0) 
	{
		if (m_eSettingsFlags & ENABLE_LOG)
			m_oLog("[TCPServer][Error] CTCPServer::SetSndTimeout : Socket error in SO_SNDTIMEO call to setsockopt.");

		return false;
	}

	return true;
}
#endif

// returns the socket of the accepted client
// maxRcvTime and maxSendTime define timeouts in µs for receiving and sending over the socket. Using a negative value
// will deactivate the timeout. 0 will set a zero timeout.
bool CTCPServer::Listen(ASocket::Socket& ClientSocket, size_t msec /*= ACCEPT_WAIT_INF_DELAY*/) 
{
	ClientSocket = INVALID_SOCKET;
	// creates a socket to listen for incoming client connections if it doesn't already exist
	if (m_ListenSocket == INVALID_SOCKET) 
	{
		m_ListenSocket = socket(AF_INET, SOCK_STREAM, 0/*IPPROTO_TCP*/);
		if (m_ListenSocket < 0) 
		{
			if (m_eSettingsFlags & ENABLE_LOG)
				m_oLog(StringFormat("[TCPServer][Error] opening socket : %s", strerror(errno)));

			m_ListenSocket = INVALID_SOCKET;
			return false;
		}

		// Allow the socket to be bound to an address that is already in use
		int opt = 1;
		int iErr = 0;

		iErr = setsockopt(m_ListenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(int));
		if (iErr < 0) 
		{
			if (m_eSettingsFlags & ENABLE_LOG)
				m_oLog("[TCPServer][Error] CTCPServer::Listen : Socket error in SO_REUSEADDR call to setsockopt.");

			close(m_ListenSocket);
			m_ListenSocket = INVALID_SOCKET;
			return false;
		}

		int iResult = bind(m_ListenSocket, reinterpret_cast<struct sockaddr*>(&m_ServAddr), sizeof(m_ServAddr));
		if (iResult < 0) 
		{
			if (m_eSettingsFlags & ENABLE_LOG)
				m_oLog(StringFormat("[TCPServer][Error] bind failed : %s", strerror(errno)));
			return false;
		}
	}

	int iResult = listen(m_ListenSocket, SOMAXCONN);
	if (iResult < 0) 
	{
		if (m_eSettingsFlags & ENABLE_LOG)
			m_oLog(StringFormat("[TCPServer][Error] listen failed : %s", strerror(errno)));

		return false;
	}

	if (msec != ACCEPT_WAIT_INF_DELAY)
	{
		int ret = SelectSocket(m_ListenSocket, msec);
		if (ret == 0) 
		{
			if (m_eSettingsFlags & ENABLE_LOG)
				m_oLog("[TCPServer][Error] CTCPServer::Listen : Timed out.");

			return false;
		}

		if (ret == -1) 
		{
			if (m_eSettingsFlags & ENABLE_LOG)
				m_oLog("[TCPServer][Error] CTCPServer::Listen : Error selecting socket.");

			return false;
		}
	}

	struct sockaddr_in ClientAddr;
	socklen_t uClientLen = sizeof(ClientAddr);
	ClientSocket = accept(m_ListenSocket, reinterpret_cast<struct sockaddr*>(&ClientAddr), &uClientLen);
	if (ClientSocket < 0) 
	{
		if (m_eSettingsFlags & ENABLE_LOG)
			m_oLog(StringFormat("[TCPServer][Error] accept failed : %s", strerror(errno)));

		return false;
	}

	if (m_eSettingsFlags & ENABLE_LOG)
		m_oLog(StringFormat("[TCPServer][Info] Incoming connection from '%s' port '%d'", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port)));
	return true;
}

/* ret > 0   : bytes received
 * ret == 0  : connection closed
 * ret < 0   : recv failed
 */
int CTCPServer::Receive(const CTCPServer::Socket ClientSocket, char* pData, const size_t uSize, bool bReadFully /*= true*/) const 
{
	if (ClientSocket < 0 || !pData || !uSize)
		return -1;

	int total = 0;
	do 
	{
		int nRecvd = recv(ClientSocket, pData + total, uSize - total, 0);

		if (nRecvd == 0)
		{
			break;
		}
		total += nRecvd;

	} while (bReadFully && (total < uSize));

	return total;
}

bool CTCPServer::Send(const Socket ClientSocket, const char* pData, size_t uSize) const 
{
	if (ClientSocket < 0 || !pData || !uSize)
		return false;

	int total = 0;
	do 
	{
		const int flags = 0;
		int nSent;

		nSent = send(ClientSocket, pData + total, uSize - total, flags);

		if (nSent < 0) 
		{
			if (m_eSettingsFlags & ENABLE_LOG)
				m_oLog("[TCPServer][Error] Socket error in call to send.");

			return false;
		}
		total += nSent;
	} while (total < uSize);

	return true;
}

bool CTCPServer::Send(const Socket ClientSocket, const std::string& strData) const 
{
	return Send(ClientSocket, strData.c_str(), strData.length());
}

bool CTCPServer::Send(const Socket ClientSocket, const std::vector<char>& Data) const 
{
	return Send(ClientSocket, Data.data(), Data.size());
}

bool CTCPServer::Disconnect(const CTCPServer::Socket ClientSocket) const 
{
	close(ClientSocket);
	return true;
}

CTCPServer::~CTCPServer() 
{
	close(m_ListenSocket);
}