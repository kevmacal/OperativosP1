#include <stdio.h>
#include <stdlib.h>

#include "ClientIRC.h"

ClientIRC *newClientIRC(int id){
    ClientIRC *nuevo=malloc(sizeof(ClientIRC));
    nuevo->idArreglo=id;
    return nuevo;
}