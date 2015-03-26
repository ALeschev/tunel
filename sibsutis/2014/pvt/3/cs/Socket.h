// Definition of the Socket class

#ifndef Socket_class
#define Socket_class


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include "string.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <iostream>

#include "ticket.hpp"

const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 500;

using namespace std;

class Socket
{
 public:
  Socket() :
  m_sock ( -1 )
{

  memset ( &m_addr,
     0,
     sizeof ( m_addr ) );

}
~Socket()
  {
  if ( is_valid() )
    ::close ( m_sock );
}

  // Server initialization
  bool create()
{
  m_sock = socket ( AF_INET,
        SOCK_STREAM,
        0 );

  if ( ! is_valid() )
    return false;

  return true;

}
  bool bind ( const int port )
{

  if ( ! is_valid() )
    {
      return false;
    }

  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  m_addr.sin_port = htons ( port );

  int bind_return = ::bind ( m_sock,
           ( struct sockaddr * ) &m_addr,
           sizeof ( m_addr ) );


  if ( bind_return == -1 )
    {
      return false;
    }

  return true;
}
  bool listen() const
{
  if ( ! is_valid() )
    {
      return false;
    }

  int listen_return = ::listen ( m_sock, MAXCONNECTIONS );


  if ( listen_return == -1 )
    {
      return false;
    }

  return true;
}
  bool accept ( Socket& new_socket ) const
{
  int addr_length = sizeof ( m_addr );
  new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );

  if ( new_socket.m_sock <= 0 )
    return false;
  else
    return true;
}

  // Client initialization
  bool connect ( const std::string host, const int port )
{
  if ( ! is_valid() ) return false;

  m_addr.sin_family = AF_INET;
  m_addr.sin_port = htons ( port );

  int status = inet_pton ( AF_INET, host.c_str(), &m_addr.sin_addr );

  if ( errno == EAFNOSUPPORT ) return false;

  status = ::connect ( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ) );

  if ( status == 0 )
    return true;
  else
    return false;
}

  // Data Transimission
  bool send (void *data, int size )
{
  int status = ::send ( m_sock, data, size, MSG_NOSIGNAL );
  if ( status == -1 )
    {
      cout << "status == -1 " << strerror(errno) << "  in Socket::send\n";
      return false;
    }
  else
    {
      return true;
    }
}
  int recv ( void *data, int size )
{
  memset ( data, 0, size);

  int status = ::recv ( m_sock, data, size, 0 );

  if ( status == -1 )
    {
      cout << "status == -1 " << strerror(errno) << "  in Socket::recv\n";
      return 0;
    }
  else if ( status == 0 )
    {
      return 0;
    }
  else
    {
      return status;
    }
}


  void set_non_blocking ( const bool b )
{

  int opts;

  opts = fcntl ( m_sock,
     F_GETFL );

  if ( opts < 0 )
    {
      return;
    }

  if ( b )
    opts = ( opts | O_NONBLOCK );
  else
    opts = ( opts & ~O_NONBLOCK );

  fcntl ( m_sock,
    F_SETFL,opts );

}

  bool is_valid() const { return m_sock != -1; }

 private:

  int m_sock;
  sockaddr_in m_addr;


};


#endif