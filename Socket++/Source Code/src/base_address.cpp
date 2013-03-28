#include "../include/base_address.hpp"

void inet_stream_addr::build_address()
{
		// build address details
        int status = 0;
		addrinfo specs;
		memset(&specs, 0, sizeof specs);	// make "specs" empty
		specs.ai_family		= inet_stream_addr::_type;
		specs.ai_socktype	= inet_stream_addr::_domain;	// TCP stream sockets
		specs.ai_flags 		= AI_PASSIVE;	// self ip

		// get address
		status = getaddrinfo( ( _address == "localhost" )? NULL : _address.c_str(), _port.c_str(), &specs, &_result );
        if (status != 0)
        {
    		    cerr << "\nCould not get address information. Error:" << gai_strerror(status);
				freeaddrinfo(_result);
		}

} 
