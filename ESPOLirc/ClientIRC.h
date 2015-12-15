/* 
 * File:   ClientIRC.h
 * Author: marlon
 *
 * Created on December 6, 2015, 9:29 AM
 */

#ifndef CLIENTIRC_H
#define	CLIENTIRC_H



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
    char nickname[9]; //debe ser de nueve caracteres
    char realName[100];
    char hostName[50];
}ClientIRC;

/*Mensaje*/
Mensaje *MensajeNew(ClientIRC *cS,char mensaje[2000]);


ClientIRC *newClientIRC(int id,int sock);
int ClientIRCxId(ClientIRC *c1,ClientIRC *c2);
int ClientIRCxSock(ClientIRC *c1,ClientIRC *c2);
int ClientIRCxUser(ClientIRC *c1,ClientIRC *c2);


#ifdef	__cplusplus
}
#endif

#endif	/* CLIENTIRC_H */

