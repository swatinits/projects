#ifndef __CLIENT_SOCKET_HPP__
#define __CLIENT_SOCKET_HPP__

#include "socket.hpp"

template <typename T>
class client_socket: public Socket<T,typename std::enable_if<Convertible<T*, base_addr*>()>::type>
{
    public:
        client_socket(unsigned int server_port = 80,string ipaddress="localhost"):_ipaddress(ipaddress), _server_port(server_port)
        {
            build_client_socket();
        }
        client_socket(string server_port = "http",string ipaddress="localhost")
        {
            build_client_socket();
        }

        // Destructor
        ~client_socket() {}

    private:
        string _ipaddress;
        int _server_port;
        void build_client_socket();

};

template <typename T>
void client_socket<T>::build_client_socket() 
{ 	
	addrinfo* r = NULL;
	T addr(std::to_string(_server_port),_ipaddress);
    for( r = addr.get_result(); r != NULL; r = r->ai_next) 
	{
		if (connect(this->get_sockfd(), r->ai_addr, r->ai_addrlen) == -1) {
			throw sock_error("Error in connect"); 
			continue;
		}
		break;
	}

	if (r == NULL) 
	{
		throw sock_error("Error in address"); 
	}
}

using client_sock_stream = client_socket<inet_stream_addr>;
#endif
