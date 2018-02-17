/* 
 * File:   tcpserver.c
 * Author: robertboyer
 *
 * Created on February 11, 2018, 1:31 PM
 *
 * File:   tcpserver.c
 * Author: robertboyer
 *
 * Created on January 27, 2018, 10:17 AM
 */
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<netdb.h>

#define SERVER_PORT 6432
#define MAX_LINE 256
#define MAX_PENDING 5
#define MAXNAME 256
#define MAXCLIENTS 10
#define REG 121
#define CONF 221
#define CHATDATA 131
#define CHATRESPONSE 231

/* structure of the packet for the CSCI_632 chat server project*/
struct  packet { 
   short type;
   char userName[MAXNAME];
   char mName [MAXNAME];
   char data [MAXNAME];
};

struct packet packet_reg;

/* structure of the Registration Table for the CSCI_632 chat server project */
struct registrationTable{
    int port;
    int sockid;
    char mName[MAXNAME];
    char userName[MAXNAME];
};

struct registrationTable table[MAXCLIENTS];

int main(int argc, char* argv[])
{
    struct  sockaddr_in sin;
    struct  sockaddr_in clientAddr;
    char    buf[MAX_LINE];
    int     s, new_s;
    int     len;
    int     regindex;
    int     optval;
    short   packettype;                     // per our protocol
    pid_t   pid;                     //used for our fork call

    printf ("TCP Server will use server port %d\n", SERVER_PORT);

    /* setup passive open */
    if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
            perror("tcpserver: socket");
            exit(1);
    }

    /* Eliminates "ERROR on binding: Address already in use" error. */   
    optval = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, 
                (const void *)&optval , sizeof(int));

    /* build address data structure and bind */
    bzero((char*)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);
    if(bind(s,(struct sockaddr *)&sin, sizeof(sin)) < 0){
            perror("tcpclient: bind");
            exit(1);
    }
    
    listen(s, MAX_PENDING);
 
    /* we are going to use parent and child processes 
     * the parent process will accept connections and then register them
     * the child process will process the data  
     * first start by accepting a connection in parent process*/
    pid = 1;
    if((new_s = accept(s, (struct sockaddr *)&clientAddr, &len)) < 0){
            perror("tcpserver: accept");
            exit(1);
        }    
    printf("\n PID: %d Client's port is %d \n", pid, ntohs(clientAddr.sin_port));
    regindex = 0;   /*set index into registration table array */
      
    while(1){ 
    
        // get a packet for currently accept()ed connection
        packettype = 0;
        if(recv(new_s,&packet_reg,sizeof(packet_reg),0) <0){
                printf("\n Pid: %d Could not receive packet \n", pid);
                exit(1);
        }      
        packettype = ntohs(packet_reg.type);
        
        // check the packet type per our protocol
        switch(packettype) {
            case REG  :
                if (regindex <= MAXCLIENTS) {
                    /* Print outs per homework instructions */
                    printf("\nPID: %d 1) Request sent to Server: %hd %s %s \n",
                         pid,
                         ntohs(packet_reg.type), 
                         packet_reg.mName, 
                         packet_reg.userName);

                    /* Send confirmation packet to client */
                    packet_reg.type = htons(CONF);
                    if (send(new_s, &packet_reg, sizeof(packet_reg),0) <0){
                            printf("\n Send confirmation packet failed\n");
                            exit(1);
                    }
                    /* Print outs per homework instructions */
                    printf("\nPID: %d 3) Response sent by Server: %hd %s %s \n", 
                         pid,
                         ntohs(packet_reg.type), 
                         packet_reg.mName, 
                         packet_reg.userName);

                    /* put new chat client info in registration table */
                    table[regindex].port = clientAddr.sin_port;
                    table[regindex].sockid = new_s;
                    strcpy(table[regindex].userName, packet_reg.userName);
                    strcpy(table[regindex].mName, packet_reg.mName);
                    regindex++; /* move to next element in registration table */
                
                    printf("\n PID: %d Welcome %s @ %s!\n",
                      pid,
                      packet_reg.userName, 
                      packet_reg.mName);
                }
                
                break; 
	
            case CHATDATA  :
                /* If we are original parent process, create a new child process
                 * to handle data for each client. But if we are a
                 * child process, don't fork, just handle chat data      */       
                if (pid == 1) pid = fork();  // create child process
                if (pid == 0) { // 0 means we are the child process                  
                    printf("\n PID: %d Message from %s @ %s |> %s\n",
                       pid,
                       packet_reg.userName, 
                       packet_reg.mName,
                       packet_reg.data );

                   /* Send chat response  packet to client */
                   packet_reg.type = htons(CHATRESPONSE); 
                   if (send(new_s, &packet_reg, sizeof(packet_reg),0) <0){
                       printf("\n PID: %d Send chat response packet failed\n", pid);
                       exit(1);
                   }
                   /* Print outs per homework instructions */
                   printf("\n PID: %d 3) Response sent by Server: %hd %s %s \n",
                       pid,
                       ntohs(packet_reg.type), 
                       packet_reg.mName,
                       packet_reg.userName);

                   packettype = 0;
                }
                else { // if here we are the original parent process
                  pid = 1; // mark that we are still original parent process
                  packettype = 0;
                  // original process now waits for another connection
                  if((new_s = accept(s, (struct sockaddr *)&clientAddr,
                      &len)) < 0){
                        perror("tcpserver: accept");
                        exit(1);
                  }    
                  printf("\n PID: %d Client's port is %d \n",
                          pid, ntohs(clientAddr.sin_port));
                } 
                
                break;
                
            default : 
                printf("\n PID: %d bad packet type received by server %hd\n",
                      pid, packettype);
                exit(1);                
        } /* end case statement */ 
        
    } /* end main while loop */
    
} /* end */
    