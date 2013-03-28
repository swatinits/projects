#include "client.hpp"

template <typename T>
class handle: public client_socket_handler<T>
{
    public:
        handle(unique_ptr<client_sock_stream> sock): client_socket_handler<T>(std::move(sock)){};    
        int handle_read()
        {   
            client_socket_handler<T>::handle_read();
            std::cout<<this->read_data;    
            write("hello");
        }   

        void write(string data)
        {   
            this->write_data = data;
            client_socket_handler<T>::write();
        }   
};

int main()
{
    try 
    {   
        Reactor *rec = Reactor::get_instance();
        async_client<inet_stream_addr, handle> client(80, "localhost");
        client.write("hello");
        rec->Run();
    }   
    catch (sock_error& serr)
    {   
        std::cout<<"Got error"<<serr.what();    
    }   
    return 0;
}
