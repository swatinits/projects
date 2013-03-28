
#ifndef __EVENT_HANDLER_HPP
#define __EVENT_HANDLER_HPP
#include<iostream>
#include<memory>
#include"reactor.hpp"

#include "socket.hpp"
#include "base_address.hpp"


const int MAX_FD = 64 * 1024;
const int TIMEOUT = 3;
class ev_handler
{
	public:
		ev_handler()
		{
		}

		virtual int handle_read(){};
		virtual int handle_write(){};
		virtual int handle_timeout(){};
		
		~ev_handler(){};
};

template <typename T>
class socket_handler: public ev_handler 
{
    public:
        socket_handler(unique_ptr<Socket<T>> sock)
        {
            sock_fd = std::move(sock);
            Reactor *rec = Reactor::get_instance();
            rec->register_handler(EV_IN, (sock_fd.get())->get_sockfd(), this);
            //rec->register_handler(EV_OUT, (sock_fd.get())->get_sockfd(), this);    
        };
        //destructor
        ~socket_handler();

        int handle_read()
        {
            string data = ((this->sock_fd).get())->read_bytes();
            std::cout<<data;   
            ((this->sock_fd).get())->write_bytes("hi");
        }

        int handle_write(){};
    protected:
        std::string data;
        unique_ptr<Socket<T>> sock_fd;
};

#endif
