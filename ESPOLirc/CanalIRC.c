
#include <stdlib.h>
#include <string.h>

#include "CanalIRC.h"

CanalIRC *CanalIRCNew(char nombre[50],char topico[50]){
    CanalIRC *nuevo=malloc(sizeof(CanalIRC));
    strcpy(nuevo->nombre,nombre);
    strcpy(nuevo->topic,topico);
    nuevo->clientes=ListaNew();
    return nuevo;
}

int CanalCmpxNombre(CanalIRC *c1,CanalIRC *c2){
    return strcmp(c1->nombre,c2->nombre);
}
