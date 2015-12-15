/* 
 * File:   Cola.h
 * Author: marlon
 *
 * Created on December 12, 2015, 1:00 PM
 */

#ifndef LISTA_H
#define	LISTA_H

#include "ClientIRC.h"


#ifdef	__cplusplus
extern "C" {
#endif
    
    int (*cmpfn)(void *, void *);
    
    typedef struct Nodo{
        void* cliente;
        struct Nodo* next;
    }Nodo;
    
    typedef struct Lista{
        Nodo *header;
        Nodo *last;
    }Lista;
/*Funciones del Nodo*/
Nodo *NodoNew(void *Cli);

/*Funciones de la cola*/
Lista *ListaNew();
void ListaAdd(Lista *list, Nodo *nAdd);
int ListaSize(Lista *list);
void *ListaGet(Lista *lista, void *comparador, int (*fn)(void*, void*));
void *ListaRemoveFirst(Lista *lista);
void *ListaRemoveLast(Lista *lista);
void *ListaRemove(Lista *lista, void *comparador, int (*fn)(void*, void*));

#ifdef	__cplusplus
}
#endif

#endif	/* COLA_H */

