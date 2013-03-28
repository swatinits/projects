#ifndef __REACTOR_HPP
#define __REACTOR_HPP
#include<map>
#include<sys/select.h>

class ev_handler;

enum Events {
	EV_IN,
	EV_OUT,
	EV_ERR,
	EV_HUP	
};


class Reactor 
{
    public:
        bool register_handler(Events ev, int fd, ev_handler *evH); 
        bool deregister_handler(Events ev, int fd); 
        bool register_timeout_handler(ev_handler *evH); 
        //run it in a different thread since it is blocking
        void Run();
		Reactor *get_instance();

    private:
        Reactor();
        std::map<int, ev_handler*> fd_read_map;
        std::map<int, ev_handler*> fd_write_map;
        fd_set master_read_set, master_write_set;
        int maxfd;
        struct timeval timeout;
        ev_handler *timeout_handler;
		Reactor *ins;
};

#endif
