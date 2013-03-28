#include "include.hpp"

class sock_error : public exception {
	string msg;

public:
	sock_error(const string& _msg) throw()
	: msg(_msg){
		msg.append(": ");
		msg.append(strerror(errno));
	}

  ~sock_error() throw() {};
};

