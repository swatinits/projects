#include "client_event_handler.hpp"

template <typename T, template <typename> class H>
class async_client
{
   public:
        async_client(unsigned int port, string address);
        void write(string data);
            
   private:
        unique_ptr<client_socket<T>> client_sock;
        H<T> client_handler;
};

template <typename T, template <typename> class H>
async_client<T, H>::async_client(unsigned int port, string address):client_sock(new client_socket<T>(port, address)), client_handler(std::move(client_sock))
{
}

template <typename T, template <typename> class H>
void async_client<T, H>::write(string data)
{
    client_handler.write(data);
}
