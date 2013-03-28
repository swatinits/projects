
#include "client_socket.hpp"
#include "event_handler.hpp"

template <typename T>
class client_socket_handler: public ev_handler
{
    public:        
        client_socket_handler(unique_ptr<client_socket<T>> sock)
        {   
            sock_fd = std::move(sock);             
            Reactor *rec = Reactor::get_instance();
            rec->register_handler(EV_IN, (sock_fd.get())->get_sockfd(), this);
        }        

        int handle_read();

        int write();        
    protected:
        std::string read_data;
        std::string write_data;
        unique_ptr<client_socket<T>> sock_fd;         
};

template <typename T>
int client_socket_handler<T>::handle_read()
{
    try 
    {   
        read_data = ((this->sock_fd).get())->read_bytes();
    }   
    catch(sock_error& serr)
    {   
        Socket<T>* sock = sock_fd.get();
        Reactor *rec = Reactor::get_instance();
        rec->deregister_handler(EV_IN, sock->get_sockfd());
    }
}

template <typename T>
int client_socket_handler<T>::write()
{
    (sock_fd.get())->write_bytes(write_data);
}
