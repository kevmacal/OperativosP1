#include <stdio.h>
#include <stdlib.h>

#include "Lista.h"

/*Funciones de Nodo*/

Nodo *NodoNew(void *Cli){
    Nodo* nuevo=malloc(sizeof(Nodo));
    nuevo->cliente=Cli;
    nuevo->next=NULL;
    return nuevo;
}


/*Funciones de Lista*/

Lista *ListaNew(){
    Lista *nueva=malloc(sizeof(Lista));
    nueva->header=NULL;
    nueva->last=NULL;
    return nueva;
}

void ListaAdd(Lista *list,Nodo *nAdd){
    Nodo *nTmp;
    if(list->header==list->last&&list->header==NULL){
        list->header=nAdd;
        list->last=nAdd;
    }else{
        list->last->next=nAdd;
        list->last=nAdd;
    }
}

int ListaSize(Lista *list){
    int i;
    Nodo *it;
    i=0;
    for(it=list->header;it!=NULL;it=it->next){
        i++;
    }
    return i;
}

void *ListaGet(Lista *lista, void *comparador, int (*fn)(void*, void*)){
    Nodo *it;
    for(it=lista->header;it!=NULL;it=it->next){
        if(fn(it->cliente,comparador)==0){
            return it->cliente;
        }
    }
    return NULL;
}
void *ListaRemoveFirst(Lista *lista){
    Nodo* it;
    if(ListaSize(lista)==0){
        return NULL;
    }
    it=lista->header;
    lista->header=it->next;
    if(lista->header==NULL){
        lista->last=NULL;
    }
    it->next=NULL;
    return it->cliente;
}
void *ListaRemoveLast(Lista *lista){
    Nodo* it,*tmp;
    if(ListaSize(lista)==0){
        return NULL;
    }
    for(it=lista->header;it!=NULL;it=it->next){
        if(it->next==lista->last){
            tmp=it;
            it=it->next;
        }
    }
    it=lista->last;
    lista->last=tmp;
    tmp->next=NULL;
    return it->cliente;
}
void *ListaRemove(Lista *lista, void *comparador, int (*fn)(void*, void*)){
    Nodo *it,*tmp;
    tmp=NULL;
    if(ListaSize(lista)==0){
        return NULL;
    }
    if(fn(lista->header->cliente,comparador)==0){
        return ListaRemoveFirst(lista);
    }
    if(fn(lista->last->cliente,comparador)==0){
        return ListaRemoveLast(lista);
    }
    for(it=lista->header;it!=NULL;it=it->next){
        if(it->next!=NULL){
            if(fn(it->next->cliente,comparador)==0){
                tmp=it;
                it=lista->last;
            }
        }
    }
    if(tmp!=NULL){
        it=tmp->next;
        tmp->next=it->next;
        it->next=NULL;
        return it->cliente;
    }
    return NULL;
}

void ListaReplaceAll(Lista *lista, void *comparador, int (*fn)(void*, void*)){
    Nodo *it;    
    for(it=lista->header;it!=NULL;it=it->next){
        if(fn(it->cliente,comparador)==0){
            it->cliente=comparador;
        }
    }
}