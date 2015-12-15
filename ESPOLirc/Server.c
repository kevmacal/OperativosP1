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

#include "ServerIRC.h"
#include "CanalIRC.h"

//http://stackoverflow.com/questions/3381080/reading-all-content-from-a-text-file-c
//falta lo del patch, lo del target server y el argumento

ServerIRC *StartServer() {
    ServerIRC *iniciar = malloc(sizeof (ServerIRC));
    strcpy(iniciar->nombre, "Server1");
    iniciar->hilos = ListaNew();
    iniciar->canales = NULL;
    return iniciar;
}

int TestAndSet(int *lock) {
    int tmp = *lock;
    *lock = 1;
    return tmp;
}

Hilo *HiloNew(int id) {
    Hilo *nuevo = malloc(sizeof (Hilo));
    nuevo->hilo = NULL;
    nuevo->id = id;
    return nuevo;
}

int HiloxId(Hilo *h1, Hilo *h2) {
    if (h1->id == h2->id) {
        return 0;
    }
    return -1;
}

const char *info(ServerIRC *servidor, char target[50]) {

    char *fcontent = NULL;
    int fsize = 0;
    FILE *fp;

    if (target == NULL) {
        fp = fopen("/etc/lsb-release", "r");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            fsize = ftell(fp);
            rewind(fp);

            fcontent = (char*) malloc(sizeof (char) * fsize);
            fread(fcontent, 1, fsize, fp);

            fclose(fp);
        }
        return fcontent;
    } else {
        if (strcmp(target, servidor->nombre) == 0) {
            fp = fopen("/etc/lsb-release", "r");
            if (fp) {
                /*
                struct utsname patch;
                int un = uname(&patch);
                 */
                fseek(fp, 0, SEEK_END);
                fsize = ftell(fp);
                rewind(fp);

                fcontent = (char*) malloc(sizeof (char) * fsize);
                fread(fcontent, 1, fsize, fp);

                fclose(fp);
                /*
                strcat(fcontent, "\nSystem: ");
                strcat(fcontent, patch.sysname);
                strcat(fcontent, "\nName: ");
                strcat(fcontent, patch.nodename);
                strcat(fcontent, "\nRelease: ");
                strcat(fcontent, patch.release);
                strcat(fcontent, "\nVersion: ");
                strcat(fcontent, patch.version);
                strcat(fcontent, "\nMachine Architecture: ");
                strcat(fcontent, patch.machine);
                 */
            }
            return fcontent;
        } else {
            return "ERR_NOSUCHSERVER\n";
        }
    }
}

const char *timee(ServerIRC *servidor, char target[50]) {

    time_t t;
    char* ct;
    if (target == NULL) {
        t = time(NULL);
        ct = ctime(&t);
    } else {
        if (strcmp(target, servidor->nombre) == 0) {
            t = time(NULL);
            ct = ctime(&t);
        } else {
            return "ERR_NOSUCHSERVER\n";
        }
    }


    return ct;
}

const char *version(ServerIRC *servidor, char target[50]) {
    char *cadena;
    struct utsname patch;
    int un = uname(&patch);
    cadena = malloc(sizeof (char)*1000);
    fflush(stdin);
    fflush(stdout);
    if (target == NULL) {
        strcat(cadena, ":");
        strcat(cadena, servidor->nombre);
        strcat(cadena, "\nSystem: ");
        strcat(cadena, patch.sysname);
        strcat(cadena, "\nName: ");
        strcat(cadena, patch.nodename);
        strcat(cadena, "\nRelease: ");
        strcat(cadena, patch.release);
        strcat(cadena, "\nVersion: ");
        strcat(cadena, patch.version);
        strcat(cadena, "\nMachine Architecture: ");
        strcat(cadena, patch.machine);
        strcat(cadena, "\n");
    } else {
        if (strcmp(target, servidor->nombre) == 0) {
            strcat(cadena, ":");
            strcat(cadena, servidor->nombre);
            strcat(cadena, "\nSystem: ");
            strcat(cadena, patch.sysname);
            strcat(cadena, "\nName: ");
            strcat(cadena, patch.nodename);
            strcat(cadena, "\nRelease: ");
            strcat(cadena, patch.release);
            strcat(cadena, "\nVersion: ");
            strcat(cadena, patch.version);
            strcat(cadena, "\nMachine Architecture: ");
            strcat(cadena, patch.machine);
            strcat(cadena, "\n");
        } else {
            return "ERR_NOSUCHSERVER\n";
        }
    }
    return cadena;
}

void quit(char *argumentos[5], ClientIRC *cl, ServerIRC *server) {
    Nodo *nTmp, *nTmp2;
    CanalIRC *cTmp;
    ClientIRC *irc;
    Lista *canales=server->canales;
    for (nTmp = canales->header; nTmp != NULL; nTmp = nTmp->next) {
        cTmp = nTmp->cliente;
        if (ListaRemove(cTmp->clientes, cl, (void*) ClientIRCxId) != NULL) {
            //irc=ListaRemove(server->clientes, cl, (void*) ClientIRCxId);
            for (nTmp2 = cTmp->clientes->header; nTmp2 != NULL; nTmp2 = nTmp2->next) {
                irc = nTmp2->cliente;
                if(argumentos[1]!=NULL){
                    strcat(argumentos[1], "\n");
                    write(irc->socket, argumentos[1], strlen(argumentos[1]));
                }                
            }
        }
    }
    close(cl->socket);
}

const char *motd(ServerIRC *servidor, char target[50]) {
    char *cadena;
    cadena = malloc(sizeof (char)*1000);
    fflush(stdin);
    fflush(stdout);
    if (target == NULL) {
        strcat(cadena, ":");
        strcat(cadena, servidor->nombre);
        strcat(cadena, " ");
        strcat(cadena, "Penas, es lo que siento en mi alma... si me va menos de 60 en operativos\n");
    } else {
        if (strcmp(target, servidor->nombre) == 0) {
            strcat(cadena, ":");
            strcat(cadena, servidor->nombre);
            strcat(cadena, " ");
            strcat(cadena, "Penas, es lo que siento en mi alma... si me va menos de 60 en operativos\n");
        } else {
            return "ERR_NOSUCHSERVER\n";
        }
    }
    return cadena;
}

void list(ServerIRC *servidor, char *argumentos[5], ClientIRC *cl) {
    char *cadena;
    Nodo *nTmp1;
    CanalIRC *cTmp1;
    char *canalArg;

    char numero[3];
    if (argumentos[1] == NULL && argumentos[2] == NULL) {
        cadena = malloc(sizeof (char)*1000);
        strcat(cadena, ":");
        strcat(cadena, servidor->nombre);
        strcat(cadena, " ");
        for (nTmp1 = servidor->canales->header; nTmp1 != NULL; nTmp1 = nTmp1->next) {
            cTmp1 = nTmp1->cliente;
            strcat(cadena, cTmp1->nombre);
            strcat(cadena, " ");
            sprintf(numero, "%d", ListaSize(cTmp1->clientes));
            strcat(cadena, numero);
            strcat(cadena, " ");
            strcat(cadena, cTmp1->topic);
            strcat(cadena, " - ");
        }
        strcat(cadena, "\n");
        write(cl->socket, cadena, strlen(cadena));
    } else if (argumentos[2] == NULL) {        
        if (argumentos[1][strlen(argumentos[1])-1] != ',') {
            canalArg = strtok(argumentos[1], ",");
            cadena = malloc(sizeof (char)*1000);
            strcat(cadena, ":");
            strcat(cadena, servidor->nombre);
            strcat(cadena, " ");
            do {
                fflush(stdin);
                fflush(stdout);
                if (canalArg != NULL) {
                    cTmp1 = ListaGet(servidor->canales, CanalIRCNew(canalArg, "Oli"), (void*) CanalCmpxNombre);
                    if (cTmp1 != NULL) {
                        strcat(cadena, cTmp1->nombre);
                        strcat(cadena, " ");
                        sprintf(numero, "%d", ListaSize(cTmp1->clientes));
                        strcat(cadena, numero);
                        strcat(cadena, " ");
                        strcat(cadena, cTmp1->topic);
                        strcat(cadena, " - ");
                    }
                }
                canalArg = strtok(NULL, ",");
            } while (canalArg != NULL);
            strcat(cadena, "\n");
            write(cl->socket, cadena, strlen(cadena));
        } else {
            write(cl->socket, "SYNTAX ERROR\n", 13);
        }
    }else if(argumentos[2]!=NULL){
        if(strcmp(argumentos[2],servidor->nombre)==0){
            if (argumentos[1][strlen(argumentos[1])-1] != ',') {
            canalArg = strtok(argumentos[1], ",");
            cadena = malloc(sizeof (char)*1000);
            strcat(cadena, ":");
            strcat(cadena, servidor->nombre);
            strcat(cadena, " ");
            do {
                fflush(stdin);
                fflush(stdout);
                if (canalArg != NULL) {
                    cTmp1 = ListaGet(servidor->canales, CanalIRCNew(canalArg, "Oli"), (void*) CanalCmpxNombre);
                    if (cTmp1 != NULL) {
                        strcat(cadena, cTmp1->nombre);
                        strcat(cadena, " ");
                        sprintf(numero, "%d", ListaSize(cTmp1->clientes));
                        strcat(cadena, numero);
                        strcat(cadena, " ");
                        strcat(cadena, cTmp1->topic);
                        strcat(cadena, " - ");
                    }
                }
                canalArg = strtok(NULL, ",");
            } while (canalArg != NULL);
            strcat(cadena, "\n");
            write(cl->socket, cadena, strlen(cadena));
        } else {
            write(cl->socket, "SYNTAX ERROR\n", 13);
        }
        }else{
            write(cl->socket, "ERR_NOSUCHSERVER\n", 17);
        }
    }
}
void user(ServerIRC *server, char *argumentos[5],ClientIRC *cl){
    ClientIRC *cTmp;
    char a,b;
    if (argumentos[1] == NULL || argumentos[2] == NULL||argumentos[3] == NULL || argumentos[4] == NULL){
        write(cl->socket,"ERR_NEEDMOREPARAMS\n",19);
    }else if(strcmp(argumentos[3],server->nombre)!=0){
        write(cl->socket, "ERR_NOSUCHSERVER\n", 17);
    }else if(argumentos[4][0]=='\"'&&argumentos[4][strlen(argumentos[4])-1]=='\"'){
        a=argumentos[4][0];
        b=argumentos[4][strlen(argumentos[4])-1];
        cTmp=newClientIRC(1,1);
        strcpy(cTmp->nickname,argumentos[1]);       
        if(ListaGet(server->clientes,cTmp,(void *)ClientIRCxUser)==NULL){
            strcpy(cl->nickname,argumentos[1]);
            strcpy(cl->hostName,argumentos[2]);            
            write(cl->socket, "Valido\n", 7);
        }else{
            write(cl->socket, "ERR_NICKNAMEINUSE\n", 18);
        }
    }else{
        write(cl->socket, "SYNTAX ERROR\n", 13);
        a=argumentos[4][0];
        b=argumentos[4][strlen(argumentos[4])-1];
    }
}