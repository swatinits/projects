/*
Linux Shell Prompt
Author: 
Swati(06-1-5-066)
Kirti Reddy(06-1-5-055) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#define STACK 1024*64

int shellprjct() ;
void buffer(char *tmp);
void freebuffer(char *buff[]);
int addpath(char *command);
void run(char *command);
char *buffer1[5],*buffer2[2],*path[2],*env[100],*args[100];
int flag=0;

int main(int argc, char *argv[], char *envp[])
{	
	void *child_stack;
   	child_stack = malloc( STACK );
   	pid_t pid;
	int i = 0;
	for(;envp[i] != NULL; i++) 
	{
		env[i] = (char *)malloc(sizeof(char) * (strlen(envp[i]) + 1));
		memcpy(env[i], envp[i], strlen(envp[i]));
	}
	i=0;
	for(;argv[i] != NULL; i++) 
	{
		args[i] = (char *)malloc(sizeof(char) * (strlen(argv[i]) + 1));
		memcpy(args[i], argv[i], strlen(argv[i]));
	}
	flag=0;
	pid = clone(&shellprjct, (void *) child_stack + STACK, CLONE_VM|CLONE_FILES, NULL);
// The parent process will sleep till child returns
	while(flag==0)				
		sleep(1);                                            
         printf( "Child thread returned and stack freed.\n" );
         free( child_stack );
	 return 0;
}
int shellprjct() 
{
	char c;
	char *tmp = (char *)malloc(sizeof(char) * 50);
	char *command = (char *)malloc(sizeof(char) * 10);
	char *input= (char *)malloc(sizeof(char) * 50);
	int i=0;

	path[0] = (char *)malloc(sizeof(char) * 10);
	path[1] = (char *)malloc(sizeof(char) * 10);
	strncpy(path[0],"/bin/",5);
	strncat(path[0],"\0",1);
	strncpy(path[1],"/usr/bin/",9);
	strncat(path[0],"\0",1);
	i=0;

	if(fork() == 0) 
	{
		execve("/usr/bin/clear", args, env);
		exit(1);
	} 
	else 
	{
		wait(NULL);
	}
	printf("prompt] ");
	fflush(stdout);
	while(c!=EOF)	
	{   fgets(input,49,stdin);
	    c=input[i];
	    if(c == '\n' && tmp[0] == '\0') 
	    	printf("prompt] ");
	    else
	   {	while(c!='\n')
	    	{	strncat(tmp, &c, 1);
			i++;
			c=input[i];
		}
		tmp[i]='\0';
		if(strcmp(tmp,"exit")==0)		           // exit command
		{	printf("\n back to default shell \n");
			flag=1;
			return 0;
		}					
		buffer(tmp);
		strncpy(command, buffer1[0], strlen(buffer1[0]));
		strncat(command, "\0", 1);
		if(addpath(command) == 0) 
			   run(command);
		else 
		{
		   printf("%s: command not found\n", command);
		}
	    	freebuffer(buffer1);  
		i=0;
	   	printf("prompt] ");
	   	bzero(tmp,50);
		bzero(input,50);
	   	bzero(command,10);
	   }
	}
	flag=1;
	return 0;
}


//The command is stored in a buffer

void buffer(char *tmp)			 
{
	char *str = tmp;
	int i = 0;
	char r[20];
	bzero(r,20);
	while(*str != '\0') 
	{
		if(i == 5)
			break;

		if(*str == ' ') 
		{
			if(buffer1[i] == NULL)
				buffer1[i] = (char *)malloc(sizeof(char) * strlen(r) + 1);
			else 
			{
				bzero(buffer1[i], strlen(buffer1[i]));
			}
			strncpy(buffer1[i], r, strlen(r));
			strncat(buffer1[i], "\0", 1);
			bzero(r, 20);
			i++;
		} 
		else 
		{
			strncat(r, str, 1);
		}
		str++;
        }
	buffer1[i] = (char *)malloc(sizeof(char) * strlen(r) + 1);
	strncpy(buffer1[i], r, strlen(r));
	strncat(buffer1[i], "\0", 1);
}

//The path is appended by the function addpath()

int addpath(char *command)				
{	
	char temp[10];
	buffer2[0] = (char *)malloc(sizeof(char) * 15);
	buffer2[1] = (char *)malloc(sizeof(char) * 15);
	
	strcpy(temp,path[0]);
	strncat(temp,command,strlen(command));
	strcpy(buffer2[0],temp);	
	bzero(temp,10);
	
	strcpy(temp,path[1]);
	strncat(temp,command,strlen(command));
	strcpy(buffer2[1],temp);	
	bzero(temp,10);
	return 0;
}

//function to run the command

void run(char *command)				
{
	int i;
	if(fork() == 0) 
	{	i = execve(buffer2[0],buffer1,env);
		if(i < 0) 
			i = execve(buffer2[1],buffer1,env);
		if(i < 0) 
		{
			printf("%s: %s\n", command, "command not found");
			exit(1);		
		}
	} else 
	{
		wait(NULL);
	}
}

//Function to free the buffer

void freebuffer(char *buffer1[])						
{
	int i;
	for(i=0;buffer1[i]!=NULL;i++) 
	{
		bzero(buffer1[i], strlen(buffer1[i])+1);
		buffer1[i] = NULL;
		free(buffer1[i]);
	}
}

