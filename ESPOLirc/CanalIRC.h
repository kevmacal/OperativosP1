/* 
 * File:   CanalIRC.h
 * Author: jackie
 *
 */

#ifndef CANALIRC_H
#define	CANALIRC_H

#include "Lista.h"


#ifdef	__cplusplus
extern "C" {
#endif
    
typedef struct CanalIRC{
    char nombre[50]; //debe ser de 50 caracteres
    char topic[50]; //Topico
    Lista *clientes;
}CanalIRC;

CanalIRC *CanalIRCNew(char nombre[50],char topico[50]);
int CanalCmpxNombre(CanalIRC *c1,CanalIRC *c2);


#ifdef	__cplusplus
}
#endif

#endif	/* CANALIRC_H */
