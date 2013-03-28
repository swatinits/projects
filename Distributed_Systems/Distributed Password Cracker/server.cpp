
#include "lsp.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <list>
#include <string.h>
using namespace std;
class Task;
class Worker;
class Request;
/*Global Lists*/
list<Task> tasks_unassigned;
list<Task> tasks_assigned;
list<Worker> workers_free;
list<Worker> workers_busy;
list<Request> requests;
/*Mutex variables*/
pthread_mutex_t taskas;
pthread_mutex_t taskun;
pthread_mutex_t workerfr;
pthread_mutex_t workerbs;
pthread_mutex_t reqst;
pthread_mutex_t cntr;
static int id = 0;

/*Class that encapsulates each unique request */
class Request
{
public:
  uint32_t reqid;
  int numjobs;
  int counter;
  int found;
  int connected;

  Request(uint32_t id,int len)
  {
    reqid=id;
    counter=0;
    connected=1;
    found=0;
    if(len>0 && len<4)
      numjobs=1;
    else if(len==4)
      numjobs=3;
    else if(len==5)
      numjobs=6;
    else if(len>5)
      numjobs=26;   
  }
};

/*Class that encapsulates the worker */
class Worker
{
public:
  uint32_t connid;
  int taskid;

  Worker(uint32_t id)
  {
    connid=id;
  }

  void setTaskID(int id)
  {
    taskid=id;
    return;
  }

};

/*Class that encapsulates a job/task for a request */
class Task 
{
public:
  string upper;
  string lower;
  string hash;
  int len;    
  uint32_t reqclient;          //conn id of the request client of the password/task
  uint32_t worker;             //stores a list of connection ids of clients assigned to the task
  int taskid;
    
  Task(string low, string upp, string hashstr, int length) 
  {
    pthread_mutex_lock (&cntr);	
    id++;
    pthread_mutex_unlock (&cntr);
    upper = upp;		
    lower = low;
    hash = hashstr;
    len = length;      
    taskid=id;	
  }

  void setReqClient(uint32_t connid)
  {
    reqclient=connid;
    return;
  }

  void setWorker(uint32_t id)
  {
    worker=id;
    return;
  }

};

/*Function that listens for join request from worker client, crack requests from request clients, and replies from the worker and processes them*/
void *listener(void* server)
{
    sleep(0.3);
    lsp_server* serv =  (lsp_server*)malloc( sizeof(lsp_server) );
    serv = (lsp_server*)server; 
    uint32_t connid;
    char buffer[1500];
    while(1)
    {
    memset(buffer, 0, 1500);
    /*Read for messages from clients*/
    int rcvbytes = lsp_server_read(serv, (uint8_t *) buffer, &connid);		
    /*Case: Client Disconnected*/
    if(rcvbytes < 0) 
    {
      cout<<"Client "<<connid<<" Disconnected"<<endl;
      pthread_mutex_lock (&taskun);
      pthread_mutex_lock (&reqst);
      /*Check if a request client is disconnected. If true, then set connected flag to false, and remove the tasks for the request from the list of unassigned tasks*/
      for(list<Request>::iterator req = requests.begin(); req != requests.end();  req++)		 
      {
         if(req->reqid==connid)                    
         {
           req->connected=0;
           for(list<Task>::iterator tsk = tasks_unassigned.begin(); tsk != tasks_unassigned.end();)
           {
             if(tsk->reqclient==connid)
             {
               tsk=tasks_unassigned.erase(tsk);
               req->counter++;
             }
             else
               tsk++;
           }
           break;
         }
      }
      pthread_mutex_unlock (&reqst);
      pthread_mutex_unlock (&taskun);

      /*If free worker is disconnected, just remove from the list*/
      pthread_mutex_lock (&workerfr);
      for(list<Worker>::iterator wrk = workers_free.begin(); wrk != workers_free.end();  wrk++)		 
      {
         if(wrk->connid==connid)                    
         {
            cout<<" Free Worker "<<connid<<"Disconnected"<<endl;
            wrk = workers_free.erase(wrk);
            break;
         }
      }
      pthread_mutex_unlock (&workerfr);
      pthread_mutex_lock (&taskas);
      pthread_mutex_lock (&workerbs);     

      /*If busy worker is disconnected, remove from list and reassign the task to some free worker.*/
      for(list<Worker>::iterator wrk = workers_busy.begin(); wrk != workers_busy.end();  wrk++)		 
      {           
         if(wrk->connid==connid)                    
         {
           pthread_mutex_lock(&taskun);          
           /*Find task whose task id matches the task id ofthe task the disconnected woker was working on*/
           for(list<Task>::iterator tsk = tasks_assigned.begin(); tsk != tasks_assigned.end();  tsk++)		 
           {
             if(tsk->worker==connid)
             {
               cout<<" Busy Worker "<<connid<<"Disconnected"<<endl;
               pthread_mutex_lock(&reqst);
               for(list<Request>::iterator req = requests.begin(); req != requests.end();  req++)
               {		 
                 if(req->reqid==tsk->reqclient)
                 {  
                   /*If password is not yet found and request client still connected, reassign the task, else remove the task from assigned task list */
                   if(req->found !=1 && req->connected==1)
                   {
                     Task retask(tsk->lower,tsk->upper,tsk->hash,tsk->len);
                     retask.setReqClient(tsk->reqclient);
                     tasks_unassigned.push_front(retask);
                   }
                   tsk = tasks_assigned.erase(tsk);  
                   break;
                 }
               }
               pthread_mutex_unlock(&reqst);
               break;
             }
           }                    
           pthread_mutex_unlock(&taskun);
           wrk = workers_busy.erase(wrk);
           break;
         }
      }
       pthread_mutex_unlock (&workerbs);
       pthread_mutex_unlock (&taskas); 
    }
    /*Case: Message received from some client*/
    if(rcvbytes > 0) 
    {
      printf("Read bytes: %s\n", buffer);
      char first = buffer[0];
      switch (first)
      {
         /*Join request from worker, add worker to list of free workers*/ 
         case 'j':
         {
           pthread_mutex_lock (&workerfr);
           Worker worker1(connid);
	   workers_free.push_back(worker1);
	   printf("Worker %d joined.\n", worker1.connid);
           pthread_mutex_unlock (&workerfr);
         }
         break;

         /*Crack request from worker*/ 
         case 'c':
         {
           static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
           strtok(buffer, ":");
	   char * hash = strtok(NULL, ":");
	   char * lower = strtok(NULL, ":");
           char * upper = strtok(NULL, ":");
           int len=atoi(strtok(NULL, ":"));
           Request req(connid,len);
           pthread_mutex_lock (&reqst);
           requests.push_back(req);
           pthread_mutex_unlock (&reqst);
           /*Dont break into subtasks if password length is less than 4. Add the entire task to unassigned task list*/
           if(len>0 && len<4)
           {
             pthread_mutex_lock(&taskun);            
             Task task1(lower, upper, hash, len);
             task1.setReqClient(connid);
             tasks_unassigned.push_back(task1);    	    
             pthread_mutex_unlock(&taskun);
             break;
           }
           /*Break into tasks: number of tasks = 3 for length 3, number = 6 for length 5, and number = 26 for length>6*/
           /*Assign upper and lower range for every task and add the task to unassigned task list*/
           else if(len>3)
           {
             int start=0;
             int end=0;   
             int num=0;
             if(len==4)
               num=3;
             else if(len==5)
               num=6;
             else if (len>5)
               num=26;
             int inc=26/num; 
             for(int j=0;j<num;j++)
             {
               if(j!=num-1)
                 end=start+inc-1;
               else
                 end=25;
               lower[0] = alphabet[start];
               upper[0] =  alphabet[end];
               start=start+inc;
               pthread_mutex_lock(&taskun);
               Task task1(lower, upper, hash, len);
               task1.setReqClient(connid);
               tasks_unassigned.push_back(task1);
               pthread_mutex_unlock(&taskun);
             }
             break;
           }
             pthread_mutex_lock(&taskun);
	   printf("Request from client %d.\n", connid);            
             pthread_mutex_unlock(&taskun);
         }
         break;

         /*If worker sends a password found message, send the password to the client, remove the unassigned tasks for the same request, set password found flag for the requst*/
         case 'f':
         {
            uint32_t requestid;
            pthread_mutex_lock (&taskas);
            for(list<Task>::iterator task = tasks_assigned.begin(); task != tasks_assigned.end();  task++)
            {
              if (task->worker==connid)
              {
                requestid=task->reqclient;
                //Send password to request client
                pthread_mutex_lock (&reqst);
                for(list<Request>::iterator rq = requests.begin(); rq != requests.end();  rq++)
                {
                  if(rq->reqid==task->reqclient && rq->connected==1)
                  {
   		    lsp_server_write(serv, (uint8_t *) buffer, rcvbytes, task->reqclient);
                    rq->counter=rq->numjobs;
                    rq->found=1;
                    break;
                  }
                }
                pthread_mutex_unlock (&reqst);
		strtok(buffer, ":");
		char * password = strtok(NULL, "\n");
		printf("Found password: %s\n", password);

                /*Add worker with connid to the list of free workers, remove from busy workers list*/
                pthread_mutex_lock (&workerbs);
                pthread_mutex_lock (&workerfr);    
                for(list<Worker>::iterator worker = workers_busy.begin(); worker != workers_busy.end();)		 
                {
                  if(worker->taskid==task->taskid)      
                  {                   
                    Worker wrkr(connid);
      		    workers_free.push_back(wrkr);
                    worker = workers_busy.erase(worker);
                  }
                  else
                    worker++;
                }
                pthread_mutex_unlock (&workerfr);
                pthread_mutex_unlock (&workerbs);
                task=tasks_assigned.erase(task);
                break;
              }           
            }
            pthread_mutex_unlock (&taskas);
   
            pthread_mutex_lock(&taskun);
            list<Task>::iterator task = tasks_unassigned.begin();
            while( task != tasks_unassigned.end())	
            {
              if (task->reqclient==requestid)
              {
                task=tasks_unassigned.erase(task);
              } 
              else
                task++;            
            }
            pthread_mutex_unlock(&taskun);

/*            pthread_mutex_lock (&reqst);
            for(list<Request>::iterator req = requests.begin(); req != requests.end();  req++)		 
            {
              if (req->reqid==requestid)
              {
                req->counter=req->numjobs;
                req->found=1;
                break;
              }                          
            }
            pthread_mutex_unlock (&reqst);*/
         }
         break;

         /*Password not found by worker*/
         case 'x':
         {
            pthread_mutex_lock (&taskas);
            for(list<Task>::iterator task = tasks_assigned.begin(); task != tasks_assigned.end();  task++)		 
            {
              if (task->worker==connid)
              {
                pthread_mutex_lock (&workerbs);
                /*Add worker with connid to the list of free workers, remove from busy workers list*/
                for(list<Worker>::iterator worker1 = workers_busy.begin(); worker1 != workers_busy.end();  worker1++)		 
                {
                  if(worker1->taskid==task->taskid)      
                  {
                   pthread_mutex_lock (&workerfr);
                    Worker wrkr(connid);
     		    workers_free.push_back(wrkr);
                   pthread_mutex_unlock (&workerfr);
                    worker1 = workers_busy.erase(worker1);
                    break;
                  }
                }
                pthread_mutex_unlock (&workerbs);
                /*Increment the Finished jobs counter of request class*/
                pthread_mutex_lock (&reqst);
                for(list<Request>::iterator req = requests.begin(); req != requests.end();  req++)		 
                  if(req->reqid==task->reqclient)
                    req->counter=req->counter+1;
                /*Remove task from list of assigned tasks*/
                pthread_mutex_unlock (&reqst);            
                task = tasks_assigned.erase(task); 
                break;
              }		
            }           
            pthread_mutex_unlock (&taskas);
         }
         break;

         default:
         {
           printf("Invalid message format from client\n");
         }
         break;
      }
    }
    }
    pthread_exit(NULL);
}

/*Function that assigns tasks rom the unassigned task list to available free workers in the free workers list*/
void *scheduler(void* server)
{

    lsp_server* serv =  (lsp_server*)malloc( sizeof(lsp_server) );
    serv = (lsp_server*)server; 
    char taskmsg[1000];
    while(1)
    {
    sleep(0.3);
    memset(taskmsg, 0, 1000);  
    /*Acquire locks*/
    pthread_mutex_lock (&taskas);
    pthread_mutex_lock (&workerbs);
    pthread_mutex_lock(&taskun); 

    /*Loop through the unassigned task list and free worker list while both are not empty. Assign each task to a free worker. Move the task to Assigned task list and the worker to the busy worker list*/
    while (!tasks_unassigned.empty() && !workers_free.empty())
    { 
       Task task1 = tasks_unassigned.front();
       cout<<"assigning task "<<task1.taskid<<endl;
       pthread_mutex_lock (&workerfr);
       Worker worker1=workers_free.front();   
       memset(taskmsg, 0, 1000);
       sprintf((char *) taskmsg,"%s%s%s%s%s%s%s%s%d%s","c", ":",(const char *)task1.hash.c_str(),":",(const char *)task1.lower.c_str(),":",(const char *)task1.upper.c_str(),":",task1.len,":");        
       worker1.taskid=task1.taskid;
       task1.worker=worker1.connid;
       lsp_server_write(serv, (uint8_t *)taskmsg, strlen((const char *)taskmsg), worker1.connid);       
       workers_free.pop_front();
       pthread_mutex_unlock (&workerfr);
       workers_busy.push_back(worker1);
       tasks_assigned.push_back(task1);
       tasks_unassigned.pop_front();         
    }

       /*Release Locks*/
       pthread_mutex_unlock(&taskun); 
       pthread_mutex_unlock (&workerbs);
       pthread_mutex_unlock (&taskas);
    }
    pthread_exit(NULL);
}

/*Function that checks for requests in Request list if they are already completed. If completed, remove them from the list*/
void *removeCompletedRequests(void* server)
{
  sleep(0.3);
  lsp_server* serv =  (lsp_server*)malloc( sizeof(lsp_server) );
  serv = (lsp_server*)server; 

  while(1)
  {  
  /*Loop through every element in the request list*/
  /*Acquire lock on the request list*/
  pthread_mutex_lock (&reqst);
  for(list<Request>::iterator req = requests.begin(); req != requests.end();)		 
  { 
    /*If every task for the request is completed or if the password was found, delete the request from the list*/  
    if(req->counter>=req->numjobs)    
    {
      /*If password was found, do nothing and delete the request*/
      if(req->found==1)
      {
        req = requests.erase(req); 
      }
      /*If password was not found and request client is still connected, send Not found message to client and remove the request*/
      else if(req->connected==1 && req->found==0)
      {
        lsp_server_write(serv, (uint8_t *)"x", strlen((const char *)"x"), req->reqid);       
        cout<<"Password not found for request client: "<<req->reqid<<endl;
        req = requests.erase(req); 
      }
      /*If password was not found and request client is not connected, send Not found message to client and remove the request*/
      else
        req++;
    }
    /*If every task for the request is not completed, iterate to next request*/
    else
      req++;
  }
  /*Release lock on the request list*/
  pthread_mutex_unlock (&reqst);  
  }
  pthread_exit(NULL);
}
		
int main(int argc, char ** argv) 
{
   /*Initialization*/
   srand(12345);
   int nt[3];
   pthread_t t_id[3];
   int port = atoi(argv[1]);

   if(argc != 2)
   { 
     cout<<"Wrong number of arguments. Format: ./server port "<<endl;
     return -1;
   }
    /*Create a server*/
    lsp_server* serv =  (lsp_server*)malloc( sizeof(lsp_server) );
    serv=lsp_server_create(port);
  
    /*Start the listener thread*/
    nt[0] = pthread_create(&t_id[0], NULL, listener, (void*) &serv);
    if(nt[0])
    {
      printf("\n ERROR: Unable to start listener thread %d \n", nt[0]);
      exit(1);
    }    

    /*Start the scheduler thread*/
    nt[1] = pthread_create(&t_id[1], NULL, scheduler, (void*) &serv);
    if(nt[1])
    {
      printf("\n ERROR: Unable to start scheduler thread %d \n", nt[1]);
      exit(1);
    }    

    /*Start the helper thread*/
    nt[2] = pthread_create(&t_id[2], NULL, removeCompletedRequests, (void*) &serv);
    if(nt[2])
    {
      printf("\n ERROR: Unable to start helper thread %d \n", nt[2]);
      exit(1);
    }    

    pthread_exit(NULL); 
}

