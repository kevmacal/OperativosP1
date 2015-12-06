/* 
 * File:   main.c
 * Author: Grupo Pro Operativos
 *
 * Created on November 29, 2015, 11:30 AM
 */

#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>

/*
Tutoriales
http://www.tutorialspoint.com/unix_sockets/socket_server_example.htm
 */

void *clientHandler(void *);

int main(int argc, char** argv) {
    
    pthread_t thread1;
    int server_sockfd,portNo,server_len,client_sock,c;
    int rc;
    struct sockaddr_in server_address,client_address;    
    int conexiones;
    
    //Launch a new socket.
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0); //Like socket()
    
    conexiones=0;
    /*Control Print*/
    printf("\nserver_sockfd: %d",server_sockfd);
    if (server_sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    //Initialize a socket structure
    bzero((char *) &server_address, sizeof(server_address));
    portNo = 9999;
    /*Control Print*/
    printf("\nportNo: %d",portNo);    
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htons(INADDR_ANY);
    server_address.sin_port = htons(portNo);
    server_len = sizeof (server_address);
    
    rc = bind(server_sockfd, (struct sockaddr *) & server_address, server_len);
    /*Control Print*/
    printf("\nBind: %d",rc);
    if(rc<0){
        perror("ERROR on binding");
        return (EXIT_FAILURE);
    }
    /*Start Listening*/
    rc = listen(server_sockfd, 5);
    /*Control Print*/
    printf("\nListen: %d",rc);

    printf("\nOli mundo =) !");
    
    c = sizeof(struct sockaddr_in);
    while((client_sock = accept(server_sockfd, (struct sockaddr *)&client_address, (socklen_t*)&c))){
        conexiones++;
        printf("\nConnection %d",conexiones);
         
        if( pthread_create( &thread1 , NULL ,  clientHandler , (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return (EXIT_FAILURE);
        }        
    }

    return (EXIT_SUCCESS);
}

void *clientHandler(void *socket_desc)
{
    
} 