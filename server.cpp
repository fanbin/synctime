#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include "sync.h"
#include <ctime>
#define MAXPENDING 500    /* Max connection requests */
#define BUFFSIZE 256

//#define CLIENT1 "172.27.17.11"
//#define CLIENT2 "172.27.17.12"
//#define PORT1 "7945"
//#define PORT2 "7945"

#define CLIENT1 "127.0.0.1"
#define PORT1 "7945"
using namespace std;


void Die(const char *mess) { cout<<"BS-SERVER FATAL ERROR: "<<mess<<endl; exit(1); }
void Sick(const char *mess) { cout<<"BS-SERVER ERROR: "<<mess<<endl; }
void *ThreadFunc(void *arg);

pthread_t ClientThread_1;
pthread_t ClientThread_2;
int sock_1, sock_2;
struct sockaddr_in clientaddr_1;
struct sockaddr_in clientaddr_2;
bool goodbye;
bool finished;

 int main(int argc, char *argv[])
 {

    finished = false;
    bool manual = false;
    if (argc==1)
      manual = true;
    char TimeStamp[56];
    char buffer[BUFFSIZE];
    unsigned int echolen;
    int received = 0;
    goodbye = false;

    /* Create the UDP socket */
    if ((sock_1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("cannot create socket");
        Die("Failed to create socket 1");
    }
    memset(&clientaddr_1, 0, sizeof(clientaddr_1));       /* Clear struct */
    clientaddr_1.sin_family = AF_INET;                  /* Internet/IP */
    clientaddr_1.sin_addr.s_addr = inet_addr(CLIENT1);  /* IP address */
    clientaddr_1.sin_port = htons(atoi(PORT1));       /* server port */

    /* Establish connection */
    if (connect(sock_1,(struct sockaddr *) &clientaddr_1,sizeof(clientaddr_1)) < 0)
    {
        perror("cannot connect to client");
        Die("Failed to connect to the client 1");
    }
 

    if ( pthread_create( &ClientThread_1, NULL, ThreadFunc, &sock_1) ) {
        Die("error in creating thread.");
    }
    
   
  
    struct timeval tv;
    string command;

    time_t now = time(NULL);    
    struct tm *tm_struct=localtime(&now);

    cerr<<"sync-server: sync time at "<<tm_struct->tm_hour<<":"<<tm_struct->tm_min<<endl;
    goodbye=true;
    void *status;


    char Stamp[256];
    struct timeval tvb;
    if (manual)
    {
    while(1){
      cout<<"sync-server: wait for your command..."<<endl;
      cout<<"sync-server:";
      cin>>command;
      if (command != "sync"){
        cout<<"Command not accepted"<<endl;
        continue;
      }
      else
        cout<<"sync-server: send sync command"<<endl;
      gettimeofday(&tvb, NULL);

      sprintf(Stamp, "timesync:%d;",tvb.tv_sec);
      /* Send the word to the server */
      echolen = strlen(Stamp);
      cout<<"\nBS-SERVER[request]:time-sync"<<endl;
      if (send(sock_1, Stamp, echolen, 0) != echolen) // send command
      {
         Die("Mismatch in number of message bytes");
      }
      /* Receive the words from the client */
      cout<<"BS-SERVER: waiting for response"<<endl;
      memset(buffer, 0, sizeof(buffer)); // clear buffer
      int byte_count = recv(sock_1, buffer, sizeof buffer, 0); // block and wait for message
      buffer[byte_count] = '\0';        /* assure null terminated string */
      cout<<"\nBS-SERVER[response]:"<<buffer<<endl;
      }
    }
    
    cerr<<"BS-SERVER: ";
    for (int i=1; i<=10; i++)
    { 
      sleep(1);
      cerr<<i<<"s...";
    }
    cerr<<endl;
    if (finished == true)
      cout<<"BS-SERVER: syncing succeed"<<endl;
    else
      Die("thread did not response");
    
    
    close(sock_2);
    close(sock_1);
    return 0;
 }



 void *ThreadFunc(void *arg){
   int sock = *(int*)arg;
   std::cout<<"\n--->>> Thread "<<sock<<" is alive"<<std::endl;
   struct timeval tvb;
   char Stamp[56];
   char buffer[BUFFSIZE];
   unsigned int echolen;
   int received = 0;
  
	 Stamp[0]='\0';
   gettimeofday(&tvb, NULL);
   sprintf(Stamp, "timesync:%d;",tvb.tv_sec);
    /* Send the word to the server */
   echolen = strlen(Stamp);
    cout<<"\nBS-SERVER[request]:time-sync"<<endl;
    if (send(sock, Stamp, echolen, 0) != echolen) // send command
    {
         Die("Mismatch in number of message bytes");
    }
    /* Receive the words from the client */
    cout<<"BS-SERVER: waiting for response"<<endl;
    memset(buffer, 0, sizeof(buffer)); // clear buffer
    int byte_count = recv(sock, buffer, sizeof buffer, 0); // block and wait for message
    buffer[byte_count] = '\0';        /* assure null terminated string */
    cout<<"\nBS-SERVER[response]:"<<buffer<<endl;
    finished = true;
       //cout<<"BS-SERVER: waiting..."<<endl;
 //sleep 10 minutes;
       
	//if (goodbye==true)
	//   {
	//       cout<<"BS-SERVER: goodbye"<<endl;
	//	   Stamp[0]='\0';
	//	   sprintf(Stamp, "shutdown:shutdown;");
	//	   send(sock, Stamp, strlen(Stamp), 0);
	//       pthread_exit(0);
	//   }
	//   cout<<"BS-SERVER: send sync signal..."<<endl;
   
   return NULL;
 }
