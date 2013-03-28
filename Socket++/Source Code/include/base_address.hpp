#ifndef __ADDRESS_HPP__
#define __ADDRESS_HPP__

#include "include.hpp"
/*
 * Base class for any type of address 
 */
class base_addr 
{
    public:
        base_addr(int type):_type(type){}
        // function to get and set the type of address
        int get_type(void) const { return _type;} 
        void set_type(int type) { _type = type; }
        
        // function to get and set the size of addr
        int get_size(void) const;
        
    protected:
        int _type;
        int _size;
};

/*
 * Class to create inet stream address
 */
class inet_stream_addr: public base_addr
{
    public:
        const static int _type = AF_INET;
        const static int  _domain = SOCK_STREAM;
        
        inet_stream_addr(string port = "80", string address = "localhost"):base_addr(inet_stream_addr::_type), _port(port), _address(address)
        {
            build_address();
        }

        addrinfo* get_result() {return _result;}
        
        void build_address();

    private:
        addrinfo* _result;
        string _address; 
        string _port;
};

#endif
