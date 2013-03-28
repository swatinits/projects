#include "lsp.h"
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <openssl/sha.h>
#define MAXTHREADS	20
using namespace std;

int main(int argc, char** argv) 
{
   srand(12345);
   int port = 2222;
   char * host = "127.0.0.1";
   char hash[40];         
   memset(hash, 0, 40);
   unsigned char joinbuf[4];   
   memset(joinbuf, 0, 4);
   char buff[1000];
   int msglen=0,i=0;
   uint8_t answer[100];
   memset(answer, 0, 100);

   /*Check if the arguments and their format is correct*/
   if(argc != 2)
   { 
      cout<<"Wrong number of arguments. Format: ./worker host:port "<<endl;
      return -1;
   }

   /*Assign ip address and port from the command line argument. If format is not host:port retrun error*/
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

   /*Create a lsp worker client*/
   lsp_client* worker =  (lsp_client*)malloc( sizeof(lsp_client) );
   worker = lsp_client_create(host, port);

   /*Send crack request to the server*/
   memset(joinbuf, 0, 4);
   sprintf((char *) joinbuf,"%s","j");        
   printf("JOIN MSG: %s\n", joinbuf);
   lsp_client_write(worker, (uint8_t *) joinbuf, strlen((const char *)joinbuf));
   printf("Keep receiving request from server\n");
   int len=0;

   /*Wait in a infinite loop for message from server. Break out of loop when message received or server disconnected */
   while(1)
   {     
      sleep(0.3); 
      memset(buff,0,1000);
      int bytes_read = lsp_client_read(worker, (uint8_t *) buff);

      /*Case: Server Disconnected*/
      if(bytes_read<0)
      {
         lsp_client_close(worker);
         cout<<"Server Disconnected. Shutting Client"<<endl;
	   exit(0);
      }
      /*Case:Message received from Server and first character is c, then request to break a password*/
      if(bytes_read>0 && buff[0]=='c')
      {
        cout<<"Received crack job"<<endl;
        cout<<buff<<endl;      

        /*Tokenize the message recieved to extract hash string, lower and upper range and the length of password*/
        strtok(buff, ":");
        char * hash = strtok(NULL, ":");
	char * lower = strtok(NULL, ":");
        char * upper = strtok(NULL, ":");
        len=atoi(strtok(NULL, ":"));
        uint8_t last[len];

        /*Generate last string with all z's to mark the corner case and avoid relooping from zzz..z to aaa..a*/
        for(int i=0; i<len;i++)
        {
          last[i]='z';
        }
        last[len]='\0';

        /*Flag to track if password is found or not*/
        bool passwd=false; 
        
        unsigned char nhash[20];
        /*Data Structure used to convert 20 byte hash to 40 byte char string to ease comparison using string compare function*/
        static const char alphabet[] = "0123456789abcdef";

        while(strcmp((const char *)lower, (const char *)upper) <= 0 && passwd == false)
        {        
          int change=0;
          int curr;
          SHA1((const unsigned char*)lower, strlen((const char *)lower), nhash);
          char hash_str[40];

          /*Convert 20 byte hash to 40 byte char string*/
          for (int k = 0; k < 20; ++k)
          {
            hash_str[2*k]     = alphabet[nhash[k] / 16];
            hash_str[2*k + 1] = alphabet[nhash[k] % 16];
          }
          hash_str[40]='\0';
//          cout<<lower<<endl;

          /*Compare the two hash strings, If equal, set password found flag to true, send the password to server and break*/
          if(strcmp((const char *)hash, (const char *)hash_str) == 0)
          {
            passwd = true;
            cout<<"Password Found: "<<lower<<" and hash: "<<hash_str<<endl;
            memset(answer, 0, 100);
            sprintf((char *) answer,"%s%s%s","f", ":",(const char *)lower);        
            lsp_client_write(worker, (uint8_t *) answer, strlen((const char *)answer));
            break;
          }
          /*If password not found, check for next string in the lower-upper range*/
          else
          {
            /*If the current string consisted of all z's, break as last elemnt in the entire range*/
            if(strcmp((const char *)lower, (const char *)last) == 0)
            {
              break;
            }
            /*Find the next string in the range, for all the last characters that were equal to z, reset them to a and increment the previous character by 1: aaz->aba, aazz->abaa*/
            for(curr = strlen((const char *)lower)-1; curr >= 0 && change !=1; curr--)
            {
              if(lower[curr] < 'z')
              {
                lower[curr]++;
                change = 1;
              }
              else lower[curr] = 'a';
            }
          }                        
           
        }
        /*Hash strings of none of the words matched, password not found, send message to server*/
        if(passwd == false)
        {
          cout<<"Password not found "<<" upper: "<<upper<<" lower: "<<lower<<" Hash: "<<hash<<endl;
          lsp_client_write(worker, (uint8_t *) "x", strlen((const char *)"x"));
        }
      }      
   }
   return 0;
}
