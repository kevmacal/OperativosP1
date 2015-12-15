#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <linux/kernel.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/wait.h>
#include <syscall.h>
#include <sys/utsname.h>
#include <time.h>

#include "ClientIRC.h"


Mensaje *MensajeNew(ClientIRC *cS,char mensaje[2000]){
    Mensaje *nuevo=malloc(sizeof(Mensaje));
    nuevo->cSend=cS;
    strcpy(nuevo->mensaje,mensaje);
    return nuevo;
}



ClientIRC *newClientIRC(int id,int sock) {
    ClientIRC *nuevo = malloc(sizeof (ClientIRC));
    nuevo->idLista = id;
    nuevo->socket=sock;
    return nuevo;
}

int ClientIRCxId(ClientIRC *c1,ClientIRC *c2){
    if(c1->idLista==c2->idLista){
        return 0;
    }
    return -1;
}

int ClientIRCxSock(ClientIRC *c1,ClientIRC *c2){
    if(c1->socket==c2->socket){
        return 0;
    }
    return -1;
}

int ClientIRCxUser(ClientIRC *c1,ClientIRC *c2){
   return strcmp(c1->nickname,c2->nickname); 
}
