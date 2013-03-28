/*client.c*/

#include <iostream>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <cstdio>
#include "defn.h"

using namespace std;

int main(int argc, char* argv[])
{
	int clientFD, maxFD;
	fd_set readFD,master;	
	struct addrinfo temp, *res;
	struct timeval tv;
	int connectVal;
	char msgBuf[BUF_SIZE];
	char largeBuf[MAX_CONTENT_BUF_SIZE];
	string uri;		

	if(argc != 4)		  	
	{
		cout << "Usage: ./client serverIP serverPort uri" << endl;
		return 1;
	}

	const char* ipaddr = argv[1];
	const char* port = argv[2];
	const char* cUri = argv[3];
	uri = string(argv[3]);	

	memset(&temp, 0, sizeof(temp));		
	temp.ai_family = AF_INET;		
	temp.ai_socktype = SOCKET_TYPE;
	if((connectVal = getaddrinfo(ipaddr, port, &temp, &res)) != 0)
	{
		cout << "Unable to connect to server";
		exit(1);
	}	
	cout << "Connecting to server" << endl;
	cout << "IP Address: " << ipaddr << endl;
	cout << "Port Number: " << port << endl;
	cout << "URI: " << uri << endl;
    
/*Create client socket*/
	
	clientFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(clientFD == -1)
	{
		perror("socket() error: ");
		exit(1);
	}
 
 	if(connect(clientFD, res->ai_addr, res->ai_addrlen) == -1)
	{
		perror("connect() error: ");
		close(clientFD);
		exit(1);
	}

	cout << "Connection to server established!" << endl;
	freeaddrinfo(res);

/*Clear the file descriptor sets and add client socket to the master set*/

	FD_ZERO(&readFD);
	FD_ZERO(&master);
	FD_SET(clientFD, &master);
	maxFD = clientFD; 

	sprintf(msgBuf, "GET %s HTTP/1.0%s", cUri, DOUBLE_CRLF);
	cout << "Sending HTTP GET message:" << endl << msgBuf << endl;
	if(send(clientFD, msgBuf, strlen(msgBuf), 0) == -1)
	{
		cout << "Could not send GET" << endl;
		close(clientFD);
		exit(1);
	}
 
/*Indefinite Loop*/

	while(1)
	{
		readFD = master;
		tv.tv_sec = 300;
		tv.tv_usec = 0; 	// Implement 5 min timeout
		connectVal = select(maxFD+1, &readFD, NULL, NULL, &tv);
		if(connectVal<=0)
		{
			perror("select() error: ");
			exit(1);
		}
		for(int i = 0; i <= maxFD; ++i)
		{
			if(FD_ISSET(i, &readFD))
			{
				if(i == clientFD)
				{
					memset(&largeBuf, 0, sizeof(largeBuf));	
					int recvbytes = recv(clientFD, largeBuf, MAX_CONTENT_BUF_SIZE, 0);
					if(recvbytes <= 0)
					{
						close(clientFD);
						exit(1);
					}else{	
//					cout<<"recv bytes:"<<recvbytes<<endl;							
				        cout<<largeBuf<<endl;
					memset(largeBuf, 0, recvbytes);
				}
				}
			}
		}			
	}	
	return 0;
}






