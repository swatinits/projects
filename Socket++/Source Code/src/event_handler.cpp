#include<iostream>
#include<map>
#include<sys/select.h>
#include<memory>
#include "event_handler.hpp"

Reactor* Reactor::get_instance() 
{
    if (ins == NULL) {
        ins = new Reactor();
    }
    return ins;
}

Reactor::Reactor(): maxfd(0) 
{   
    timeout.tv_sec  = TIMEOUT * 60;
    timeout.tv_usec = 0;
    FD_ZERO(&master_read_set);
    FD_ZERO(&master_write_set);
}

bool Reactor::register_handler(Events ev, int fd, ev_handler *evH) 
{
    switch (ev)
    {
        case EV_IN:
            FD_SET(fd, &master_read_set);
            fd_read_map.insert(std::pair<int, ev_handler*>(fd, evH));
            break;
        case EV_OUT:
            FD_SET(fd, &master_write_set);
            fd_read_map.insert(std::pair<int, ev_handler*>(fd, evH));
            break;
    }
    if(fd > maxfd) 
    {
        maxfd = fd;
    }
    return true;
}

bool Reactor::register_timeout_handler(ev_handler *evH) 
{
    timeout_handler = evH;
}

bool Reactor::deregister_handler(Events ev, int fd) 
{
    std::map<int, ev_handler*>::iterator iter;
    switch (ev) 
    {
        case EV_IN:
            FD_CLR(fd, &master_read_set);
            iter = fd_read_map.find(fd);
            fd_read_map.erase(iter);
            break;
        case EV_OUT:
            FD_CLR(fd, &master_write_set);
            iter = fd_write_map.find(fd);
            fd_write_map.erase(iter);
            break;
    }
    if (fd == maxfd) 
    {
        while (FD_ISSET(maxfd, &master_write_set) == false || FD_ISSET(maxfd, &master_read_set) == false) 
        {
            maxfd -= 1;
        }
    }
    return true;
}

void Reactor::Run() 
{
    int rc;
    fd_set working_read_set, working_write_set;
    FD_ZERO(&working_read_set);
    FD_ZERO(&working_write_set);
    do 
    {
        memcpy(&working_read_set, &master_read_set, sizeof(fd_set));
        memcpy(&working_write_set, &master_write_set, sizeof(fd_set));

        rc = select(maxfd + 1, &working_read_set, NULL, NULL, NULL);

        if (rc < 0)
        {
            perror("  select() failed");
            break;
        }

        if (rc == 0)
        {
            printf("  select() timed out\n");
            timeout_handler->handle_timeout();
            continue;
        }

        int totalfds = rc;
        for (int i=0; i <= maxfd  && totalfds > 0; ++i)
        {
            if (FD_ISSET(i, &working_read_set))
            {
                fd_read_map[i]->handle_read();
                totalfds--;
            } 

            if (FD_ISSET(i, &working_write_set)) 
            {
                fd_read_map[i]->handle_write();
                totalfds--;
            }
        }
    } while(1);
}

Reactor* Reactor::ins = NULL;
