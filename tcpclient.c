/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tcpclient.c
 * Author: robertboyer
 *
 * Created on January 27, 2018, 10:11 AM
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include <errno.h>
#include <sys/utsname.h>

#define SERVER_PORT 6432
#define MAX_LINE 256
#define MAXNAME 256
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

int main(int argc, char* argv[])
{

  
    struct  utsname buffer;
    struct  hostent *hp;
    struct  sockaddr_in sin;
    char    *host;
    char    *cName;
    char    *username;
    char    buf[MAX_LINE];
    int     s;
    int     len;
    short   packettype;


    if(argc == 3){
            host = argv[1];
            username = argv[2];
    }
    else{
            fprintf(stderr, "usage:client servername username\n");
            exit(1);
    }

    printf ("TCP Client will use server port %d\n", SERVER_PORT);
    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if(!hp){
            fprintf(stderr, "unknown host: %s\n", host);
            exit(1);
    }

    printf("TCP Client, active open\n");
    /* active open */
    if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
            perror("tcpclient: socket");
            exit(1);
    }

    printf("TCP Client, build address data structure\n");
    /* build address data structure */
    bzero((char*)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);


    if(connect(s,(struct sockaddr *)&sin, sizeof(sin)) < 0){
            perror("tcpclient: connect");
            close(s);
            exit(1);
    }

    /* Constructing the registration packet at client */
    /* Using uname because gethostname() didn't work on Mac OSX */
    errno = 0;
    if (uname(&buffer) != 0) { 
      perror("username");         
      exit(EXIT_FAILURE);      
    }
    packet_reg.type = htons(REG);
    strcpy(packet_reg.mName,buffer.nodename);
    strcpy(packet_reg.userName,username); 

    /* Send the registration packet to the server */
    if (send(s, &packet_reg, sizeof(packet_reg),0) <0) {
            printf("\n Send failed\n");
            exit(1);
    }
    /* Print outs per homework instructions */
     printf("\n2) Request sent by Client: %hd %s %s \n", (ntohs(packet_reg.type)), packet_reg.mName, packet_reg.userName);
     
    /* wait for confirmation from server */
    packettype = 0;
    while (packettype != CONF) {
        if(recv(s,&packet_reg,sizeof(packet_reg),0) <0) {
            printf("\n Could not receive confirmation packet \n");
            exit(1);
        }
        packettype = ntohs(packet_reg.type);

    }
    /* Print outs per homework instructions */
     printf("\n4) Response received by Client: %hd %s %s \n", (ntohs(packet_reg.type)), packet_reg.mName, packet_reg.userName);
    
    strncpy(packet_reg.data, "", MAX_LINE); /* make sure data is empty */
    while(fgets(buf, sizeof(buf), stdin)){
        buf[MAX_LINE-1] = '\0';
        len = strlen(buf) + 1;                  /* get data */

        packet_reg.type = htons(CHATDATA);      /* put data in packet */
        strcpy(packet_reg.data,buf);

        if (send(s, &packet_reg, sizeof(packet_reg),0) <0) {
            printf("\n Send failed\n");
            exit(1);
        } /*send data */
        printf("\n2) Chat sent by Client: %hd %s %s %s \n",
            (ntohs(packet_reg.type)), 
             packet_reg.mName,
             packet_reg.userName,
             packet_reg.data);


        /* wait for chat response packet from server */
        packettype = 0;
        while (packettype != CHATRESPONSE) {
            if(recv(s,&packet_reg,sizeof(packet_reg),0) <0)
            {
                printf("\n Could not receive chat response packet \n");
                exit(1);
            }
            packettype = ntohs(packet_reg.type);
        }
        /* Print outs per homework instructions */
        printf("\n4) Response received by Client: %hd %s %s \n", (ntohs(packet_reg.type)), packet_reg.mName, packet_reg.userName);
    
        strncpy(packet_reg.data, "", MAX_LINE); /* sent now, so empty */
    }
}
