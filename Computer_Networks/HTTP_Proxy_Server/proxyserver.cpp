/*Server.cpp*/

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
#include <time.h>
#include <map>
#include "defn.h"
using namespace std;

struct pageResp
{
string msg;
string status;
string header;
string htmlContent;
string expires;
string lastUsed;
string lastModified;
bool foundExpired;
bool foundLM;
};

map<string,pageResp> Cache;

string currTime()
{
  time_t rawtime;
  struct tm * ptm;
  time ( &rawtime );
  ptm = gmtime ( &rawtime );
  char timebuff [80];
  strftime (timebuff,80,"%a, %d %b %Y %H:%M:%S %Z",ptm);
  return string(timebuff);
}

int timeCompare(string time1, string time2)
{
  time_t ctime1,ctime2;
  float diff;
  const char* chtime1=time1.c_str();
  const char* chtime2=time2.c_str();
  struct tm tm1;
  struct tm tm2;
  memset(&tm1, 0, sizeof(struct tm));
  strptime(chtime1, "%a, %d %b %Y %H:%M:%S %Z", &tm1);
  memset(&tm2, 0, sizeof(struct tm));
  strptime(chtime2, "%a, %d %b %Y %H:%M:%S %Z", &tm2);
  ctime1 = mktime(&tm1);
  ctime2 = mktime(&tm2);
  diff = difftime(ctime1, ctime2);
  if(diff<0)
    return -1;
  else if(diff>0)
    return 1;
  else
    return 0;
}

void addToCache(string req, struct pageResp resp)
{
  resp.lastUsed=currTime();
  bool flagReplace=false;
  map<string,struct pageResp>::iterator iter;
  for (iter=Cache.begin();iter != Cache.end(); iter++)
  {
    if(req.compare(iter->first)==0)
    {
      flagReplace=true;
      Cache.erase(iter);
      Cache.insert(pair<string,struct pageResp>(req,resp));
      break;
    }
  }
  if(flagReplace==false)
  {
    if (Cache.size()<=5){
      Cache.insert(pair<string,struct pageResp>(req,resp));
      cout<<"in the ft, added last modiied time: "<<resp.lastModified<<endl;
    }
    else
    {
      string minm;
      iter=Cache.begin();
      minm=(iter->second).lastUsed;
      iter++;
      for (iter; iter != Cache.end(); iter++)
      {
        if(timeCompare(minm,(iter->second).lastUsed)==1)
          minm=(iter->second).lastUsed;
      }
      iter=Cache.begin();
      while((iter->second).lastUsed != minm)
       iter++;
      Cache.erase(iter);
      Cache.insert(pair<string,struct pageResp>(req,resp));
     }
      map<string,struct pageResp>::iterator iter;
      for (iter=Cache.begin();iter != Cache.end(); iter++)
      {
        cout<<"map first: "<<iter->first<<endl;
        cout<<"map second elem: "<<(iter->second).status<<endl;
        cout<<"map last modified time: "<<(iter->second).lastModified<<endl;
      }
   }
      return;
}

int compareHead(string lmTime, const char* request, const char* url, char* servPort,string& getExpired)
{
  char largeBuf[MAX_CONTENT_BUF_SIZE];
  string buff;
  string tempbuf, modTime, modExpires;
  int pos,pos2;
  int connectVal,servFD;
  struct addrinfo tempp, *ress;
  memset(&tempp, 0, sizeof(tempp));
  tempp.ai_family = AF_INET;
  tempp.ai_socktype = SOCKET_TYPE;
  if((connectVal = getaddrinfo(url,servPort, &tempp, &ress)) != 0)
  {
     cout << "Bad Request. Unable to get server address: "<<url<<endl;
     return 2;
  }
  else
  {
     cout << "Connecting to server: " <<url<<endl;
     servFD = socket(ress->ai_family, ress->ai_socktype, ress->ai_protocol);
     if(servFD == -1)
     {
        perror("Error while creating socket for connecting to server");
        return 2;
     }
     else
     {
        if(connect(servFD, ress->ai_addr,ress->ai_addrlen)==-1)
        {
           perror("connect() error while connecting to server");
           close(servFD);
           return 2;
        }
        else
        {
            cout << "Connection to server established!" << endl;
            freeaddrinfo(ress);

            cout << "Sending HTTP GET message:" << endl << request << endl;
            if(send(servFD, request, strlen(request), 0) == -1)
            {
              cout << "Could not send GET" << endl;
              close(servFD);
            }
            while(1)
            {
               int recvbytes = recv(servFD, largeBuf, MAX_CONTENT_BUF_SIZE, 0);
               if(recvbytes <= 0)
               {
                  break;
               }
               else
               {
                  buff=buff.append(string(largeBuf));
                  memset(largeBuf, 0, recvbytes);
               }
            }

            if ((pos=buff.find("Last-Modified: "))!=string::npos)
            {
               tempbuf=buff.substr(pos);
               pos2=tempbuf.find("\r\n");
               modTime=buff.substr(pos+15,pos2-15);
            }
            if ((pos=buff.find("Expires: "))!=string::npos)
            {
               tempbuf=buff.substr(pos);
               pos2=tempbuf.find("\r\n");
               modExpires=buff.substr(pos+15,pos2-15);
               getExpired=modExpires;
            }
            int ret=timeCompare(modTime,lmTime);
            return ret;
         }
      }
   }
}

struct pageResp getPage(const char* request,const char* url, char* servPort)
{
  char largeBuf[MAX_CONTENT_BUF_SIZE];
  string buff;
  string tempStr;
  int servFD;
  struct pageResp a;
  int connectVal;

  struct addrinfo tempp, *ress;
  memset(&tempp, 0, sizeof(tempp));
  tempp.ai_family = AF_INET;
  tempp.ai_socktype = SOCKET_TYPE;
  if((connectVal = getaddrinfo(url,servPort, &tempp, &ress)) != 0)
  {
     cout << "Bad Request. Unable to get server address: "<<url<<endl;
  }
  else
  {
     cout << "Connecting to server: " <<url<<endl;
     servFD = socket(ress->ai_family, ress->ai_socktype, ress->ai_protocol);
     if(servFD == -1)
     {
        perror("Error while creating socket for connecting to server");
     }
     else
     {
        if(connect(servFD, ress->ai_addr,ress->ai_addrlen)==-1)
        {
           perror("connect() error while connecting to server");
           close(servFD);
        }
        else
        {
            cout << "Connection to server established!" << endl;
            freeaddrinfo(ress);

            cout << "Sending HTTP GET message:" << endl << request << endl;
            if(send(servFD, request, strlen(request), 0) == -1)
            {
              cout << "Could not send GET" << endl;
              close(servFD);
            }
            while(1)
            {
               memset(&largeBuf, 0, sizeof(largeBuf));
               int recvbytes = recv(servFD, largeBuf, MAX_CONTENT_BUF_SIZE, 0);
               if(recvbytes <= 0)
               {
                  break;
               }
               else
               {
                 buff=buff.append(string(largeBuf));
                 memset(largeBuf, 0, recvbytes);
               }
             }
             string tempbuf;
             size_t pos2;
             a.msg=buff;
             size_t pos=buff.find("\r\n");
             a.status=buff.substr(0,pos);
             buff=buff.substr(pos+2);
             pos = buff.find("\r\n\r\n");
             a.header=buff.substr(0,pos);
             a.htmlContent=buff.substr(pos+4);
             buff=a.header;
             if ((pos=buff.find("Expires: "))!=string::npos)
             {
                a.foundExpired=true;
                tempbuf=buff.substr(pos);
                pos2=tempbuf.find("\r\n");
                a.expires=buff.substr(pos+9,pos2-9);
                if((a.expires).find("GMT")==string::npos)
                {
                  a.expires=currTime();
                }
             }
             else
                a.foundExpired=false;
             if((pos=buff.find("Last-Modified: "))!=string::npos)
             {
               a.foundLM=true;
               tempbuf=buff.substr(pos);
               pos2=tempbuf.find("\r\n");
               a.lastModified=buff.substr(pos+15,pos2-15);
             }
             else
               a.foundLM=false;
        }
     }
   }
   return a;
}

int main(int argc, char* argv[])
{
  int serverFD, maxFD;
  fd_set readFD,master;
  struct addrinfo temp, *res;
  int connectVal;
  char msgBuf[BUF_SIZE];
  char buff[MAX_CONTENT_BUF_SIZE];
  char largeBuf[MAX_CONTENT_BUF_SIZE];
  string uri;
  struct sockaddr_storage clientAddr;
  socklen_t clientAddrSize = sizeof(clientAddr);
  struct timeval tv;
  tv.tv_sec = 300;
  tv.tv_usec = 0; // 5 min timeout
  string tempStr,sUri;
  const char* request;
  char errorBuf[1024];

  if(argc != 3)
  {
    cout << "Usage: ./client serverIP serverPort" << endl;
    return 1;
  }

  const char* ipaddr = argv[1];
  const char* port = argv[2];

  memset(&temp, 0, sizeof(temp));
  temp.ai_family = AF_UNSPEC;
  temp.ai_socktype = SOCKET_TYPE;
  if((connectVal = getaddrinfo(ipaddr, port, &temp, &res)) != 0)
  {
    cout << "getaddrinfo error";
    exit(1);
  }

  cout << "Starting the server..." << endl;
  cout << "IP Address: " << ipaddr << endl;
  cout << "Port Number: " << port << endl;

  serverFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if(serverFD == -1)
  {
    perror("Error in creating Server Socket");
    exit(1);
  }
  int yes = 1;
  if(setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
  {
    perror("setsockopt() error: ");
    exit(1);
  }

  if(bind(serverFD, res->ai_addr, res->ai_addrlen) == -1)
  {
    perror("bind() error: ");
    close(serverFD);
    exit(1);
  }

  freeaddrinfo(res);

  if(listen(serverFD, 10) == -1)
  {
    perror("listen() error:");
    close(serverFD);
    exit(1);
  }

  cout << "Server is now listening!" << endl;

//initialize FD sets for select()

  FD_ZERO(&readFD);
  FD_ZERO(&master);
  FD_SET(serverFD, &master);
  maxFD = serverFD;

  while(1)
  {
    readFD = master;
    connectVal = select(maxFD+1, &readFD, NULL, NULL, &tv);
    if(connectVal = 0)
    {
      perror("select() timeout error ");
    }
    if(connectVal < 0)
    {
      perror("select() error ");
    }

    for(int i = 0; i <= maxFD; ++i)
    {
      if(FD_ISSET(i, &readFD))
      {
        if(i==serverFD)
        {
          int clientFD = accept(serverFD, (struct sockaddr *)&clientAddr, &clientAddrSize);
          if(clientFD == -1)
          {
            perror("Server couldnot accept connection from a Client");
            close(clientFD);
          }
          cout << "Accepted connection from a client!" << endl;
          FD_SET(clientFD, &master);
          if(clientFD > maxFD)
            maxFD = clientFD;
        }
        else
        {
          memset(&msgBuf, 0, BUF_SIZE);
          int recvbytes = recv(i, msgBuf, BUF_SIZE, 0);
          if(recvbytes <= 0)
          {
            if(recvbytes == 0)
            {
              cout<<"select server: socket hung up"<<endl;
            }
            else
            {
              perror("recv error");
            }
            close(i);
            FD_CLR(i, &master);
          }
          else
          {
            cout<<"Recvd msg: "<<msgBuf<<endl;
            tempStr=string(msgBuf);
            size_t fir=tempStr.find("http");
            string temp2=tempStr.substr(fir);
            string temp3=tempStr.substr(fir+7);
            size_t sec=temp3.find("/");
            size_t spc=temp3.find(" ");
            string sUri=temp3.substr(0,sec);
            string reqst=temp3.substr(sec,spc-sUri.size());
            reqst="http://"+sUri+reqst;
            const char* url=sUri.c_str();
            char servPort[2];
            int prt=80;
            sprintf(servPort,"%d",prt);

            cout<<"Server: "<<sUri<<endl;
            cout<<"request: "<<reqst<<endl;

            string tempReq="HEAD "+reqst+" HTTP/1.0\r\n\r\n";
            request=tempReq.c_str();

            bool foundflag=false;
            struct pageResp page;

            for( map<string,struct pageResp>::iterator it=Cache.begin(); it!=Cache.end(); ++it)
            {
               string getExpired;
               if (reqst.compare(it->first)==0)
               {
                  foundflag=true;
                  string currenttime=currTime();
                  cout<<"current time: "<<currenttime<<endl;
                  if((it->second).foundExpired==true)
                  {
                     if(timeCompare((it->second).expires,currenttime)!=1)
                     {
                        int compModTime=compareHead((it->second).lastModified,request,url,servPort,getExpired);
                        if(compModTime!=0)
                        {
                           page = getPage(msgBuf,url,servPort);
                           if((page.status).find("200")!=string::npos)
                               addToCache(reqst,page);
                           cout<<"Getting page from server: Expired and Modified"<<endl;
                           break;
                         }
                         else
                         {
                           page = it->second;
                           (it->second).lastUsed=currTime();
                           (it->second).expires=getExpired;
                           cout<<"getting page from cache:Expired but Not Modified"<<endl;
                           break;
                         }
                      }
                      else
                      {
                        page = it->second;
                        (it->second).lastUsed=currTime();
                        cout<<"getting page from cache: Not expired"<<endl;
                        break;
                      }
                    }else if((it->second).foundLM==true)
                    {
                       int compModTime=compareHead((it->second).lastModified, request,url,servPort,getExpired);
                       if(compModTime!=0)
                       {
                           page = getPage(msgBuf,url,servPort);
                           if((page.status).find("200")!=string::npos)
                               addToCache(reqst,page);
                           cout<<"Getting Page from Server: No expired field.Page changed at server side"<<endl;
                           break;
                        }
                        else
                        {
                           time_t rawtime;
                           struct tm * ptm;
                           time ( &rawtime );
                           ptm = gmtime ( &rawtime );
                           ptm->tm_sec += 10;
                           char timebuff [80];
                           strftime (timebuff,80,"%a, %d %b %Y %H:%M:%S %Z",ptm);
                           (it->second).expires=string(timebuff);
                           page = it->second;
                           (it->second).lastUsed=currTime();
                           cout<<"getting page from cache: No expired field. Not modified at server"<<endl;
                           break;
                         }
                      }
                      else
                      {
                         page = getPage(msgBuf,url,servPort);
                         if((page.status).find("200")!=string::npos)
                           addToCache(reqst,page);
                         cout<<"No expired or Modified field found. Get from Server itself"<<endl;
                         break;
                      }
                  }
                }
                if(foundflag==false)
                {
                    page = getPage(msgBuf,url,servPort);
                    if((page.status).find("200")!=string::npos)
                      addToCache(reqst,page);
                    cout<<"Page not in cache. Getting from Server"<<endl;
                }

                memset(&largeBuf, 0, MAX_CONTENT_BUF_SIZE);
                if((page.status).find("200")!=string::npos)
                {
                    char* ch1=(char*)page.htmlContent.c_str();
                    memcpy(largeBuf,ch1,page.htmlContent.length());
                    if(send(i,largeBuf,strlen(largeBuf), 0) == -1)
                    {
                      cout << "Unable to send the HTML page requested to the client" << endl;
                    }
                }
                else
                {
                    char* ch2=(char*)(page.status).c_str();
                    memcpy(largeBuf,ch2,page.status.length());
                    if(send(i,largeBuf,strlen(largeBuf), 0) == -1)
                    {
                      cout << "Unable to send Retrieval Error message to the client"<< endl;
                    }
                 }
                 foundflag=false;
                 cout << "Closing connection with the client" << endl;
                 close(i);
                 FD_CLR(i, &master);
              }
          }     //close of else construct that handles data received from client
        }      //end of if construct that checks if i is in fdset
      }     //end of for loop that iterates through the fds
    }        // close of while loop

  close(serverFD);
  return 0;
}                                                                                                                                                                                            
