#include <iostream>
#include <vector>

#include "socket.h"

ASocket::SocketGlobalInitializer& ASocket::SocketGlobalInitializer::instance()
{
   static SocketGlobalInitializer inst{};
   return inst;
}

ASocket::SocketGlobalInitializer::SocketGlobalInitializer() {}

ASocket::SocketGlobalInitializer::~SocketGlobalInitializer() {}

ASocket::ASocket(const LogFnCallback& oLogger,
                 const SettingsFlag eSettings /*= ALL_FLAGS*/) :
   m_oLog(oLogger),
   m_eSettingsFlags(eSettings),
   m_globalInitializer(SocketGlobalInitializer::instance())
{

}

ASocket::~ASocket() {}

std::string ASocket::StringFormat(const std::string strFormat, ...)
{
   va_list args;
   va_start (args, strFormat);
   size_t len = std::vsnprintf(NULL, 0, strFormat.c_str(), args);
   va_end (args);
   std::vector<char> vec(len + 1);
   va_start (args, strFormat);
   std::vsnprintf(&vec[0], len + 1, strFormat.c_str(), args);
   va_end (args);
   return &vec[0];
}

int ASocket::SelectSocket(const ASocket::Socket sd, const size_t msec)
{
   if (sd < 0)
   {
      return -1;
   }

   struct timeval tval;
   struct timeval* tvalptr = nullptr;
   fd_set rset;
   int res;

   if (msec > 0)
   {
      tval.tv_sec = msec / 1000;
      tval.tv_usec = (msec % 1000) * 1000;
      tvalptr = &tval;
   }

   FD_ZERO(&rset);
   FD_SET(sd, &rset);

   // block until socket is readable.
   res = select(sd + 1, &rset, nullptr, nullptr, tvalptr);

   if (res <= 0)
      return res;

   if (!FD_ISSET(sd, &rset))
      return -1;

   return 1;
}

int ASocket::SelectSockets(const ASocket::Socket* pSocketsToSelect, const size_t count,
                           const size_t msec, size_t& selectedIndex)
{
   if (!pSocketsToSelect || count == 0)
   {
      return -1;
   }

   fd_set rset;
   int res = -1;

   struct timeval tval;
   struct timeval* tvalptr = nullptr;
   if (msec > 0)
   {
      tval.tv_sec = msec / 1000;
      tval.tv_usec = (msec % 1000) * 1000;
      tvalptr = &tval;
   }

   FD_ZERO(&rset);

   int max_fd = -1;
   for (size_t i = 0; i < count; i++)
   {
      FD_SET(pSocketsToSelect[i], &rset);

      if (pSocketsToSelect[i] > max_fd)
      {
         max_fd = pSocketsToSelect[i];
      }
   }

   // block until one socket is ready to read.
   res = select(max_fd + 1, &rset, nullptr, nullptr, tvalptr);

   if (res <= 0)
      return res;

   // find the first socket which has some activity.
   for (size_t i = 0; i < count; i++)
   {
      if (FD_ISSET(pSocketsToSelect[i], &rset))
      {
         selectedIndex = i;
         return 1;
      }
   }

   return -1;
}

struct timeval ASocket::TimevalFromMsec(unsigned int time_msec){
   struct timeval t;

   t.tv_sec = time_msec / 1000;
   t.tv_usec = (time_msec % 1000) * 1000;

   return t;
}