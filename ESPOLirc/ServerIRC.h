/* 
 * File:   ServerRC.h
 * Author: jackie
 *
 */

#ifndef SERVERRC_H
#define	SERVERIRC_H

#include "Lista.h"
#include "ClientIRC.h"
#include "CanalIRC.h"


#ifdef	__cplusplus
extern "C" {
#endif
    
typedef struct ServerIRC{
    char nombre[50]; //debe ser de 50 caracteres
    Lista* hilos;
    Lista *canales;
    Lista *clientes;
}ServerIRC;

typedef struct Hilo{
    int id;
    pthread_t hilo;
}Hilo;

Hilo *HiloNew(int id);
int HiloxId(Hilo *h1,Hilo *h2);

ServerIRC *StartServer();
int TestAndSet(int *lock);
void limpiarCadena(char cadena[2000]);

const char *info(ServerIRC *servidor,char target[50]) ; 
const char *timee(ServerIRC *servidor, char target[50]);
const char *version(ServerIRC *servidor, char target[50]);
void quit(char *argumentos[5],ClientIRC *cl,ServerIRC *server);
const char *motd(ServerIRC *servidor, char target[50]);
void list(ServerIRC *servidor,char *argumentos[5],ClientIRC *cl);
void user(ServerIRC *server, char *argumentos[5],ClientIRC *cl);
void users(ServerIRC *servidor, char *argumentos[5],ClientIRC *cl);
void setname(ServerIRC *server, char *argumentos[5], ClientIRC *cl);
void join(ServerIRC *server, char *argumentos[5],ClientIRC *cl);
void part(ServerIRC *server, char *argumentos[5], ClientIRC *cl);

#ifdef	__cplusplus
}
#endif

#endif	/* SERVERIRC_H */
