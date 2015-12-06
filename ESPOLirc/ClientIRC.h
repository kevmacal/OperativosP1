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
    
typedef struct ClientIRC{
    int idArreglo;
}ClientIRC;

ClientIRC *newClientIRC(int id);

#ifdef	__cplusplus
}
#endif

#endif	/* CLIENTIRC_H */

