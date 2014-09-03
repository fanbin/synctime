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
#include "sync.h"

#define MAXPENDING 5    /* Max connection requests */
#define BUFFSIZE 256

using namespace std;

ofstream log;

void Die(const char *mess) { log<<"BS-client FATAL ERROR: "<<mess<<endl; exit(1); }
void Sick(const char *mess) { log<<"BS-client ERROR: "<<mess<<endl; }
void Say(const char *mess) { log<<"BS-client: "<<mess<<endl; }


int HandleClient(int sock, char* machine) {
    char buffer[BUFFSIZE];
    char responsebuffer[BUFFSIZE];
    int received = -1;
    /* Receive message */
    log<<"BS-client:connection ready, wait for command"<<endl;
    if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
        Die("Failed to receive initial bytes from client");
    }
    string tt;
    struct timeval owntv;
    struct timeval tv;
    struct tm *ptm;
    /* Send bytes and check for more incoming data in loop */
    while (received > 0) 
	{
        // parse time information first
		if (Parse(buffer, "shutdown")!="0")
		{
			log<<buffer<<endl;
			return -1;
		}

        tt = Parse(buffer, "timesync");
        if (tt != "0"){

            int errorno1 = gettimeofday(&owntv, NULL);
            ptm = localtime( &(owntv.tv_sec) );
            tv.tv_sec = atoi(tt.c_str());
            tv.tv_usec =  0;
            int offset = tv.tv_sec-owntv.tv_sec;
            int errorno2 = settimeofday(&tv, NULL);
            log<<"BS-client["<<ptm->tm_mon<<"/"<<ptm->tm_mday<<"/"<<ptm->tm_year+1900<<" "<<ptm->tm_hour<<":"<<ptm->tm_min<<":"<< ptm->tm_sec<<"]: timesync-signal="
            <<atoi(tt.c_str())<<";operation="<<(errorno2==0?"succeed":"fail")<<endl;

            /* Send response to server */
            sprintf(responsebuffer, "client:%s;timeoffset:%d;result:%d;",machine,offset, errorno2);
            if (send(sock, responsebuffer, sizeof(responsebuffer), 0) != sizeof(responsebuffer))
            {
                Die("Failed to send bytes to client");
            }
        }
        // parse other information

        /* Check for more data */
        memset(buffer, 0, sizeof(buffer));
        log<<"BS-client:connection ready, wait for command"<<endl;
        if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
            Die("Failed to receive additional bytes from client");
        }
     }
	return 0;
}


int main(int argc, char *argv[])
{
        int serversock, clientsock;
        struct sockaddr_in echoserver, echoclient;

        if (argc != 3)
        {
            fprintf(stderr, "USAGE: client <machinename> <port>\n");
            exit(1);
        }
        /* Our process ID and Session ID */
        pid_t pid, sid;

        /* Fork off the parent process */
        pid = fork();
        if (pid < 0) {
            exit(EXIT_FAILURE);
        }

        //exit the parent process
        if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        /* Change the file mode mask */
        umask(0);

        /* Open any logs here */
        log.open("/var/log/bs-client.log");
        if(!log.is_open())
        {
            cerr<<"BS-client FATAL ERROR: cannot open log "<<endl;
            exit(1);
        }

        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
                Die("Failed to get sid");
        }

        /* Change the current working directory */
        if ((chdir("/")) < 0) {
                Die("Failed to chdir to root");
        }

        /* Close out the standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        /* Daemon-specific initialization goes here */

        /* The Big Loop */
        /* Create the TCP socket */
        if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
           Die("Failed to create socket");
        }
        /* Construct the server sockaddr_in structure */
        memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
        echoserver.sin_family = AF_INET;                  /* Internet/IP */
        echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
        echoserver.sin_port = htons(atoi(argv[2]));       /* server port */

          /* Bind the listening socket */
         if (bind(serversock, (struct sockaddr *) &echoserver,sizeof(echoserver)) < 0)
         {
           Die("Failed to bind system socket");
         }
         /* Listen on the socket */
         if (listen(serversock, MAXPENDING) < 0) 
		 {
           Die("Failed to listen on server socket");
         }

         /* Run until cancelled */
         while (1)
         {
            unsigned int clientlen = sizeof(echoclient);
            /* Wait for server time-sync signal */
            log<<"BS-client:system ready, wait for connect"<<endl;
            if ((clientsock = accept(serversock, (struct sockaddr *) &echoclient,
                             &clientlen)) < 0)
            {
                   Die("Failed to accept server connection");
            }

            log<<"BS-client:server connection detected "<<inet_ntoa(echoclient.sin_addr)<<endl;
            if(HandleClient(clientsock, argv[1])==-1)
			{
				Die("normal exit");
				exit(EXIT_SUCCESS);
			}
            log<<"BS-client:server connection from "<<inet_ntoa(echoclient.sin_addr)<<" breaks"<<endl;

         }
        exit(EXIT_SUCCESS);
}
