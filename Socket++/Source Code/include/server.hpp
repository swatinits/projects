#include "include.hpp"
#include "server_event_handler.hpp"


template <typename T, template <typename> class H, template <typename, template <typename> class> class S>
class async_server
{
    public:
        async_server(int listenPort);
    
    private:
        unique_ptr<server_socket<T>> listen_sock;
        S<T, H> server_handler;
};

template <typename T, template <typename> class H, template <typename, template <typename> class> class S>
async_server<T, H, S>::async_server(int listenPort): listen_sock(new server_socket<T>(listenPort)), server_handler(std::move(listen_sock))
{
}
