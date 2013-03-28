#include <memory>
#include "event_handler.hpp"
#include "server_socket.hpp"

template <typename T, template <typename> class H>
class server_socket_handler: public ev_handler
{
    public:
      server_socket_handler(unique_ptr<server_socket<T>> sock)
      {
          sock_fd = std::move(sock);
          Reactor *rec = Reactor::get_instance();
          rec->register_handler(EV_IN, (sock_fd.get())->get_sockfd(), this);    
      }
      int handle_read();      
    
    private:
        unique_ptr<server_socket<T>> sock_fd;
        vector<unique_ptr<H<T>>> mapClient;
}; 

template <typename T, template <typename> class H>
int server_socket_handler<T, H>::handle_read()
{
    std::cout<<"Got a new connection"<<std::endl;
    std::unique_ptr<Socket<T>> newSock;
    newSock = (sock_fd.get())->accept_conn(std::move(newSock));
    unique_ptr<H<T>> sockHd(new H<T>(std::move(newSock)));
    mapClient.push_back(std::move(sockHd));
}
