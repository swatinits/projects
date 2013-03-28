#include "lsp.h"
#include "lspmessage.pb-c.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include  <signal.h>

using namespace std;


/*
 *
 *
 *				LSP RELATED FUNCTIONS
 *
 *
 */  
 
double epoch_lth = _EPOCH_LTH;
int epoch_cnt = _EPOCH_CNT;
double drop_rate = _DROP_RATE;

void lsp_set_epoch_lth(double lth){epoch_lth = lth;}
void lsp_set_epoch_cnt(int cnt){epoch_cnt = cnt;}
void lsp_set_drop_rate(double rate){drop_rate = rate;}


//Global Server Variable

lsp_server main_Server;


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
////////////////// SERVER LSP API /////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////


//Server's Queues' and their methods for the connection////
queue<struct message> ServerInboxQueue;
queue<struct message> ServerOutboxQueue;

//enqueue the ServerInboxQueue	
void enqueue_ServerInboxQueue(struct message entry)
{
	pthread_mutex_lock(&main_Server.ServerInboxMutex);
	ServerInboxQueue.push(entry);
	pthread_mutex_unlock(&main_Server.ServerInboxMutex);
}

//dequeue the ServerInboxQueue	
void dequeue_ServerInboxQueue(struct message *entry, bool *isempty)
{
	pthread_mutex_lock(&main_Server.ServerInboxMutex);
	if( ServerInboxQueue.empty() )
		*isempty = true;
	else{
		*entry = ServerInboxQueue.front();
		ServerInboxQueue.pop();
		*isempty = false;
	}
	pthread_mutex_unlock(&main_Server.ServerInboxMutex);
}

//enqueue the ServerOutboxQueue	
void enqueue_ServerOutboxQueue(struct message entry)
{
	pthread_mutex_lock(&main_Server.ServerOutBoxMutex);
	ServerOutboxQueue.push(entry);
	pthread_mutex_unlock(&main_Server.ServerOutBoxMutex);
}

//dequeue the ServerOutboxQueue	
void dequeue_ServerOutboxQueue(struct message *entry, bool *isempty)
{
	pthread_mutex_lock(&main_Server.ServerOutBoxMutex);
	if( ServerOutboxQueue.empty() )
		*isempty = true;
	else{
		*entry = ServerOutboxQueue.front();
		ServerOutboxQueue.pop();
		*isempty = false;
	}
	pthread_mutex_unlock(&main_Server.ServerOutBoxMutex);
}



//Connection Queues, a map for them and their methods
//a map that contains  connections' Queues, each connection has three Queues
map<int,connection_queues> server_connection_queues;

//enqueue the ConnectionAckQueueLog	
void enqueue_ConnectionAckQueueLog(int index, struct message entry)
{

	pthread_mutex_lock(&main_Server.server_connection_map.at(index).ConnectionAckLogMutex);
	server_connection_queues.at(index).ConnectionAckQueueLog.push(entry);
	pthread_mutex_unlock(&main_Server.server_connection_map.at(index).ConnectionAckLogMutex);
}

//dequeue the ConnectionAckQueueLog	
void dequeue_ConnectionAckQueueLog(int index, struct message *entry, bool *isempty)
{
	pthread_mutex_lock(&main_Server.server_connection_map.at(index).ConnectionAckLogMutex);
	if( server_connection_queues.at(index).ConnectionAckQueueLog.empty() )
		*isempty = true;
	else{
		*entry = server_connection_queues.at(index).ConnectionAckQueueLog.front();
		server_connection_queues.at(index).ConnectionAckQueueLog.pop();
		*isempty = false;
	}
	pthread_mutex_unlock(&main_Server.server_connection_map.at(index).ConnectionAckLogMutex);
}

//enqueue the ConnectionDataLog	
void enqueue_ConnectionDataLog(int index, struct message entry)
{

	pthread_mutex_lock(&main_Server.server_connection_map.at(index).ConnectionDataLogMutex);
	server_connection_queues.at(index).ConnectionDataLog.push(entry);
	pthread_mutex_unlock(&main_Server.server_connection_map.at(index).ConnectionDataLogMutex);
}

//dequeue the ConnectionDataLog	
void dequeue_ConnectionDataLog(int index, struct message *entry, bool *isempty)
{
	pthread_mutex_lock(&main_Server.server_connection_map.at(index).ConnectionDataLogMutex);
	if( server_connection_queues.at(index).ConnectionDataLog.empty() )
		*isempty = true;
	else{
		*entry = server_connection_queues.at(index).ConnectionDataLog.front();
		server_connection_queues.at(index).ConnectionDataLog.pop();
		*isempty = false;
	}
	pthread_mutex_unlock(&main_Server.server_connection_map.at(index).ConnectionDataLogMutex);
}

//take a copy of the front of ConnectionDataLog without pop, needs for keep alive prtocol
void frontOf_ConnectionDataLog(int index, struct message *entry, bool *isempty)
{
	pthread_mutex_lock(&main_Server.server_connection_map.at(index).ConnectionDataLogMutex);
	if( server_connection_queues.at(index).ConnectionDataLog.empty() )
		*isempty = true;
	else{
		*entry = server_connection_queues.at(index).ConnectionDataLog.front();
		*isempty = false;
	}
	pthread_mutex_unlock(&main_Server.server_connection_map.at(index).ConnectionDataLogMutex);
}

//return whether the ConnectionDataLog is empty or not
void isEmpty_ConnectionDataLog(int index, bool *isempty)
{
	pthread_mutex_lock(&main_Server.server_connection_map.at(index).ConnectionDataLogMutex);
	if(server_connection_queues.at(index).ConnectionDataLog.empty())
		*isempty = true;
	else
		*isempty = false;
	pthread_mutex_unlock(&main_Server.server_connection_map.at(index).ConnectionDataLogMutex);
}


//enqueue the ConnectionToBeSentQueue	
void enqueue_ConnectionToBeSentQueue(int index, struct message entry)
{

	pthread_mutex_lock(&main_Server.server_connection_map.at(index).ConnectionToBeSentMutex);
	server_connection_queues.at(index).ConnectionToBeSentQueue.push(entry);
	pthread_mutex_unlock(&main_Server.server_connection_map.at(index).ConnectionToBeSentMutex);
}

//dequeue the ConnectionToBeSentQueue	
void dequeue_ConnectionToBeSentQueue(int index, struct message *entry, bool *isempty)
{
	pthread_mutex_lock(&main_Server.server_connection_map.at(index).ConnectionToBeSentMutex);
	if( server_connection_queues.at(index).ConnectionToBeSentQueue.empty() )
		*isempty = true;
	else{
		*entry = server_connection_queues.at(index).ConnectionToBeSentQueue.front();
		server_connection_queues.at(index).ConnectionToBeSentQueue.pop();
		*isempty = false;
	}
	pthread_mutex_unlock(&main_Server.server_connection_map.at(index).ConnectionToBeSentMutex);
}

//return whether the ConnectionToBeSentQueue is empty or not
void isEmpty_ConnectionToBeSentQueue(int index, bool *isempty)
{
	pthread_mutex_lock(&main_Server.server_connection_map.at(index).ConnectionToBeSentMutex);
	if(server_connection_queues.at(index).ConnectionToBeSentQueue.empty())
		*isempty = true;
	else
		*isempty = false;
	pthread_mutex_unlock(&main_Server.server_connection_map.at(index).ConnectionToBeSentMutex);
}


/*
*
*
*			Server Threads
*
*
*
*/


///////////////////////////////////////////////////////////
////////////// Server's Receivng Thread ///////////////////
///////////////////////////////////////////////////////////

void* serverReceivngThread(void *arg)
{	
	//struct of different messages
	struct message receivedMessage;
	struct message ackForConnectionRequest;
	struct message ackForReceivedMessage;
	struct message loggedMessageforData;
	struct message nextMessagetoSend;
	struct server_connection new_connection;
	struct sockaddr_in cli_addr;
	socklen_t slen=sizeof(cli_addr);
	bool isempty = true;
	uint8_t buf[MAX_SG_SIZE];
	int receivedBufferlen;
	int index;
	int i;
	
	
	while(true) 
	{

		//printf("\n Receiving thread for server\n");
		
		//receiving messages from the server
		if (recvfrom(main_Server.sockfd, buf, MAX_SG_SIZE, 0, (struct sockaddr*)&(cli_addr),&slen)==-1)
		{
			printf("error in the Server Receivng Thread\n");
			
		}

	
//		printf("Server Receive a Packet From:%s\n: %d\n",
//		inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
		
		
		//preparing for unmarshaling
		receivedBufferlen = buf[0];
		uint8_t tempBuffer[receivedBufferlen]; 
		for(index=0;index<receivedBufferlen;index++)
			tempBuffer[index] = buf[index+1];

/*
		printf("\n");
		for(i=0; i<MAX_SG_SIZE; i++)
			printf("%d", buf[i]);
		printf("\n");
*/	
	
		//unmarshaling the message
		LSPMessage *lspmsg;
		lspmsg = lspmessage__unpack(NULL,  receivedBufferlen, tempBuffer); 
	   
		if (lspmsg == NULL)
		{     //Something failed
			printf("error while unmarshaling incoming message from server\n");

		}
/*		
		printf("lspmsg->connid: %d\n",lspmsg->connid );
		printf("lspmsg->seqnum: %d\n",lspmsg->seqnum );
		printf("lspmsg->payload.data: %s\n",lspmsg->payload.data);
		printf("sizeof(lspmsg->payload.len): %d\n",lspmsg->payload.len);
*/		
		//filling the fields for the receivedMessage struct
		receivedMessage.connid = lspmsg->connid;
		receivedMessage.seqnum = lspmsg->seqnum;
		receivedMessage.payloadLength = lspmsg->payload.len;
		memcpy(receivedMessage.payload,lspmsg->payload.data,  lspmsg->payload.len*sizeof(uint8_t));
	  
/*		printf("The Server recieved the following from the server\n");	
		printf("receivedMessage.msgType: %d\n", receivedMessage.msgType );
		printf("receivedMessage.connid: %d\n", receivedMessage.connid );
		printf("receivedMessage.seqnum: %d\n", receivedMessage.seqnum );
		printf("receivedMessage.payloadLength: %d\n", receivedMessage.payloadLength );
		printf("receivedMessage.payload: %s\n", receivedMessage.payload );
*/		

		
		if(lspmsg->payload.len == 0)//this is an Ack or a connection request or some sent data before
		{
			receivedMessage.msgType = 1;
//			check the connection ID of the received message, if it is zero this is new client
      		if(receivedMessage.connid == 0 || (receivedMessage.connid !=0 && receivedMessage.seqnum ==0))// this is a new connection request or a repeated 				connection request for some previous client because of the keep alive protocol
      		{	
//      			check if this client is already connected, this might happen because the client might send more then one connection request because of the 					keep alive
      			std::pair<std::map<int, struct sockaddr_in>::iterator,bool> ret;
			  	ret = main_Server.connection_sockaddr_map.insert ( std::pair<int, struct sockaddr_in>(ntohs(cli_addr.sin_port),cli_addr) );
			  	if (ret.second == true)//new connection request
			  	{
			 		
//					building a new connection structure
					new_connection.port = ntohs(cli_addr.sin_port);
					new_connection.conncectionID = ++  main_Server.conncectionID;
					new_connection.seqnum = 0;
					new_connection.last_data_received_seqnum = 0;
					new_connection.dataReceivedBefore = false;//this must be set true when this connection received any data, this flag for the Epoch 						thread
					new_connection.alive = true;//this must be set false when the server lost the connection, this is handled by epoch thread
					new_connection.connection_sock_addr = cli_addr;
					new_connection.ConnectionAckLogMutex = PTHREAD_MUTEX_INITIALIZER;
					new_connection.ConnectionDataLogMutex = PTHREAD_MUTEX_INITIALIZER;
					new_connection.ConnectionToBeSentMutex = PTHREAD_MUTEX_INITIALIZER;
					
//					add this new connection structure to the server_connection_map					
					main_Server.server_connection_map.insert(pair<int,server_connection>(new_connection.conncectionID,new_connection));
				
/*					printf("main_Server.server_connection_map.at(new_connection.conncectionID).conncectionID=%d\n", main_Server.server_connection_map.at
					(new_connection.conncectionID).conncectionID);
					printf("main_Server.server_connection_map.at(new_connection.conncectionID).port=%d\n", main_Server.server_connection_map.at
					(new_connection.conncectionID).port);		
					printf("main_Server.server_connection_map.at(new_connection.conncectionID).seqnum=%d\n", main_Server.server_connection_map.at
					(new_connection.conncectionID).seqnum);
*/
//					create three queues, add them to the server connection queues, the first element is one which is the first connection ID assigned
					struct connection_queues new_queues;
					server_connection_queues.insert(pair<int,struct connection_queues>(new_connection.conncectionID,new_queues));
					
//					here build an Ack For the Connection Request using the ackForConnectionRequest struct
					
					ackForConnectionRequest.msgType = 1; //0 for connection request, 1 for ack, 2 for data
					ackForConnectionRequest.connid = new_connection.conncectionID;
					ackForConnectionRequest.seqnum = 0;
					ackForConnectionRequest.payloadLength = 0;
					ackForConnectionRequest.payload[0] = '\0';
					
					
//					put this reply or Ack for the connection request in the ServerOutboxQueue to be sent to the client 
					enqueue_ServerOutboxQueue(ackForConnectionRequest);
					
//					update the connection_last_received_time for this connection
					time(&main_Server.server_connection_map.at(new_connection.conncectionID).lastReceivedDataTime);
				}
		
				else
				{	
//					an already connected cleint, just ignore the request
//					printf("receive a connection request of an already connected client because of the keep alive protocol we build\n");			
					time(&main_Server.server_connection_map.at(receivedMessage.connid).lastReceivedDataTime);
							
				}
			}
			
			else// this is an Ack for some data sent before
			{
//				based on the connection ID in the received message, dequeue the corresponding ConnectionDataLog and compare the sequence #
//				two meeesages and they must be at least equal because of the sequence protocl we built!

//				update the connection_last_received_time for this connection
				time(&main_Server.server_connection_map.at(receivedMessage.connid).lastReceivedDataTime);
		
				dequeue_ConnectionDataLog(receivedMessage.connid, &loggedMessageforData, &isempty);
				
				if(isempty == true)
				{
//					printf("receive an ack for some old data because of the live sequence protocol!\n");
					;
				}
				else
					if(loggedMessageforData.seqnum == receivedMessage.seqnum)//match 
					{
//						printf("match of two seq numbers, pop antoher message to be sent!\n");
//						set the dataReceivedBefore for this connection to true if it is NOT
						if(!main_Server.server_connection_map.at(receivedMessage.connid).dataReceivedBefore)
							main_Server.server_connection_map.at(receivedMessage.connid).dataReceivedBefore = true;
//						pop another message from the ConnectionToBeSentQueue and insert it in the ServerOutboxQueue to be sent
						dequeue_ConnectionToBeSentQueue(receivedMessage.connid, &nextMessagetoSend, &isempty);
						
//						fill the seq number, the connection ID must be filled before when the server write & push it ServerOutboxQueue 
						if(isempty == false)
						{
							nextMessagetoSend.seqnum = ++ main_Server.server_connection_map.at(receivedMessage.connid).seqnum;// increment the 
//							sequence number fot this connection's


							enqueue_ServerOutboxQueue(nextMessagetoSend);
						}
						else
						{
//							printf("Server has nothing to be sent to Connection: %d\n", receivedMessage.connid);
							;
						}
					}
					else if(receivedMessage.seqnum < loggedMessageforData.seqnum )// error, dismatch of sequence number, this is recieved because of the
//					keep alive protocol we build, 
					{
//						printf("Server has received an Ack for some old data for connection: %d\because of the live sequence protocol!\n",receivedMessage.connid);		
						;										
					}
					else//almost impossible, recieve an Ack for large sequence number
						{
//							printf("dismatch of sequence number, receive an ack for some data with a sequence number which is gratet than the expected!\n");
							;
						}
			}
		}
		
		
			
		else// this is for data
		{
			
//			update the connection_last_received_time for this connection
			time(&main_Server.server_connection_map.at(receivedMessage.connid).lastReceivedDataTime);
//			printf("Recceive a message  with sequence number=%d\n", receivedMessage.seqnum);	
			
			if(main_Server.server_connection_map.at(receivedMessage.connid).last_data_received_seqnum == 0)//this is the first received message
			{
			
//				printf("The first message recceived with a sequence number=%d\n", receivedMessage.seqnum);
				if(receivedMessage.seqnum == 1)// this is the first message received, and it's sequence number must be 1, else, the first message is dropped, so do NOT ack this message until the other side send a message with sequence number 1
				{
				
					main_Server.server_connection_map.at(receivedMessage.connid).last_data_received_seqnum = receivedMessage.seqnum;
				
	//				set the message type to 2, and enqueue it in the ServerInboxQueue to be read by the server
					receivedMessage.msgType = 2;
					enqueue_ServerInboxQueue(receivedMessage);
			
			
	//				create a an Ack for this received data message and enqueue it directly in the outbox queue to be sent soon by the sending thread
					ackForReceivedMessage.msgType = 1;
					ackForReceivedMessage.connid = receivedMessage.connid;
					ackForReceivedMessage.seqnum = receivedMessage.seqnum;
					ackForReceivedMessage.payloadLength = 0;
					ackForReceivedMessage.payload[0] = '\0';
					enqueue_ServerOutboxQueue(ackForReceivedMessage);
				}
				else//ignore the message
					;
			}
			

			else if (receivedMessage.seqnum == main_Server.server_connection_map.at(receivedMessage.connid).last_data_received_seqnum +1) //set the message type 				to 2, and enqueue it in the inbox queue to be read by the server
			{
			 
//			 	printf("Match the received message with the expected sequencen number + 1=%d\n", main_Server.server_connection_map.at(receivedMessage.connid).last_data_received_seqnum + 1);
//				increment the last_data_received_seqnum for this connection
				 main_Server.server_connection_map.at(receivedMessage.connid).last_data_received_seqnum ++;
				 
//				set the message type to 2, and enqueue it in the ServerInboxQueue to be read by the server
				receivedMessage.msgType = 2;
				enqueue_ServerInboxQueue(receivedMessage);
			
			
//				create a an Ack for this received data message and enqueue it directly in the outbox queue to be sent soon by the sending thread
				ackForReceivedMessage.msgType = 1;
				ackForReceivedMessage.connid = receivedMessage.connid;
				ackForReceivedMessage.seqnum = receivedMessage.seqnum;
				ackForReceivedMessage.payloadLength = 0;
				ackForReceivedMessage.payload[0] = '\0';
				enqueue_ServerOutboxQueue(ackForReceivedMessage);
		
			}
			
			else if (receivedMessage.seqnum <= main_Server.server_connection_map.at(receivedMessage.connid).last_data_received_seqnum)// just ignore the message, duplication, but you have to send an Ack, because it might be the last one lost
			{
//				printf("the server receive a duplicated data message from this connection because of the live sequence protocol we build \n");
//				printf("NO Match between the received message and the expected sequencen number + 1=%d\n", main_Server.server_connection_map.at(receivedMessage.connid).last_data_received_seqnum +1);
				//create a an Ack for this received data message and enqueue it directly in the outbox queue to be sent soon by the sending thread
				ackForReceivedMessage.msgType = 1;
				ackForReceivedMessage.connid = receivedMessage.connid;
				ackForReceivedMessage.seqnum = receivedMessage.seqnum;
				ackForReceivedMessage.payloadLength = 0;
				ackForReceivedMessage.payload[0] = '\0';
				enqueue_ServerOutboxQueue(ackForReceivedMessage);
			}
		}
		
	isempty = true;
	sleep(0.3);


	}
}
	
		
///////////////////////////////////////////////////////////
////////////// Server's Sending Thread ////////////////////
///////////////////////////////////////////////////////////	
	
void* serverSendingThread(void *arg)
{

	struct message toSendMessage;
	bool isempty;
	isempty = true;
	LSPMessage lspmsg = LSPMESSAGE__INIT;
	uint8_t* buf;
	uint8_t* tempBuffer;
	double random;
	socklen_t slen;	
	int index;
	int len;
	int i;
		
	while (true)
	{
		
//		printf("\n Sending thread for server\n");
		
		while(isempty)
		{	
//			printf("outbox queue is empty\n");
			dequeue_ServerOutboxQueue(&toSendMessage,&isempty);
			sleep(0.3);
		}
		
//		marshaling the message to send it
		lspmsg.connid =  toSendMessage.connid;	
		lspmsg.seqnum =  toSendMessage.seqnum;
		lspmsg.payload.data = (uint8_t*) malloc(sizeof(uint8_t) * toSendMessage.payloadLength);
		lspmsg.payload.len = toSendMessage.payloadLength;
		memcpy(lspmsg.payload.data, toSendMessage.payload, toSendMessage.payloadLength*sizeof(uint8_t));
	  
	
		len = lspmessage__get_packed_size(&lspmsg);
		buf = (uint8_t*)malloc(len+1);
		tempBuffer = (uint8_t*)malloc(len);
		lspmessage__pack(&lspmsg, tempBuffer);
		buf[0] = len;
		for(index=0;index<len+1;index++)
			buf[index+1] = tempBuffer[index];
		
		


			
/*		printf("\n");

		for(i=0; i<len+1; i++)
			printf("%d ", buf[i]);
		printf("\n");
		printf("len=%d\n", len);
*/	
		//end of marshaling
		
				
		slen=sizeof(main_Server.server_connection_map.at(toSendMessage.connid).connection_sock_addr);
	
		random = ((double) rand() / (RAND_MAX));

		//pick a random number and decides whether to send a message or not
		if(random>= drop_rate)//then send the message, else just ignore the sending
		{	
/*			if(lspmsg.payload.len > 0){
			printf("the server sends the following message, the rand number is= %f\n", random);
			printf("Sending the following to the client\n");	
			printf("lspmsg->connid: %d\n",lspmsg.connid );
			printf("lspmsg->seqnum: %d\n",lspmsg.seqnum );
			printf("lspmsg->payload.data: %s\n",lspmsg.payload.data);
			printf("lspmsg(back->payload.len): %d\n",lspmsg.payload.len);
			}
//			sending this message to the client
*/			if (sendto(main_Server.sockfd, buf, len+1, 0, (struct sockaddr*)&main_Server.server_connection_map.at
						(toSendMessage.connid).connection_sock_addr,slen)==-1)
			{
				printf("error in sending the message to the client\n ");
			}
	
		}
		
		else
		{
//			printf("the message is dropped, the rand number is= %f\n", random);
			;
		}
		
		
		
//		log the message, put a copy of it either in ConnectionDataLog or ConnectionAckQueueLog 
		
		if(toSendMessage.msgType == 2 )//0 for connection request, 1 for ack, 2 for data
			enqueue_ConnectionDataLog(toSendMessage.connid, toSendMessage);
		else if(toSendMessage.msgType == 1)//Ack
			enqueue_ConnectionAckQueueLog(toSendMessage.connid, toSendMessage);
		
		isempty = true;
		
	}

}

///////////////////////////////////////////////////////////
//////////////// Server's Epoch Thread ////////////////////
///////////////////////////////////////////////////////////

void* serverEpochThread(void *arg)
{
	
	struct message loggedSentData;
	struct message loggedReceivedAck;
	struct message dummyAck;
	struct message terminationMsg;
	time_t now;
	double differenceinSeconds;

	int index;
	bool isemptyData = true;
	bool isemptyAck = true;

	while(true)
	{   
//		printf("\n Epoch thread for server\n");
		sleep(epoch_lth);
		

		for(index=1; index<=main_Server.conncectionID; index++ )
		{
			
			time(&now);
			differenceinSeconds = difftime(now, main_Server.server_connection_map.at(index).lastReceivedDataTime);
			
				
//			printf(" differenceinSeconds for connection %d is %.f\n",index,  differenceinSeconds);
//			check the last time each connection received a data, if it is greater than the allowed, create a termination message and puch it in the Inbox 				Queue to be read by the server, the lsp_server_read will simply return -1 in that case
			if( epoch_lth*epoch_cnt < differenceinSeconds && main_Server.server_connection_map.at(index).alive == true)
			{
				printf("Server terminate connection: %d, no data has been received since: %f\n", index, epoch_lth*epoch_cnt);
//				insert this message with type = -1, to let the server nows that it's conection is terminated
				
				main_Server.server_connection_map.at(index).alive = false;				
				terminationMsg.msgType = -1;
				terminationMsg.connid = index;
				terminationMsg.seqnum = 0;
				terminationMsg.payloadLength = 0;
				terminationMsg.payload[0] = '\0';
	
				enqueue_ServerInboxQueue(terminationMsg);
				
			}
			
			else
			{
		
//				
				
//					check for the acknowledgments for the most recently received data messages from connection the server
					dequeue_ConnectionAckQueueLog(index, &loggedReceivedAck,&isemptyAck);
//					if there is any unacknowledged data, just resend it by putting it in the ServerOutboxQueue
					if(!isemptyAck)
						enqueue_ServerOutboxQueue(loggedReceivedAck);
					else
					{
//						printf("Epoch thread find no acknowledgments for the most recently received data messages for connection: %d\n", index);
//						check the dataReceivedBefore flag for all connections, send an Ack for the connection request if this flag is flase
						//no data has been received for this connection, resend the connection request ack 
//						if(main_Server.server_connection_map.at(index).dataReceivedBefore == false)
//						{
							dummyAck.msgType = 1;//0 for connection request, 1 for ack, 2 for data
							dummyAck.connid = index;
							dummyAck.seqnum = 0;
							dummyAck.payloadLength = 0;
							dummyAck.payload[0] = '\0';
							enqueue_ServerOutboxQueue(dummyAck);				

//						}
					}
				
//					check for unacknowledged sent data messages
					frontOf_ConnectionDataLog(index, &loggedSentData,&isemptyData);
//					if there is any unacknowledged logged data, just resend it by putting it in the ClientOutboxQueue
					if(!isemptyData)
					{
//						resend by putting it in the ServerOutboxQueue
						enqueue_ServerOutboxQueue(loggedSentData);
//						printf("Epoch thread find a data message in ConnectionDataLog and pushed it in ServerOutboxQueue: %d\n", index);	
						
						
/*						printf("This is the message that pushed:\n");	
						printf("loggedSentData.msgType: %d\n", loggedSentData.msgType );
						printf("loggedSentData.connid: %d\n", loggedSentData.connid );
						printf("loggedSentData.seqnum: %d\n", loggedSentData.seqnum );
						printf("loggedSentData.payloadLength: %d\n", loggedSentData.payloadLength );
						printf("loggedSentData.payload: %s\n", loggedSentData.payload );
*/					}
					else
					{
//						printf("Epoch thread find no unacknowledged data message for connection: %d\n", index);	
						;
					}
				
				
			}
			
		}
		isemptyData = true;
		isemptyAck = true;
	}
	
		
}


/*
 *
 *
 *				Server RELATED FUNCTIONS
 *
 *
 */  
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
////////////// Server's Create Function ///////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

lsp_server* lsp_server_create(int port)
{
	int sockfd;
	int status;
	
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
		printf("error in lsp_server_create\n");
		return NULL;
	}
	

//	initlaize and build the returnedServer 
	lsp_server* returnedServer =  (lsp_server*)malloc( sizeof(lsp_server) );
	returnedServer = (lsp_server*)malloc( sizeof(lsp_server) );
	
	returnedServer->sockfd = sockfd;
	returnedServer->port = port;	
	returnedServer->conncectionID = 0;
	bzero(&(returnedServer->my_addr), sizeof(returnedServer->my_addr));
	returnedServer->my_addr.sin_family = AF_INET;
	returnedServer->my_addr.sin_port = htons(port);
	returnedServer->my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	returnedServer->ServerInboxMutex = PTHREAD_MUTEX_INITIALIZER;
	returnedServer->ServerOutBoxMutex = PTHREAD_MUTEX_INITIALIZER;


//	binding for the server
	if (bind(sockfd, (struct sockaddr* ) &returnedServer->my_addr, sizeof(returnedServer->my_addr))==-1)
	{
		printf("error in server binding\n");
		return NULL;
	}
	 
	 

//	initialize the threads for the server
	
	status = pthread_create(&(returnedServer->threadsID[0]), NULL,&serverReceivngThread, NULL);
   	if (status != 0)
            printf("\nCan't create Server's Receiving Thread :[%s]", strerror(status));
      else
            printf("\n  Server's Receiving Thread created successfully\n");
            
      status = pthread_create(&(returnedServer->threadsID[1]), NULL,&serverSendingThread, NULL);
   	if (status != 0)
            printf("\nCan't create Server's Sending Thread :[%s]", strerror(status));
      else
            printf("\n  Server's Sending Thread created successfully\n");
	
	status = pthread_create(&(returnedServer->threadsID[2]), NULL,&serverEpochThread, NULL);
   	if (status != 0)
            printf("\nCan't create Server's Epoch Thread :[%s]", strerror(status));
      else
            printf("\n  Server's Epoch Thread created successfully\n");
	

//	printf("Create Server Successfully\n");
	main_Server = *returnedServer;
	return returnedServer;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
////////////// Server's Write Function ////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

bool lsp_server_write(lsp_server* a_srv, uint8_t* pld, int lth, uint32_t conn_id)
{
	bool isConnectionDataLogempty = true;
	bool isConnectionToBeSentQueueempty;
//	printf("Strat of Write method in the server \n");
//	check if the server tries to send NULL, if yes prevent it and return false
	if(pld == NULL)
	{
		printf("error, the server tries to send NULL");
		return false;
	}
	
	if(conn_id  > main_Server.conncectionID)
	{
		printf("error, there is no client with this connection %d\n", conn_id);
		return false;
	}
	
      struct message enteredMsg;
	
//	prepare the recieved message to be put in the queue
	
	enteredMsg.msgType = 2;	
//	fill the connection ID
	enteredMsg.connid = conn_id;
	enteredMsg.payloadLength = lth;
	memcpy(enteredMsg.payload, pld, lth*sizeof(uint8_t));
  

  
 	//check the ConnectionDataLog, if it is empty, that means this server does not wait for an Ack for some previous messages
	isEmpty_ConnectionDataLog(conn_id, &isConnectionDataLogempty);
	isEmpty_ConnectionToBeSentQueue(conn_id, &isConnectionToBeSentQueueempty);
	if(isConnectionDataLogempty == true && isConnectionToBeSentQueueempty == true )//prepare and push the enteredMsg in the ServerOutboxQueue to be sent directly
	{
//		printf("Insert directly in ServerOutboxQueue the both of ConnectionDataLog and ConnectionToBeSentQueue are empty \n");
		enteredMsg.seqnum = ++ main_Server.server_connection_map.at(conn_id).seqnum;// increment the sequence number of the sent message here
		enqueue_ServerOutboxQueue(enteredMsg);
	}
	
	else//there are some unacknowledged messages this client waiting for, the Sequence number will be filled by the serverReceivngThread later
	{
//		push the message in the ConnectionToBeSentQueue to be sent	
//		printf("Insert in ConnectionToBeSentQueue the ConnectionDataLog and ConnectionToBeSentQueue are not empty \n");
		enqueue_ConnectionToBeSentQueue(conn_id, enteredMsg);
	}
  

/*
	printf("pld: %s\n", pld );
	printf("The Server writes the following to be sent\n");	
	printf("enteredMsg.msgType: %d\n", enteredMsg.msgType );
	printf("enteredMsg.connid: %d\n", enteredMsg.connid );
	printf("enteredMsg.seqnum: %d\n", enteredMsg.seqnum );
	printf("enteredMsg.payloadLength: %d\n", enteredMsg.payloadLength );
	printf("enteredMsg.payload: %s\n", enteredMsg.payload );
*/	
	return true;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
////////////// Server's Read Function ////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

int  lsp_server_read(lsp_server* a_srv, uint8_t* pld, uint32_t* conn_id)
{

      struct message returnedMsg;
	
	bool isempty = true;
	int pld_size;
	int index;
	
	//pop from the ServerInboxQueue
	dequeue_ServerInboxQueue(&returnedMsg,&isempty);
	
	if(isempty)
	{
//		printf("nothing to be read by the server\n");
		return 0;
	}     
	
	else
	{
	
		if(returnedMsg.msgType == -1)//termination for this conncetion
		{
			*conn_id = returnedMsg.connid;
			return -1;
		}
		memcpy(pld, returnedMsg.payload, returnedMsg.payloadLength*sizeof(uint8_t));
		*conn_id = returnedMsg.connid;
		//retur the number of read bytes
		return returnedMsg.payloadLength;
	}
	
	
}






///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
////////////// Server's Close Function ////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool lsp_server_close(lsp_server* a_srv, uint32_t conn_id)
{
//	make this connection as a dead one
	main_Server.server_connection_map.at(conn_id).alive = false;				
	
	
	pthread_join( main_Server.threadsID[0], NULL);
	pthread_join( main_Server.threadsID[1], NULL);	
	pthread_join( main_Server.threadsID[2], NULL);
	
	return true;
	
}





//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
////////////////// CLIENT LSP API /////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//Global Client Variable
lsp_client current_Client;
//this flag is used to kill threads when the client loses its connection with the server
bool threadKillingFlag;


////////////////Client's Queues' methods//////////////////////


queue<struct message> ClientInboxQueue;
queue<struct message> ClientOutboxQueue;
queue<struct message> ClientAckQueueLog;
queue<struct message> ClientDataLog;
queue<struct message> ClientToBeSentQueue;

//enqueue the ClientInboxQueue	
void enqueue_ClientInboxQueue(struct message entry)
{
	pthread_mutex_lock(&current_Client.ClientInboxMutex);
	ClientInboxQueue.push(entry);
	pthread_mutex_unlock(&current_Client.ClientInboxMutex);
}
//dequeue the ClientInboxQueue	
void dequeue_ClientInboxQueue(struct message *entry, bool *isempty)
{
	pthread_mutex_lock(&current_Client.ClientInboxMutex);
	if( ClientInboxQueue.empty() )
		*isempty = true;
	else{
		*entry = ClientInboxQueue.front();
		ClientInboxQueue.pop();
		*isempty = false;
	}
	pthread_mutex_unlock(&current_Client.ClientInboxMutex);
}

//enqueue the ClientOutboxQueue	
void enqueue_ClientOutboxQueue(struct message entry)
{

	pthread_mutex_lock(&current_Client.ClientOutBoxMutex);		
	ClientOutboxQueue.push(entry);
	pthread_mutex_unlock(&current_Client.ClientOutBoxMutex);
}

//dequeue the ClientOutboxQueue	
void dequeue_ClientOutboxQueue(struct message *entry, bool *isempty)
{	
	pthread_mutex_lock(&current_Client.ClientOutBoxMutex);
	if( !ClientOutboxQueue.size() )
		*isempty = true;
	else{
		*entry = ClientOutboxQueue.front();
		ClientOutboxQueue.pop();
		*isempty = false;
	}
	pthread_mutex_unlock(&current_Client.ClientOutBoxMutex);
}


//enqueue the ClientAckQueueLog	
void enqueue_ClientAckQueueLog( struct message entry)
{
	pthread_mutex_lock(&current_Client.ClientAckMutex);
	ClientAckQueueLog.push(entry);
	pthread_mutex_unlock(&current_Client.ClientAckMutex);
}

//dequeue the ClientAckQueueLog	
void dequeue_ClientAckQueueLog( struct message *entry, bool *isempty)
{
	pthread_mutex_lock(&current_Client.ClientAckMutex);
	if( ClientAckQueueLog.empty() )
		*isempty = true;
	else{
		*entry = ClientAckQueueLog.front();
		ClientAckQueueLog.pop();
		*isempty = false;
	}
	pthread_mutex_unlock(&current_Client.ClientAckMutex);
}

//enqueue the ClientDataLog	
void enqueue_ClientDataLog( struct message entry)
{
	pthread_mutex_lock(&current_Client.ClientDataLogMutex);
	ClientDataLog.push(entry);
	pthread_mutex_unlock(&current_Client.ClientDataLogMutex);
}

//dequeue the ClientDataLog	
void dequeue_ClientDataLog( struct message *entry, bool *isempty)
{
	pthread_mutex_lock(&current_Client.ClientDataLogMutex);
	if( ClientDataLog.empty() )
		*isempty = true;
	else{
		*entry = ClientDataLog.front();
		ClientDataLog.pop();
		*isempty = false;
	}
	pthread_mutex_unlock(&current_Client.ClientDataLogMutex);
}

//take a copy of the front without pop, needs for keep alive prtocol
void frontOf_ClientDataLog(struct message *entry, bool *isempty)
{
	pthread_mutex_lock(&current_Client.ClientDataLogMutex);
	if( ClientDataLog.empty() )
		*isempty = true;
	else{
		*entry = ClientDataLog.front();
		*isempty = false;
	}
	pthread_mutex_unlock(&current_Client.ClientDataLogMutex);
}

//return whether the ClientDataLog is empty or not
void isEmpty_ClientDataLog(bool *isempty)
{
	pthread_mutex_lock(&current_Client.ClientDataLogMutex);
	if( ClientDataLog.empty() )
		*isempty = true;
	else
		*isempty = false;
	pthread_mutex_unlock(&current_Client.ClientDataLogMutex);
}

//enqueue the ClientToBeSentQueue	
void enqueue_ClientToBeSentQueue(struct message entry)
{
	pthread_mutex_lock(&current_Client.ClientToBeSentMutex);
	ClientToBeSentQueue.push(entry);
	pthread_mutex_unlock(&current_Client.ClientToBeSentMutex);
}
//dequeue the ClientToBeSentQueue	
void dequeue_ClientToBeSentQueue(struct message *entry, bool *isempty)
{
	pthread_mutex_lock(&current_Client.ClientToBeSentMutex);
	if( ClientToBeSentQueue.empty() )
		*isempty = true;
	else{
		*entry = ClientToBeSentQueue.front();
		ClientToBeSentQueue.pop();
		*isempty = false;
	}
	pthread_mutex_unlock(&current_Client.ClientToBeSentMutex);
}


//return whether the ClientDataLog is empty or not
void isEmpty_ClientToBeSentQueue(bool *isempty)
{
	pthread_mutex_lock(&current_Client.ClientToBeSentMutex);
	if( ClientToBeSentQueue.empty() )
		*isempty = true;
	else
		*isempty = false;
	pthread_mutex_unlock(&current_Client.ClientToBeSentMutex);
}

/*
*
*
*			Client Threads
*
*
*
*/

///////////////////////////////////////////////////////////
//////////////// Client's Receivng Thread /////////////////
///////////////////////////////////////////////////////////

void* clientReceivngThread(void *arg)
{	
//	struct for different messages
	struct message receivedMessage;
	struct message ackForReceivedMessage;
	struct message loggedMessageforData;	
	struct message nextMessagetoSend;	
	bool isempty = true;
	socklen_t slen = sizeof(current_Client.serv_addr);
	uint8_t buf[MAX_SG_SIZE];
	int receivedBufferlen;
	int index;
	int i;
	
	
	
	while(true) 
	{
		sleep(0.3);

//		printf("\n Receiving thread for client\n");
		
		
//		receiving the message from the server
		if (recvfrom(current_Client.sockfd, buf, MAX_SG_SIZE, 0, (struct sockaddr*)&(current_Client.serv_addr),&slen)==-1)
		{
			printf("error in receiving message from the server\n");
			
		}
	
//		preparing for unmarshaling

		receivedBufferlen = buf[0];
		uint8_t tempBuffer[receivedBufferlen]; 
		for(index=0;index<receivedBufferlen;index++)
			tempBuffer[index] = buf[index+1];

/*
		printf("\n");
		for(i=0; i<MAX_SG_SIZE; i++)
			printf("%d", buf[i]);
		printf("\n");
*/	
	
//		unmarshaling the message
		LSPMessage *lspmsg;
		lspmsg = lspmessage__unpack(NULL,  receivedBufferlen, tempBuffer); 
	   
		if (lspmsg == NULL)
		{     //Something failed
			printf("error while unmarshaling incoming message from server\n");

		}

//		printf("Client Receive a Packet From:%s\n: %d\n", inet_ntoa(current_Client.serv_addr.sin_addr), ntohs(current_Client.serv_addr.sin_port));
	
	
		//fill the fields for the receivedMessage struct
		receivedMessage.connid = lspmsg->connid;
		receivedMessage.seqnum = lspmsg->seqnum;
		receivedMessage.payloadLength = lspmsg->payload.len;
		memcpy(receivedMessage.payload,lspmsg->payload.data,  lspmsg->payload.len*sizeof(uint8_t));
	  

/*		printf("The Client recieved the following from the server\n");	
		printf("receivedMessage.msgType: %d\n", receivedMessage.msgType );
		printf("receivedMessage.connid: %d\n", receivedMessage.connid );
		printf("receivedMessage.seqnum: %d\n", receivedMessage.seqnum );
		printf("receivedMessage.payloadLength: %d\n", receivedMessage.payloadLength );
		printf("receivedMessage.payload: %s\n", receivedMessage.payload );
*/		
	
//		the client supposed to recieve wehter an Ack or a data from the server
//		check the type of the received message depending on the payload's length

		if(lspmsg->payload.len == 0)//this is for Ack
		{
			receivedMessage.msgType = 1;
//			check the sequence number of the received Ack, 0 for connection request, 1 for ack, 2 for data
      		if(receivedMessage.seqnum == 0)// this is a connection request Ack, set the coonection ID for this client
      		{	
      			
//				check the connection ID for this client, if it is zero then pop a message from the set it
//				otherwise just ignore the Ack, it is resent because of the keep alive protocol we have
      			if(current_Client.conncectionID == 0)
      			{	
//					set the connection ID, and pop another message from the ClientToBeSentQueue and insert it in the ClientOutboxQueue to be sent
					current_Client.conncectionID = receivedMessage.connid;

					
					dequeue_ClientToBeSentQueue(&nextMessagetoSend,&isempty);
					if(isempty == false)//fill the connection ID and seq number & push it in ClientOutboxQueue
					{
						nextMessagetoSend.connid = current_Client.conncectionID;
						nextMessagetoSend.seqnum = ++ current_Client.seqnum;// increment the sequence number of the sent message
						enqueue_ClientOutboxQueue(nextMessagetoSend);
//						time stamp for the lastReceivedDataTime for this client 
						time(&current_Client.lastReceivedDataTime);
					}
					else
					{
//						printf("ClientOutboxQueue is empty, nothing to be sent to the server!\n");
						;
				}	}
				else// error, dismatch of sequence number	and enqueue the loggedMessageforData again in the ClientDataLog to be logged again in the future
				{
					//time stamp for the lastReceivedDataTime for this client 
					time(&current_Client.lastReceivedDataTime);
//					printf("receive another connection request ID because of the live sequence protocol!\n");
					;
				}
			}
      		
      		else// this is an Ack for data
      		{
      			
//					time stamp for the lastReceivedDataTime for this client 
					time(&current_Client.lastReceivedDataTime);
//					dequeue the ClientDataLog and compare the sequence number of the
//					two meeesages and they must be at least equal because of the sequence protocl we built!

					dequeue_ClientDataLog(&loggedMessageforData,&isempty);
					if(isempty == true)
					{
//						printf("receive an ack for some old data because of the live sequence protocol!\n");
						;
					}
					else
						if(loggedMessageforData.seqnum == receivedMessage.seqnum)
						{
//							printf("match of two seq numbers, pop antoher message to be sent!\n");
//							pop another message from the ClientToBeSentQueue and insert it in the ClientOutboxQueue to be sent
							dequeue_ClientToBeSentQueue(&nextMessagetoSend,&isempty);
							if(isempty == false)//fill the connection ID and seq number & push it in ClientOutboxQueue
							{
								nextMessagetoSend.connid = current_Client.conncectionID;
								nextMessagetoSend.seqnum = ++ current_Client.seqnum;// increment the sequence number of the sent message
								enqueue_ClientOutboxQueue(nextMessagetoSend);
							}
						else
							{
//								printf("ClientOutboxQueue is empty, nothing to be sent to the server!\n");
								;
							}
						}
						// error, dismatch of sequence number, this is recieved because of the keep alive protocol we build	
						else if(receivedMessage.seqnum < loggedMessageforData.seqnum )
						{
//							printf("dismatch of sequence number, receive an ack for some old data because of the live sequence protocol!\n");
							;
						
						}
						else//almost impossible, recieve an Ack for large sequence number
						{
//							printf("dismatch of sequence number, receive an ack for some data with a sequence number which is gratet than the 							expected!\n");
							
						}
			
      		}
		}	
		else//this is for data
		{

//			time stamp for the lastReceivedDataTime for this client 
			time(&current_Client.lastReceivedDataTime);
//			check the last_data_received_seqnum for this client 
//			printf("Recceive a message  with sequence number=%d\n", receivedMessage.seqnum);
			if(current_Client.last_data_received_seqnum == 0)//this is the first received message
			{

				if(receivedMessage.seqnum == 1)//this is the first message received, and it's sequence number must be 1, else, the first message is dropped, so do NOT ack this message until the other side send a message with sequence number 1

				{
//					printf("The first message recceived with a sequence number=%d\n", receivedMessage.seqnum);
					current_Client.last_data_received_seqnum = receivedMessage.seqnum;
				
					receivedMessage.msgType = 2;
					enqueue_ClientInboxQueue(receivedMessage);
			
			
	//				create a an Ack for this received data message and enqueue it directly in the outbox queue to be sent soon by the sending thread
					ackForReceivedMessage.msgType = 1;
					ackForReceivedMessage.connid = receivedMessage.connid;
					ackForReceivedMessage.seqnum = receivedMessage.seqnum;
					ackForReceivedMessage.payloadLength = 0;
					ackForReceivedMessage.payload[0] = '\0';
					enqueue_ClientOutboxQueue(ackForReceivedMessage);
				}
				else//just ignore the receined message
//				printf("Ignore the message recceived with a sequence number=%d\n", receivedMessage.seqnum);
				;
			}
//			set the message type to 2, and enqueue it in the inbox queue to be read by the cleint

			else if (receivedMessage.seqnum == current_Client.last_data_received_seqnum +1 ) 
			{
//				printf("Match the received message with the expected sequencen number + 1=%d\n", current_Client.last_data_received_seqnum+1);
//				increment the last_data_received_seqnum for this connection
				
				current_Client.last_data_received_seqnum ++;
				
				receivedMessage.msgType = 2;
				enqueue_ClientInboxQueue(receivedMessage);
			
			
//				create a an Ack for this received data message and enqueue it directly in the outbox queue to be sent soon by the sending thread
				ackForReceivedMessage.msgType = 1;
				ackForReceivedMessage.connid = receivedMessage.connid;
				ackForReceivedMessage.seqnum = receivedMessage.seqnum;
				ackForReceivedMessage.payloadLength = 0;
				ackForReceivedMessage.payload[0] = '\0';
				enqueue_ClientOutboxQueue(ackForReceivedMessage);
			}
			
			else if(receivedMessage.seqnum <= current_Client.last_data_received_seqnum)// just ignore the message, duplication, but we have to send an Ack becuase of the lat one might be lost
			{
//				printf("NO Match between the received message and the expected sequencen number + 1=%d\n", current_Client.last_data_received_seqnum+1);
//				printf("the client receive a duplicated data message because of the live sequence protocol we build \n");
				//create a an Ack for this received data message and enqueue it directly in the outbox queue to be sent soon by the sending thread
				ackForReceivedMessage.msgType = 1;
				ackForReceivedMessage.connid = receivedMessage.connid;
				ackForReceivedMessage.seqnum = receivedMessage.seqnum;
				ackForReceivedMessage.payloadLength = 0;
				ackForReceivedMessage.payload[0] = '\0';
				enqueue_ClientOutboxQueue(ackForReceivedMessage);
			}
			
		}
		isempty = true;

	}
	
	if(threadKillingFlag)
		return 0;
}

///////////////////////////////////////////////////////////
//////////////// Client's Sending Thread //////////////////
///////////////////////////////////////////////////////////

void* clientSendingThread(void *arg)
{

	struct message toSendMessage;
	bool isempty;
	isempty = true;
	LSPMessage lspmsg = LSPMESSAGE__INIT;
	uint8_t* buf;
	uint8_t* tempBuffer;
	double random;
	int index;
	int len;
	int i;

	

	while (true)
	{
		
//		printf("\n Sending thread for client\n");
		
		while(isempty)
		{	
//			printf("outbox queue is empty\n");
			dequeue_ClientOutboxQueue(&toSendMessage,&isempty);
			sleep(0.3);
		}
		
//		marshaling the message to send it
		lspmsg.connid =  toSendMessage.connid;	
		lspmsg.seqnum =  toSendMessage.seqnum;
		lspmsg.payload.data = (uint8_t*) malloc(sizeof(uint8_t) * toSendMessage.payloadLength);
		lspmsg.payload.len = toSendMessage.payloadLength;
		memcpy(lspmsg.payload.data, toSendMessage.payload, toSendMessage.payloadLength*sizeof(uint8_t));
	  
	
		len = lspmessage__get_packed_size(&lspmsg);
		buf = (uint8_t*)malloc(len+1);
		tempBuffer = (uint8_t*)malloc(len);
		lspmessage__pack(&lspmsg, tempBuffer);
		buf[0] = len;
		for(index=0;index<len+1;index++)
			buf[index+1] = tempBuffer[index];
		


			
/*		printf("\n");

		for(i=0; i<len+1; i++)
			printf("%d ", buf[i]);
		printf("\n");
		printf("len=%d\n", len);
*/	
		//end of marshaling
		
//		generate a random number to drop some packets by just not sending them 
		
		random = ((double) rand() / (RAND_MAX));
		
		if(random>= drop_rate)//then send the message, else just ignore the sending
		{	
/*			if(lspmsg.payload.len > 0){
			printf("The Client is Sending the following to the server\n");	
			printf("lspmsg->connid: %d\n",lspmsg.connid );
			printf("lspmsg->seqnum: %d\n",lspmsg.seqnum );
			printf("lspmsg->payload.data: %s\n",lspmsg.payload.data);
			printf("lspmsg(back->payload.len): %d\n",lspmsg.payload.len);
			}
			printf("the message is sent, the rand number is= %f\n", random);
//			sending this message to the server

*/			if (sendto(current_Client.sockfd, buf, len+1, 0, (struct sockaddr*)&current_Client.serv_addr, sizeof(current_Client.serv_addr))==-1)
			{
				printf("error in sending the message to the server\n ");

			}
		
		}
		
		else
		{
//			printf("the message is dropped, the rand number is= %f\n", random);
			;
		}
//		log the message, put a copy of it either in ClientAckQueueLog or ClientDataLog 
		
		if(toSendMessage.msgType == 2 )//0 for connection request, 1 for ack, 2 for data
			enqueue_ClientDataLog(toSendMessage);
		else if(toSendMessage.msgType == 1)//Ack
			enqueue_ClientAckQueueLog(toSendMessage);
		
		isempty = true;
		
	}
	if(threadKillingFlag)
		return 0;
}	
		

///////////////////////////////////////////////////////////
//////////////// Client's Epoch Thread ////////////////////
///////////////////////////////////////////////////////////

void* clientEpochThread(void *arg)
{
	
	struct message loggedSentData;
	struct message loggedReceivedAck;
	struct message dummyAck;
	bool isemptyData = true;
	bool isemptyAck = true;
	time_t now;
	double differenceinSeconds;

	while(true)
	{   
//		printf("\n Epoch thread for client\n");
		sleep(epoch_lth);
		
		
		time(&now);
		differenceinSeconds = difftime(now, current_Client.lastReceivedDataTime);
//		printf(" differenceinSeconds for client is %.f\n", differenceinSeconds);
		
		if( epoch_lth*epoch_cnt < differenceinSeconds)
		{
			struct message terminationMsg;

//			printf("\n!!!!!!!!!!!!!! Connection Broken !!!!!!!!!!!!!! \n");
//			printf("Nothing received from the server since:%d seconds\n", waitingTime);
	
//			insert this message with type = -1, to let the client nows that it's conection is terminated
			terminationMsg.msgType = -1;
			terminationMsg.seqnum = 0;
			terminationMsg.payloadLength = 0;
			terminationMsg.payload[0] = '\0';	
			enqueue_ClientInboxQueue(terminationMsg);
			pthread_join( current_Client.threadsID[0], NULL);
			pthread_join( current_Client.threadsID[1], NULL);
			pthread_join( current_Client.threadsID[2], NULL);
			threadKillingFlag = true;
	
		}
	
		
		if(current_Client.conncectionID == 0)
		{

			struct message connectionRequestMsg;
			connectionRequestMsg.msgType = 0;//connection request
			connectionRequestMsg.connid = 0;//connection ID
			connectionRequestMsg.seqnum = 0;
			connectionRequestMsg.payloadLength = 0;
			connectionRequestMsg.payload[0] = '\0';
			enqueue_ClientOutboxQueue(connectionRequestMsg);
//			sleep(1.0);
			
		}
		
		else
		{
//			check for the acknowledgments for the most recently received data messages from the server
			dequeue_ClientAckQueueLog(&loggedReceivedAck,&isemptyAck);
//			if there is any unacknowledged logged connection request or data, just resend it by putting it in the ClientOutboxQueue
			if(!isemptyAck)
				enqueue_ClientOutboxQueue(loggedReceivedAck);
			else
			{
//				printf("Epoch thread find no acknowledgments for the most recently received data messages from the server, it will create an Ack with 					sequence number = 0\n");
				dummyAck.msgType = 1;
				dummyAck.connid = current_Client.conncectionID;
				dummyAck.seqnum = 0;
				dummyAck.payloadLength = 0;
				dummyAck.payload[0] = '\0';
				enqueue_ClientOutboxQueue(dummyAck);
			
			}
			
//			check for unacknowledged sent data messages
			frontOf_ClientDataLog(&loggedSentData,&isemptyData);
//			if there is any unacknowledged logged connection request or data, just resend it by putting it in the ClientOutboxQueue
			if(!isemptyData)
//				resend by putting it in the ClientOutboxQueue
				enqueue_ClientOutboxQueue(loggedSentData);
			else
			{
//				printf("Epoch thread find no unacknowledged data message or a connection request for resending \n");	
				;
			}
		}
		isemptyData = true;
		isemptyAck = true;
		if(threadKillingFlag)
			return 0;
	}
		
}




/*
 *
 *
 *				CLIENT RELATED FUNCTIONS
 *
 *
 */  
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
////////////// Client's Create Function ///////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

lsp_client* lsp_client_create(const char* dest, int port)
{
	
	struct sockaddr_in serv_addr;
	int sockfd;
	int returnedBytes;
	int status;
	uint8_t buffer[MAX_SG_SIZE];
	threadKillingFlag = false;
    	
    
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
		printf("error in lsp_client_create\n");
		return false;
	}

		
		
	//create and initialize lsp_client
	lsp_client* returnedClient =  (lsp_client*)malloc( sizeof(lsp_client));
	returnedClient->dest = const_cast<char *> (dest);
	returnedClient->port = port;
	returnedClient->conncectionID = 0;
	returnedClient->seqnum = 0;
	returnedClient->last_data_received_seqnum = 0;
	returnedClient->sockfd = sockfd;
	//time stamp for the lastReceivedDataTime for this client 
	time(&returnedClient->lastReceivedDataTime);
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port =htons(port);
	returnedClient->serv_addr = serv_addr;
	
	//fill the returned client fields
	returnedClient->ClientInboxMutex = PTHREAD_MUTEX_INITIALIZER;
	returnedClient->ClientOutBoxMutex = PTHREAD_MUTEX_INITIALIZER;
	returnedClient->ClientAckMutex = PTHREAD_MUTEX_INITIALIZER;
	returnedClient->ClientDataLogMutex = PTHREAD_MUTEX_INITIALIZER;
	returnedClient->ClientToBeSentMutex = PTHREAD_MUTEX_INITIALIZER;


				
				
				
	//building the connection request for the client
	struct message connectionRequestMsg;
	connectionRequestMsg.msgType = 0;//connection request
	connectionRequestMsg.connid = 0;//connection ID
	connectionRequestMsg.seqnum = 0;
	connectionRequestMsg.payloadLength = 0;
	connectionRequestMsg.payload[0] = '\0';
	
	//enqueue the connection request in the outbox to be sent
	enqueue_ClientOutboxQueue(connectionRequestMsg);
	
	
	
	//initialize the threads for the client
	
	status = pthread_create(&(returnedClient->threadsID[0]), NULL,&clientSendingThread, NULL);
   	if (status != 0)
            printf("\nCan't create Client's Sending Thread :[%s]", strerror(status));
      else
            printf("\n  Client's Sending Thread created successfully\n");
            
      status = pthread_create(&(returnedClient->threadsID[1]), NULL,&clientEpochThread, NULL);
   	if (status != 0)
            printf("\nCan't create Client's Epoch :[%s]", strerror(status));
      else
            printf("\n  Client's Epoch Thread created successfully\n");
	
	status = pthread_create(&(returnedClient->threadsID[2]), NULL,&clientReceivngThread, NULL);
   	if (status != 0)
            printf("\nCan't create Client's Receiving Thread:[%s]", strerror(status));
      else
            printf("\n Client's Receiving Thread created successfully\n");
          
      current_Client = *returnedClient;  
      

      return returnedClient;    
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
////////////// Client's Write Function ////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

bool lsp_client_write(lsp_client* a_client, uint8_t* pld, int lth)
{
	bool isClientDataLogempty = true;
	bool isClientToBeSentQueueempty = true;
//	printf("Strat of Write method in client \n");
//	check if the client tries to send NULL, if yes prevent it and return false
	if(pld == NULL)
	{
		printf("error, the cleint tries to send NULL");
		return false;
	}
	
      struct message enteredMsg;
	
//	prepare the recieved message to be put in the queue
	
	enteredMsg.msgType = 2;	
//	the connection ID and the Sequence number are filled when the previous message is received i.e. in the receiving thread of the client
	enteredMsg.payloadLength = lth;
	memcpy(enteredMsg.payload, pld, lth*sizeof(uint8_t));
	
//	check the ClientDataLog, if it is empty, that means this client does not wait for an Ack for some previous messages
	sleep(1.0);
	isEmpty_ClientDataLog(&isClientDataLogempty);
	isEmpty_ClientToBeSentQueue(&isClientToBeSentQueueempty);

	if(isClientDataLogempty == true && isClientToBeSentQueueempty == true)//prepare and push the enteredMsg in the ClientOutboxQueue to be sent directly
	{
		while(current_Client.conncectionID == 0)//do not continue until this client receive the Ack for the connection request from the server
			sleep(1.0);
			
//		printf("Insert directly in ClientOutboxQueue the both of ClientDataLog and ClientToBeSentQueue are empty \n");

		enteredMsg.connid = current_Client.conncectionID;
		enteredMsg.seqnum = ++ current_Client.seqnum;// increment the sequence number of the sent message
		enqueue_ClientOutboxQueue(enteredMsg);
	}
	
	else//there are some unacknowledged messages theis client waiting for
	{
 //		push the message in the ClientToBeSentQueue to be sent	
		enqueue_ClientToBeSentQueue(enteredMsg);
//		printf("Insert in ClientToBeSentQueue the ClientDataLog and ClientToBeSentQueue are not empty \n");
	}
//	printf("pld: %s\n", pld );
/*	printf("The Client writes the following to be sent\n");	
	printf("enteredMsg.msgType: %d\n", enteredMsg.msgType );
	printf("enteredMsg.connid: %d\n", enteredMsg.connid );
	printf("enteredMsg.seqnum: %d\n", enteredMsg.seqnum );
	printf("enteredMsg.payloadLength: %d\n", enteredMsg.payloadLength );
	printf("lenteredMsg.payload: %s\n", enteredMsg.payload );
*/	
	return true;
}




///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
////////////// Client's Read Function ////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

int lsp_client_read(lsp_client* a_client, uint8_t* pld)
{

      struct message returnedMsg;
	
	bool isempty = true;
	int pld_size;
	int index;
	
	dequeue_ClientInboxQueue(&returnedMsg,&isempty);
	if(isempty == false)
	{
//		if the message type is -1, that means the client lost its connection with the server and terminated
		if(returnedMsg.msgType == -1)//termination for this client, the server is down
		{
			return -1;
		}
		else
		{
//			return the number of read bytes
			memcpy(pld, returnedMsg.payload, returnedMsg.payloadLength*sizeof(uint8_t));
			return returnedMsg.payloadLength;
		}
	}
	
	else
		return 0;	
	
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
////////////// Client's Close Function ////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool lsp_client_close(lsp_client* a_client)
{
	//free for a_client
	free(a_client);
	
	//killing the threads
	pthread_kill( current_Client.threadsID[0], 0);
	pthread_kill( current_Client.threadsID[1], 0);
	pthread_kill( current_Client.threadsID[2], 0);
	return true;
	
}



