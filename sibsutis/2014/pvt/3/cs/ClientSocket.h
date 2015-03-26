// Definition of the ClientSocket class

#ifndef ClientSocket_class
#define ClientSocket_class

#include "Socket.h"
#include "ticket.hpp"
#include "SocketException.h"

using namespace std;

class ClientSocket : private Socket
{
public:
	ClientSocket ( std::string host, int port )
	{
		if ( ! Socket::create() )
		{
			throw SocketException ( "Could not create client socket." );
		}

		if ( ! Socket::connect ( host, port ) )
		{
			throw SocketException ( "Could not bind to port." );
		}
	}

	virtual ~ClientSocket(){};

	const ClientSocket& operator << ( ticket *tick)
	{
		if ( ! Socket::send ( (void *)tick, sizeof (*tick) ) )
		{
			throw SocketException ( "Could not write to socket." );
		}

		return *this;
	}

	const ClientSocket& operator >> ( ticket *tick)
	{
		if ( ! Socket::recv ( (void *)tick, sizeof (*tick) ) )
		{
			throw SocketException ( "Could not read from socket." );
		}

		return *this;
	}
};


#endif