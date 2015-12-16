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
#include <semaphore.h>

#include "ClientIRC.h"
#include "Lista.h"
#include "CanalIRC.h"
#include "ServerIRC.h"

/*
Tutoriales
http://www.tutorialspoint.com/unix_sockets/socket_server_example.htm
 */

typedef struct Params {
    int sock;
    ClientIRC *cl;
} Params;

void *clientHandlerProducer(void *);
int splitMessage(char *argumentos[5], char message[2000]);
int splitMessage2(char *argumentos[5], char message[2000]);
void *consumer(void *);
/*

void *producer(void *);
 */

int stock_count = 0;
const int stock_max_limit = 5;

ClientIRC *ClientConected[10];
int conexiones;
int lock;
/*Listas: Canales, clientes, mensajes*/
ServerIRC *servidor;
Lista *canales;
Lista *clientes;
Lista *mensajes;

int client_sock;

pthread_t HiloServ, HiloServ2;
sem_t semP, semC;

sem_t *mutex, *mut, *full, *empty;

int main(int argc, char** argv) {


    int server_sockfd, portNo, server_len, c;
    int rc;
    struct sockaddr_in server_address, client_address;
    lock = 0;


    sem_init(&mutex, 0, 1);
    sem_init(&mut, 0, 1);
    sem_init(&full, 0, 1);
    sem_init(&empty, 0, 1);
    //Crear ListaHilos
    servidor = StartServer();

    //Start Channels
    clientes = ListaNew();
    canales = ListaNew();
    mensajes = ListaNew();
    servidor->canales = canales;
    servidor->clientes = clientes;
    ListaAdd(canales, NodoNew(CanalIRCNew("#Conectado", "Clientes Conectados")));

    //Launch a new socket.
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0); //Like socket()

    conexiones = 0;
    /*Control Print*/
    printf("\nserver_sockfd: %d", server_sockfd);
    if (server_sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    //Initialize a socket structure
    bzero((char *) &server_address, sizeof (server_address));
    portNo = 9999;
    /*Control Print*/
    printf("\nportNo: %d", portNo);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htons(INADDR_ANY);
    server_address.sin_port = htons(portNo);
    server_len = sizeof (server_address);

    rc = bind(server_sockfd, (struct sockaddr *) & server_address, server_len);
    /*Control Print*/
    printf("\nBind: %d", rc);
    if (rc < 0) {
        perror("ERROR on binding");
        return (EXIT_FAILURE);
    }
    /*Start Listening*/
    rc = listen(server_sockfd, 10);
    /*Control Print*/
    printf("\nListen: %d", rc);

    // printf("\nOli mundo =) !");
    sem_init(&semP, 0, 0);
    sem_init(&semC, 0, 0);

    pthread_create(&HiloServ, NULL, consumer, NULL);
    //pthread_create(&HiloServ2, NULL, producer, NULL);
    //pthread_join(HiloServ, NULL);
    //pthread_join(HiloServ2, NULL);

    sem_destroy(&semC);
    sem_destroy(&semP);


    c = sizeof (struct sockaddr_in);
    do {
        fflush(stdin);
        fflush(stdout);
        //printf("\n%d",TestAndSet(&lock));
        sem_wait(&mutex);
        if (client_sock = accept(server_sockfd, (struct sockaddr *) &client_address, (socklen_t*) & c)) {
            conexiones++;
            printf("\nConnection %d--%d", conexiones, client_sock);
            /*
            if (conexiones > 3) {
                Nodo *nTmp,*nTmp2;
                CanalIRC *cTmp;
                for (nTmp = canales->header; nTmp != NULL; nTmp = nTmp->next) {
                    cTmp = nTmp->cliente;
                    for (nTmp2 = cTmp->clientes->header; nTmp2 != NULL; nTmp2 = nTmp2->next) {
                        ClientIRC *cl = nTmp2->cliente;
                        write(cl->socket, "Mensaje", 7);
                    }
                }
            }
             */
            //write(client_sock, conexiones, 4);


            ListaAdd(servidor->hilos, NodoNew(HiloNew(conexiones)));
            Hilo *tmp;
            tmp = (Hilo*) servidor->hilos->last->cliente;

            /*Creating client with an independent connection*/
            CanalIRC *CTmp;
            CTmp = (CanalIRC *) ListaGet(canales, CanalIRCNew("#Conectado", "Clientes"), (void*) CanalCmpxNombre);
            Params *pTmp = malloc(sizeof (Params));
            if (CTmp != NULL) {
                ClientIRC *nuevoIRC = newClientIRC(conexiones, conexiones + server_sockfd);
                printf("\n%s--%d--%d", CTmp->nombre, nuevoIRC->idLista, nuevoIRC->socket);
                ListaAdd(CTmp->clientes, NodoNew(nuevoIRC));
                ListaAdd(clientes, NodoNew(nuevoIRC));
                pTmp->cl = nuevoIRC;
                pTmp->sock = client_sock;
                //nuevoIRC = CTmp->clientes->header->cliente;
                //printf("\n%s--%d--%d", CTmp->nombre, nuevoIRC->idLista, nuevoIRC->socket);
            }
            if (pthread_create(&(tmp->hilo), NULL, clientHandlerProducer, (void*) pTmp) < 0) {
                perror("could not create thread");
                return (EXIT_FAILURE);
            }
        }
        sem_post(&mutex);
    } while (1);

    return (EXIT_SUCCESS);
}

void *clientHandlerProducer(void *socket_desc) {
    //Get the socket descriptor
    Params *param = socket_desc;
    int sock = param->sock;
    char *message, client_message[2000];
    ClientIRC *irc;
    CanalIRC *cTmp;
    int i, j;
    Nodo *nTmp, *nTmp2;
    int read_size;
    char str[20];
    char a;
    int ia;
    int contArg; //Maximo 5
    char *argumentos[5];
    i = 0;

    write(sock, "Utilizar Comando USER para identificarse\n", 41);
    //Receive a message from client
    do {
        //irc = ListaGet(clientes, param->cl, ClientIRCxId);
        if ((read_size = recv(sock, client_message, 2000, 0)) > 0) {
            sem_wait(&empty);
            sem_wait(&mut);
            //contArg = 0;
            //end of string marker
            client_message[read_size - 2] = '\0';
            client_message[read_size - 1] = '\0';
            client_message[read_size] = '\0';
            irc = ListaGet(clientes, param->cl, ClientIRCxId);
            printf("\nCliente%d-%d", irc->idLista, irc->socket);
            ListaAdd(mensajes, NodoNew(MensajeNew(irc, client_message)));

            //write(sock,client_message,strlen(client_message));
            //contArg=splitMessage(argumentos,client_message);
            //Send the message back to client
            //a = client_message[0];
            //sleep(1);
            //a = 'q';
            /*
            if (a != 'q') {
                ia = a - '0';
                irc = ListaGet(clientes, newClientIRC(ia,0), ClientIRCxId);
                if (irc != NULL) {
                    sprintf(str, "Soy el cliente %d", irc->idLista);
                } else {
                    sprintf(str, "Invalido cod:%d", -1);
                }
            } else {
                break;
            }
             */
            //clear the message buffer
            memset(client_message, 0, 2000);
            i++;
            /*
            if (i > 3) {
                break;
            }
             */
        }
        sem_post(&mut);
        sem_post(&full);
    } while (1);
    read_size = 0;
    if (read_size == 0) {
        printf("\nClient disconnected");
        fflush(stdout);
        /*
        for(i=0;i<sock;i++){
            write(i,"desconexiones",13);
        }
         */
        //close(sock);
        //pthread_exit(0);
    } else if (read_size == -1) {
        perror("recv failed");
    }
    /*
    irc = ListaRemove(clientes, newClientIRC(0, sock), (void *) ClientIRCxSock);
    for (nTmp = canales->header; nTmp != NULL; nTmp = nTmp->next) {
        cTmp = nTmp->cliente;
        i = 0;
        for (nTmp2 = cTmp->clientes->header; nTmp2 != NULL; nTmp2 = nTmp2->next) {
            ClientIRC *cl = nTmp2->cliente;
            if (irc->idLista == cl->idLista && irc->socket == cl->socket) {
                i = 1;
            }
        }
        ListaRemove(cTmp->clientes, irc, (void *) ClientIRCxSock);
        if (i == 1) {
            for (nTmp2 = cTmp->clientes->header; nTmp2 != NULL; nTmp2 = nTmp2->next) {
                ClientIRC *cl = nTmp2->cliente;
                write(cl->socket, "Desconexion", 12);
            }
        }
    }
     */
    //ListaRemove(servidor->hilos,HiloNew(conexiones),(void *)HiloxId);
    //printf("%d",servidor->nombre);
    return 0;
}

int splitMessage(char *argumentos[5], char message[2000]) {
    int nArgs;
    nArgs = 0;
    if (message[0] != '\0' && message[0] != ' ') {
        argumentos[nArgs] = strtok(message, " ");
        do {
            nArgs++;
            argumentos[nArgs] = strtok(NULL, " ");
        } while (argumentos[nArgs] != NULL);
    }
    return nArgs;
}

int splitMessage2(char *argumentos[5], char message[2000]) {
    int nArgs;
    nArgs = 0;
    if (message[0] != '\0' && message[0] != ' ') {
        argumentos[nArgs] = strtok(message, " ");
        do {
            nArgs++;
            argumentos[nArgs] = strtok(NULL, " ");
        } while (argumentos[nArgs] != NULL && nArgs < 3);
        nArgs++;
        argumentos[nArgs] = strtok(NULL, "\0");
    }
    return nArgs;
}

/*
void *producer(void *arg) {
    int i, sum=0;
    for (i = 0; i < 10; i++) {

        while(stock_max_limit == stock_count){
            printf("stock overflow, production on wait..\n");
            sem_wait(&semC);
            printf("production operation continues..\n");
        }

        sleep(1);   //production decided here
        stock_count++;
        printf("P::stock-count : %d\n",stock_count);
        sem_post(&semP);
        printf("P::post signal..\n");
    }
 }
 */

void *consumer(void *arg) {
    int i, sum = 0;
    Mensaje *men;

    do {
        fflush(stdin);
        fflush(stdout);
        sem_wait(&full);
        sem_wait(&mut);
        if (ListaSize(mensajes) > 0) {
            char *argumentos[5];
            char mens[2000];
            argumentos[0] = NULL;
            argumentos[1] = NULL;
            argumentos[2] = NULL;
            argumentos[3] = NULL;
            argumentos[4] = NULL;
            int nArg;
            men = ListaRemoveFirst(mensajes);
            strcpy(mens, men->mensaje);
            nArg = splitMessage(argumentos, men->mensaje);
            if (strlen(men->cSend->nickname) != 0) {
                if (nArg > 0 && argumentos[0] != NULL) {
                    if (strcmp(argumentos[0], "QUIT") == 0) {
                        quit(argumentos, men->cSend, servidor);
                    } else if (strcmp(argumentos[0], "INFO") == 0) {
                        write(men->cSend->socket, info(servidor, argumentos[1]), strlen(info(servidor, argumentos[1])));
                    } else if (strcmp(argumentos[0], "TIME") == 0) {
                        write(men->cSend->socket, timee(servidor, argumentos[1]), strlen(timee(servidor, argumentos[1])));
                    } else if (strcmp(argumentos[0], "VERSION") == 0) {
                        write(men->cSend->socket, version(servidor, argumentos[1]), strlen(version(servidor, argumentos[1])));
                    } else if (strcmp(argumentos[0], "MODT") == 0) {
                        write(men->cSend->socket, motd(servidor, argumentos[1]), strlen(motd(servidor, argumentos[1])));
                    } else if (strcmp(argumentos[0], "LIST") == 0) {
                        list(servidor, argumentos, men->cSend);
                    } else if (strcmp(argumentos[0], "USER") == 0) {
                        char *argumentos2[5];
                        argumentos2[0] = NULL;
                        argumentos2[1] = NULL;
                        argumentos2[2] = NULL;
                        argumentos2[3] = NULL;
                        argumentos2[4] = NULL;
                        splitMessage2(argumentos2, mens);
                        user(servidor, argumentos2, men->cSend);
 					} else if (strcmp(argumentos[0], "USERS") == 0) {
                        users(servidor, argumentos, men->cSend);
					 } else if (strcmp(argumentos[0], "SETNAME") == 0) {
                        if (nArg > 1) {
                            char *argumentos2[5];
                            argumentos2[0] = NULL;
                            argumentos2[1] = NULL;
                            argumentos2[2] = NULL;
                            argumentos2[3] = NULL;
                            argumentos2[4] = NULL;
                            splitMessage2(argumentos2, mens, 0);
                            setname(servidor, argumentos2, men->cSend);
                        } else {
                            write(men->cSend, "ERR_NEEDMOREPARAMS\n", 19);
                        }
                    } else if (strcmp(argumentos[0], "/NICK") == 0) {
                        if (nArg == 2) {
                            nick(servidor, argumentos, men->cSend);
                        } else {
                            write(men->cSend, "ERR_NONICKNAMEGIVEN\n", 20);
                        }
                    
                }
            } else if (nArg > 0) {
                if (strcmp(argumentos[0], "/QUIT") != 0 && strcmp(argumentos[0], "/USER") != 0) {
                    write(men->cSend->socket, "Utilizar Comando /USER para identificarse\n", 42);
                }
                if (strcmp(argumentos[0], "/QUIT") == 0) {
                    char *argumentos2[5];
                    argumentos2[0] = NULL;
                    argumentos2[1] = NULL;
                    argumentos2[2] = NULL;
                    argumentos2[3] = NULL;
                    argumentos2[4] = NULL;
                    splitMessage2(argumentos2, mens, 0);
                    quit(argumentos2, men->cSend, servidor);
                } else if (strcmp(argumentos[0], "/USER") == 0) {
                    char *argumentos2[5];
                    argumentos2[0] = NULL;
                    argumentos2[1] = NULL;
                    argumentos2[2] = NULL;
                    argumentos2[3] = NULL;
                    argumentos2[4] = NULL;
                    splitMessage2(argumentos2, mens);
                    user(servidor, argumentos2, men->cSend);
                }
            }
        }
        sem_post(&mut);
        sem_post(&empty);
    } while (1);
}

