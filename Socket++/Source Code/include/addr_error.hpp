#include "include.hpp"

class addr_error : public exception {
	string msg;

public:
	addr_error(const string& _msg) throw()
	: msg(_msg){
		msg.append(": ");
		msg.append( strerror(errno) );
	}

	addr_error( const string& _msg, int family_type ) throw()
	: msg(_msg){
		stringstream s;
		s << family_type;
		msg.append( ": Unknown family type (" + s.str() + ").");
	}

  ~addr_error() throw() {};

};

