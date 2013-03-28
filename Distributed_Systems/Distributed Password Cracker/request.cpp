#include "lsp.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <openssl/sha.h>
#define MAX 4096

int main(int argc, char** argv) 
{
   /*Initialize the variables*/
   srand(12345);
   int port = 2222;
   char * host = "127.0.0.1";
   char hash[40];   
   memset(hash, 0, 40);      
   unsigned char requestbuf[1000];   
   memset(requestbuf, 0, 1000);
   char  buffer[1000];
   memset(buffer, 0, 1000);
   int msglen=0,i=0;

   /*Check for correct argument types*/
   if(argc != 4)
   { 
      cout<<"Wrong number of arguments. Enter host:port hash len"<<endl;
      return -1;
   }

   memcpy(hash,argv[2], strlen(argv[2]));
   msglen = atoi(argv[3]);                             
   host = strtok(argv[1], ":");
   if(host != NULL)
   {
     port = atoi(strtok(NULL, " ")); 
   }
   else
   {
     printf("The correct format is host:port\n");
     return -1;   
   }

   if(msglen<=0)
   {
     printf("Password length must be greater than 0\n");
     return -1;   
   }

   /*Generate upper and lower ranges as aa..a and zz.z depending on length of string*/
   char lower[msglen+1];
   char upper[msglen+1];
   for(int i = 0; i < msglen; i++)
   {
       lower[i] = 'a';
       upper[i] = 'z';
   }  
   lower[msglen]='\0';
   upper[msglen]='\0';

   /*Create a lsp request client*/
   lsp_client* request =  (lsp_client*)malloc( sizeof(lsp_client) );
   request = lsp_client_create(host, port);

    /*Send crack request to the server*/
    sprintf((char *) requestbuf,"%s%s%s%s%s%s%s%s%d%s","c", ":",(const char *)hash,":",(const char *)lower,":",(const char *)upper,":",msglen,":");        
    printf("REQUEST MSG: %s\n", requestbuf);      
    lsp_client_write(request, (uint8_t *) requestbuf, strlen((const char *)requestbuf));
      
    /*Wait for reply from server, break if reply received or server disconnected*/  
    while(1)
    {
      sleep(0.3);
      int bytes_read = lsp_client_read(request, (uint8_t *) buffer);
      /*Case:Server Disconnected*/     
//      cout<<"bytes_read: "<<bytes_read<<endl;
      if(bytes_read < 0)
      {
         cout<<"Disconnected"<<endl;
         lsp_client_close(request);
           exit(0);
      }

     /*Message Received*/
      else if (bytes_read>0)
      {
        cout<<buffer<<endl;
        char first = buffer[0];
       switch (first)
       {
         /*Password Found*/
         case 'f':
         {
	   strtok(buffer, ":");
           char * password = strtok(NULL, "\n");
           cout<<"Found: "<<password<<endl;
           break;
         }
         /*Password Not Found*/
         case 'x':
         {
           cout<<"Not Found"<<endl;
           break;
         }
       }
      }

  }        
    return 0;
}

