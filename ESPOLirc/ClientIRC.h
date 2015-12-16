/* 
 * File:   ClientIRC.h
 * Author: marlon
 *
 * Created on December 6, 2015, 9:29 AM
 */

#ifndef CLIENTIRC_H
#define	CLIENTIRC_H

#include "Lista.h"




#ifdef	__cplusplus
extern "C" {
#endif
    
typedef struct Mensaje{
    struct ClientIRC *cSend;
    char mensaje[2000];
}Mensaje;
    
    
typedef struct ClientIRC{
    int idLista;
    int socket;
    int isConectado;  //Si es cero, es que no esta conectado, si es uno, esta conectado (Inicio sesion valida)
    char nickname[9]; //debe ser de nueve caracteres
    char realName[100];
    char hostName[50];
}ClientIRC;

/*Mensaje*/
Mensaje *MensajeNew(ClientIRC *cS,char mensaje[2000]);


ClientIRC *newClientIRC(int id,int sock);
int ClienteIRCVerificarConexion(Lista *usuarios,ClientIRC *cl); //Si es uno es posible realizar otra conexion
void ClientIRCConectar(ClientIRC *cl, int valor);
int ClientIRCxId(ClientIRC *c1,ClientIRC *c2);
int ClientIRCxSock(ClientIRC *c1,ClientIRC *c2);
int ClientIRCxUser(ClientIRC *c1,ClientIRC *c2);
int ClientIRCxUserConnected(ClientIRC *c1,ClientIRC *c2);
ClientIRC *ClientIRCResetUsername(ClientIRC *cl);


#ifdef	__cplusplus
}
#endif

#endif	/* CLIENTIRC_H */

