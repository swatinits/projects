//#include "server_socket.hpp"
#include "server.hpp"
#include "client_socket.hpp"

template <typename T>
class handle: public socket_handler<T>
{
   public:
    handle(unique_ptr<Socket<T>> sock): socket_handler<T>(std::move(sock)){};      
    int handleRead()
    {
        socket_handler<T>::handleRead();
        write(this->read_data);        
    }

    void write(string data)
    {
        this->write_data = data;
        socket_handler<T>::write();
    }
};

int main()
{
    Reactor rec = Reactor::get_instance();
    async_server<inet_stream_addr, handle, server_socket_handler> server(8080);   
    rec.Run();
    return 0;
}
