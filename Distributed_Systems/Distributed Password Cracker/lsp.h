#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <queue>
#include <map>
#include <time.h>
#include <set>

using namespace std;


// Global Parameters. For both server and clients.

#define _EPOCH_LTH 20.0;
#define _EPOCH_CNT 15.0;
#define _DROP_RATE 0.0;
#define MAX_SG_SIZE 1000



void lsp_set_epoch_lth(double lth);
void lsp_set_epoch_cnt(int cnt);
void lsp_set_drop_rate(double rate);

//general structure for all messages
struct message
{
	int msgType;//0 for connection request, 1 for ack, 2 for data, -1 is a termination message sent to the application client
	uint32_t connid; 
	uint32_t seqnum;
	uint32_t payloadLength;
	uint8_t payload[MAX_SG_SIZE];
};


	
struct lsp_client
{
	char* dest;
	int sockfd;
	int port;
	uint32_t conncectionID;
	uint32_t seqnum;
	//this sequence number to keep track of the received data messages and ignore the old in case of duplicate messages
	uint32_t last_data_received_seqnum;
	
	//this flag is used by the epoch thread to check whether the max allowed time _EPOCH_LTH * _EPOCH_CNT is reached, if so, consider the server as terminated
	time_t lastReceivedDataTime;
	
	struct sockaddr_in serv_addr;	
	pthread_mutex_t ClientInboxMutex;
	pthread_mutex_t ClientOutBoxMutex;
	pthread_mutex_t ClientAckMutex;
	pthread_mutex_t ClientDataLogMutex;
	pthread_mutex_t ClientToBeSentMutex;
		
	pthread_t threadsID[3];
	
};

	

lsp_client* lsp_client_create(const char* dest, int port);
int lsp_client_read(lsp_client* a_client, uint8_t* pld);
bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth);
bool lsp_client_close(lsp_client* a_client);

//Connections' Queues structure
struct connection_queues{
	queue<struct message> ConnectionAckQueueLog;
	queue<struct message> ConnectionDataLog;
	queue<struct message> ConnectionToBeSentQueue;
};


struct server_connection
{
	int port;
	uint32_t conncectionID;
	uint32_t seqnum;
	
	//this sequence number to keep track of the received data messages and ignore the old in case of duplicate messages
	uint32_t last_data_received_seqnum;
	
	//this flag is used by the epoch thread for the keep alive protocol, it is just to check whether the serve has received any previous data for this connection
	bool dataReceivedBefore;
	
	//this flag is to keep track if the connection is still alive or terminated
	bool alive;
	
	//this flag is used by the epoch thread to check whether the max allowed time _EPOCH_LTH * _EPOCH_CNT is reached, if so, consider this connection as terminated
	time_t lastReceivedDataTime;
	
	struct sockaddr_in connection_sock_addr;	
	
	pthread_mutex_t ConnectionAckLogMutex;
	pthread_mutex_t ConnectionDataLogMutex;
	pthread_mutex_t ConnectionToBeSentMutex;
		
};


struct lsp_server
{	
	//this map will be used just to prevent or ignore a connection request sent from an already connected client, because of the keep alive protocol we build
      map<int, struct sockaddr_in> connection_sockaddr_map;
		
	//this map is used by the server to keep track of all of its connectins
	map<int,server_connection> server_connection_map; 
		      
	uint32_t conncectionID;
	int sockfd;
	int port;
	struct sockaddr_in my_addr;
	pthread_mutex_t ServerInboxMutex;
	pthread_mutex_t ServerOutBoxMutex;
	
	pthread_t threadsID[3];

} ;


lsp_server* lsp_server_create(int port);
int  lsp_server_read(lsp_server* a_srv, uint8_t* pld, uint32_t* conn_id);
bool lsp_server_write(lsp_server* a_srv, uint8_t* pld, int lth, uint32_t conn_id);
bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id);

