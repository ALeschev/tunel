// Definition of the ServerSocket class

#ifndef ServerSocket_class
#define ServerSocket_class

#include "Socket.h"
#include "ticket.hpp"
#include "SocketException.h"

using namespace std;

class ServerSocket : private Socket
{
public:
	ServerSocket ( int port )
	{
		if ( ! Socket::create() )
		{
			throw SocketException ( "Could not create server socket." );
		}

		if ( ! Socket::bind ( port ) )
		{
			throw SocketException ( "Could not bind to port." );
		}

		if ( ! Socket::listen() )
		{
			throw SocketException ( "Could not listen to socket." );
		}

	}

	ServerSocket(){}
	~ServerSocket(){}

	const ServerSocket& operator << ( ticket *tick )
	{
		if ( ! Socket::send ( (void *)tick, sizeof (*tick) ) )
		{
				throw SocketException ( "Could not write to socket." );
		}

		return *this;
	}

	const ServerSocket& operator >> ( ticket *tick )
	{
		if ( ! Socket::recv ( (void *)tick, sizeof (*tick) ) )
		{
				throw SocketException ( "Could not read from socket." );
		}

		return *this;
	}

	void accept ( ServerSocket& sock )
	{
		if ( ! Socket::accept ( sock ) )
		{
				throw SocketException ( "Could not accept socket." );
		}
	}
};


#endif