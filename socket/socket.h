#ifndef INCLUDE_ASOCKET_H_
#define INCLUDE_ASOCKET_H_

#include <cstdio>         // snprintf
#include <exception>
#include <functional>
#include <memory>
#include <stdarg.h>       // va_start, etc.
#include <stdexcept>
#include <limits>

#ifdef WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#define ACCEPT_WAIT_INF_DELAY std::numeric_limits<size_t>::max()
#define INVALID_SOCKET -1

class ASocket
{
public:
   typedef std::function<void(const std::string&)> LogFnCallback;
   typedef int Socket;

   enum SettingsFlag
   {
      NO_FLAGS = 0x00,
      ENABLE_LOG = 0x01,
      ALL_FLAGS = 0xFF
   };

   explicit ASocket(const LogFnCallback& oLogger,
                    const SettingsFlag eSettings = ALL_FLAGS);
   virtual ~ASocket() = 0; // Pure virtual destructor

   static int SelectSockets(const Socket* pSocketsToSelect, const size_t count,
                            const size_t msec, size_t& selectedIndex);

   static int SelectSocket(const Socket sd, const size_t msec);

   static struct timeval TimevalFromMsec(unsigned int time_msec);

   static std::string StringFormat(const std::string strFormat, ...);

protected:
    const LogFnCallback m_oLog;
    SettingsFlag m_eSettingsFlags;

private:
   friend class SocketGlobalInitializer;
   class SocketGlobalInitializer {
   public:
      static SocketGlobalInitializer& instance();

      SocketGlobalInitializer(SocketGlobalInitializer const&) = delete;
      SocketGlobalInitializer(SocketGlobalInitializer&&) = delete;

      SocketGlobalInitializer& operator=(SocketGlobalInitializer const&) = delete;
      SocketGlobalInitializer& operator=(SocketGlobalInitializer&&) = delete;

      ~SocketGlobalInitializer();

   private:
      SocketGlobalInitializer();
   };
   SocketGlobalInitializer& m_globalInitializer;
};

class EResolveError : public std::logic_error
{
public:
   explicit EResolveError(const std::string &strMsg) : std::logic_error(strMsg) {}
};

#endif // INCLUDE_ASOCKET_H_