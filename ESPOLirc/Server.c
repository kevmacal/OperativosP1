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
#include <pthread.h>

#include "ServerIRC.h"
#include "Lista.h"
#include "ClientIRC.h"
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

void limpiarCadena(char cadena[2000]) {
    int i;
    for (i = 0; i < 2000; i++) {
        cadena[i] = '\0';
    }
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
    Lista *canales = server->canales;
    for (nTmp = canales->header; nTmp != NULL; nTmp = nTmp->next) {
        cTmp = nTmp->cliente;
        if (ListaRemove(cTmp->clientes, cl, (void*) ClientIRCxId) != NULL) {
            //irc=ListaRemove(server->clientes, cl, (void*) ClientIRCxId);
            if (cl->isConectado == 1) {
                ClientIRCConectar(cl, 0);
                for (nTmp2 = cTmp->clientes->header; nTmp2 != NULL; nTmp2 = nTmp2->next) {
                    irc = nTmp2->cliente;
                    if (argumentos[1] != NULL && irc->isConectado == 1) {
                        char cad[2000];
                        limpiarCadena(cad);
                        strcat(cad, cl->nickname);
                        strcat(cad, ": Has quit - ");
                        strcat(cad, argumentos[1]);
                        strcat(cad, "\n");
                        write(irc->socket, cad, strlen(cad));
                    }
                }
            }
        }
    }
    close(cl->socket);
    Hilo *tmp=ListaGet(server->hilos,HiloNew(cl->idLista),(void*)HiloxId);
    pthread_cancel(tmp->hilo);
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
            numero[0] = '\0';
            numero[1] = '\0';
            numero[2] = '\0';
            sprintf(numero, "%d", ListaSize(cTmp1->clientes));
            strcat(cadena, numero);
            strcat(cadena, " ");
            strcat(cadena, cTmp1->topic);
            strcat(cadena, "\n");
        }
        write(cl->socket, cadena, strlen(cadena));
    } else if (argumentos[2] == NULL) {
        if (argumentos[1][strlen(argumentos[1]) - 1] != ',') {
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
                        numero[0] = '\0';
                        numero[1] = '\0';
                        numero[2] = '\0';
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
    } else if (argumentos[2] != NULL) {
        if (strcmp(argumentos[2], servidor->nombre) == 0) {
            if (argumentos[1][strlen(argumentos[1]) - 1] != ',') {
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
                            numero[0] = '\0';
                            numero[1] = '\0';
                            numero[2] = '\0';
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
        } else {
            write(cl->socket, "ERR_NOSUCHSERVER\n", 17);
        }
    }
}

void user(ServerIRC *server, char *argumentos[5], ClientIRC *cl) {
    ClientIRC *cTmp;
    if (argumentos[1] == NULL || argumentos[2] == NULL || argumentos[3] == NULL || argumentos[4] == NULL) {
        write(cl->socket, "ERR_NEEDMOREPARAMS\n", 19);
    } else if (strcmp(argumentos[3], server->nombre) != 0) {
        write(cl->socket, "ERR_NOSUCHSERVER\n", 17);
    } else if (argumentos[4][0] == '\"' && argumentos[4][strlen(argumentos[4]) - 1] == '\"') {
        cTmp = newClientIRC(1, 1);
        if (strlen(argumentos[1]) < 10) {
            strcpy(cTmp->nickname, argumentos[1]);
            cTmp = ListaGet(server->clientes, cTmp, (void *) ClientIRCxUser);
            if (cTmp == NULL) {
                strcpy(cl->nickname, argumentos[1]);
                strcpy(cl->hostName, argumentos[2]);
                argumentos[4][0] = ' ';
                argumentos[4][strlen(argumentos[4]) - 1] = ' ';
                strcpy(cl->realName, argumentos[4]);
                ClientIRCConectar(cl, 1);
                //ListaReplaceAll(server->clientes,cl,(void *)ClientIRCxSock);
                write(cl->socket, "Valido\n", 7);
            } else {
                if (ClienteIRCVerificarConexion(server->clientes, cTmp) == 1) {
                    strcpy(cl->nickname, argumentos[1]);
                    strcpy(cl->hostName, argumentos[2]);
                    argumentos[4][0] = ' ';
                    argumentos[4][strlen(argumentos[4]) - 1] = ' ';
                    strcpy(cl->realName, argumentos[4]);
                    ClientIRCConectar(cl, 1);
                    //ListaReplaceAll(server->clientes,cl,(void *)ClientIRCxSock);
                    write(cl->socket, "Valido\n", 7);
                } else {
                    write(cl->socket, "ERR_NICKNAMEINUSE\n", 18);
                }
            }
        } else {
            write(cl->socket, "ERR_ERRONEUSNICKNAME\n", 21);
        }
    } else {
        write(cl->socket, "SYNTAX ERROR\n", 13);
    }
}

void users(ServerIRC *servidor, char *argumentos[5], ClientIRC *cl) {
    Nodo *it;
    ClientIRC *cc;
    if (argumentos[1] != NULL) {
        if (strcmp(argumentos[1], servidor->nombre) == 0) {
            for (it = servidor->clientes->header; it != NULL; it = it->next) {
                char cad[2000];
                limpiarCadena(cad);
                cc = it->cliente;
                if (cc->isConectado) {
                    strcat(cad, ":");
                    strcat(cad, servidor->nombre);
                    strcat(cad, " ");
                    strcat(cad, cc->nickname);
                    strcat(cad, " - ");
                    strcat(cad, cc->realName);
                    strcat(cad, "\n");
                    write(cl->socket, cad, strlen(cad));
                }
            }
        } else {
            write(cl->socket, "ERR_NOSUCHSERVER\n", 17);
        }
    } else {
        for (it = servidor->clientes->header; it != NULL; it = it->next) {
            char cad[2000];
            limpiarCadena(cad);
            cc = it->cliente;
            if (cc->isConectado == 1) {
                strcat(cad, ":");
                strcat(cad, servidor->nombre);
                strcat(cad, " ");
                strcat(cad, cc->nickname);
                strcat(cad, " - ");
                strcat(cad, cc->realName);
                strcat(cad, "\n");
                write(cl->socket, cad, strlen(cad));
            }
        }
    }
}

void setname(ServerIRC *server, char *argumentos[5], ClientIRC *cl) {
    if (argumentos[0] == NULL || argumentos[1] == NULL) {
        write(cl->socket, "ERR_NEEDMOREPARAMS\n", 19);
    } else if (argumentos[1][0] == '\"' && argumentos[1][strlen(argumentos[1]) - 1] == '\"') {
        char cad[2000];
        limpiarCadena(cad);
        argumentos[1][0] = ' ';
        argumentos[1][strlen(argumentos[1]) - 1] = ' ';
        strcpy(cl->realName, argumentos[1]);
        strcat(cad, ":");
        strcat(cad, server->nombre);
        strcat(cad, " REALNAME ");
        strcat(cad, cl->realName);
        strcat(cad, "\n");
        write(cl->socket, cad, strlen(cad));
    } else {
        write(cl->socket, "SYNTAX ERROR\n", 13);
    }
}

void nick(ServerIRC *server, char *argumentos[5], ClientIRC *cl) {
    ClientIRC *cTmp;
    cTmp = newClientIRC(1, 1);
    strcpy(cTmp->nickname, argumentos[1]);
    cTmp = ListaGet(server->clientes, cTmp, (void *) ClientIRCxUser);
    if (cTmp == NULL) {
        char cad[2000];
        limpiarCadena(cad);
        if (strlen(argumentos[1]) < 10) {
            ClientIRCResetUsername(cl);
            strcpy(cl->nickname, argumentos[1]);
            strcat(cad, ":");
            strcat(cad, server->nombre);
            strcat(cad, " NICK ");
            strcat(cad, cl->nickname);
            strcat(cad, "\n");
            write(cl->socket, cad, strlen(cad));
        } else {
            write(cl->socket, "ERR_ERRONEUSNICKNAME\n", 21);
        }
    } else {
        if (cTmp->isConectado == 0) {
            char cad[2000];
            limpiarCadena(cad);
            if (strlen(argumentos[1]) < 10) {
                ClientIRCResetUsername(cl);
                strcpy(cl->nickname, argumentos[1]);
                strcat(cad, ":");
                strcat(cad, server->nombre);
                strcat(cad, " NICK ");
                strcat(cad, cl->nickname);
                strcat(cad, "\n");
                write(cl->socket, cad, strlen(cad));
            } else {
                write(cl->socket, "ERR_ERRONEUSNICKNAME\n", 21);
            }
        } else {
            write(cl->socket, "SYNTAX ERROR\n", 13);
        }
    }
}

void privmsg(ServerIRC *server, char *argumentos[5], ClientIRC *cl) {
    char a;
    CanalIRC *cTmp;
    Nodo *nTmp, *nTmp2;
    ClientIRC *clTmp;
    a = argumentos[1][0];
    if (a == '#') {
        cTmp = ListaGet(server->canales, CanalIRCNew(argumentos[1], ":*"), (void *) CanalCmpxNombre);
        if (cTmp != NULL && argumentos[2][0] == '\"' && argumentos[2][strlen(argumentos[2]) - 1] == '\"') {
            for (nTmp = cTmp->clientes->header; nTmp != NULL; nTmp = nTmp->next) {
                clTmp = nTmp->cliente;
                if (clTmp != cl && clTmp->isConectado == 1) {
                    char cad[2000];
                    limpiarCadena(cad);
                    strcat(cad, cl->nickname);
                    strcat(cad, ":");
                    argumentos[2][0] = ' ';
                    argumentos[2][strlen(argumentos[2]) - 1] = ' ';
                    strcat(cad, argumentos[2]);
                    strcat(cad, "\n");
                    write(clTmp->socket, cad, strlen(cad));
                }
            }
        } else {
            if (cTmp == NULL) {
                write(cl->socket, "ERR_CANNOTSENDTOCHAN\n", 21);
            } else {
                write(cl->socket, "SYNTAX ERROR\n", 13);
            }
        }
    } else {
        clTmp = newClientIRC(0, 0);
        strcpy(clTmp->nickname, argumentos[1]);
        clTmp = ListaGet(server->clientes, clTmp, (void *) ClientIRCxUserConnected);
        if (clTmp != NULL) {
            if (clTmp != cl) {
                char cad[2000];
                limpiarCadena(cad);
                strcat(cad, cl->nickname);
                strcat(cad, ":");
                argumentos[2][0] = ' ';
                argumentos[2][strlen(argumentos[2]) - 1] = ' ';
                strcat(cad, argumentos[2]);
                strcat(cad, "\n");
                write(clTmp->socket, cad, strlen(cad));
            }
        } else {
            write(cl->socket, "ERR_NOSUCHNICK\n", 15);
        }
    }
}

void join(ServerIRC *server, char *argumentos[5], ClientIRC *cl) {
    Lista *lCanBusq = ListaNew();
    Lista *lTop = ListaNew();
    char *canales, *topics;
    Nodo *nTmp, *nTmp2;
    CanalIRC *cTmp, *cTmp2;
    CanalIRC *cEnc1;
    CanalIRC *cCreated;

    if (argumentos[0] != NULL && argumentos[1] != NULL) {
        canales = strtok(argumentos[1], ",");
        ListaAdd(lCanBusq, NodoNew(CanalIRCNew(canales, "topic")));
        do {
            canales = strtok(NULL, ",");
            if (canales != NULL) {
                ListaAdd(lCanBusq, NodoNew(CanalIRCNew(canales, "topic")));
            }
        } while (canales != NULL);
        if (argumentos[2] != NULL) {
            topics = strtok(argumentos[2], ",");
            ListaAdd(lTop, NodoNew(CanalIRCNew("canal", topics)));
            do {
                topics = strtok(NULL, ",");
                if (topics != NULL) {
                    ListaAdd(lTop, NodoNew(CanalIRCNew("canal", topics)));
                }
            } while (topics != NULL);
        }
        nTmp2 = lTop->header;
        for (nTmp = lCanBusq->header; nTmp != NULL; nTmp = nTmp->next) {
            if (nTmp2 != NULL) {
                cTmp2 = nTmp2->cliente;
            } else {
                cTmp2 = NULL;
            }
            cTmp = nTmp->cliente;
            if (cTmp->nombre[0] == '#') {
                cEnc1 = ListaGet(server->canales, cTmp, (void *) CanalCmpxNombre);
                if (cEnc1 != NULL) {
                    if (cTmp2 != NULL) {
                        if (strcmp(cTmp2->topic, cEnc1->topic) == 0 && ListaGet(cEnc1->clientes, cl, (void*) ClientIRCxUser) == NULL) {
                            ListaAdd(cEnc1->clientes, NodoNew(cl));
                            write(cl->socket, "Agregado\n", 9);
                        }
                    } else {
                        if (strlen(cEnc1->topic) == 0 && ListaGet(cEnc1->clientes, cl, (void*) ClientIRCxUser) == NULL) {
                            ListaAdd(cEnc1->clientes, NodoNew(cl));
                            write(cl->socket, "Agregado\n", 9);
                        }
                    }
                } else {
                    if (cTmp2 != NULL) {
                        cCreated = CanalIRCNew(cTmp->nombre, cTmp2->topic);
                        ListaAdd(server->canales, NodoNew(cCreated));
                        ListaAdd(cCreated->clientes, NodoNew(cl));
                        write(cl->socket, "Canal Agregado\n", 15);
                    } else {
                        cCreated = CanalIRCNew(cTmp->nombre, "");
                        ListaAdd(server->canales, NodoNew(cCreated));
                        ListaAdd(cCreated->clientes, NodoNew(cl));
                        write(cl->socket, "Canal Agregado\n", 15);

                    }
                }
            }
            if (nTmp2 != NULL) {
                nTmp2 = nTmp2->next;
            }
        }
    }
}

void part(ServerIRC *server, char *argumentos[5], ClientIRC *cl) {
    CanalIRC *cTmp, *cEnc1;
    ClientIRC *ccTmp;
    Nodo *nTmp, *nTmp2;
    char *canales;
    Lista *lCanBusq = ListaNew();
    int exito;

    exito = 0;
    if (argumentos[1] != NULL && argumentos[2] != NULL) {
        canales = strtok(argumentos[1], ",");
        ListaAdd(lCanBusq, NodoNew(CanalIRCNew(canales, "topic")));
        do {
            canales = strtok(NULL, ",");
            if (canales != NULL) {
                ListaAdd(lCanBusq, NodoNew(CanalIRCNew(canales, "topic")));
            }
        } while (canales != NULL);
        for (nTmp = lCanBusq->header; nTmp != NULL; nTmp = nTmp->next) {
            cTmp = nTmp->cliente;
            if (cTmp->nombre[0] == '#') {
                cEnc1 = ListaGet(server->canales, cTmp, (void *) CanalCmpxNombre);
                if (cEnc1 != NULL) {
                    if (ListaRemove(cEnc1->clientes, cl, (void *) ClientIRCxId) != NULL) {
                        exito = 1;
                        for (nTmp2 = cEnc1->clientes->header; nTmp2 != NULL; nTmp2 = nTmp2->next) {
                            ccTmp = nTmp2->cliente;
                            if (argumentos[2][0] == '\"' && argumentos[2][strlen(argumentos[2]) - 1] == '\"') {
                                char cad[2000];
                                limpiarCadena(cad);
                                strcat(cad, cl->nickname);
                                strcat(cad, ":");
                                argumentos[2][0] = ' ';
                                argumentos[2][strlen(argumentos[2]) - 1] = ' ';
                                strcat(cad, argumentos[2]);
                                strcat(cad, "\n");
                                write(ccTmp->socket, cad, strlen(cad));
                            } else {
                                ListaAdd(cEnc1->clientes, NodoNew(cl));
                                write(cl->socket, "SYNTAX ERROR\n", 13);
                            }
                        }
                    } else {
                        write(cl->socket, "ERR_NOTONCHANNEL\n", 17);
                    }
                } else {
                    write(cl->socket, "ERR_NOSUCHCHANNEL\n", 18);
                }
            }
        }
    }
    if (exito == 1) {
        write(cl->socket, "SUCCESS\n", 7);
    }
}

void names(ServerIRC *server, char *argumentos[5], ClientIRC *cl) {
    Lista *lCanBusq = ListaNew();
    Nodo *nTmp, *nTmp2;
    CanalIRC *cTmp;
    CanalIRC *cEnc1;
    ClientIRC *clTmp;
    char *canales;
    char numero[3];


    if (argumentos[1] != NULL) {
        canales = strtok(argumentos[1], ",");
        ListaAdd(lCanBusq, NodoNew(CanalIRCNew(canales, "topic")));
        do {
            canales = strtok(NULL, ",");
            if (canales != NULL) {
                ListaAdd(lCanBusq, NodoNew(CanalIRCNew(canales, "topic")));
            }
        } while (canales != NULL);
        if (argumentos[2] != NULL) {
            if (strcmp(server->nombre, argumentos[2]) == 0) {
                for (nTmp = lCanBusq->header; nTmp != NULL; nTmp = nTmp->next) {
                    cTmp = nTmp->cliente;
                    cEnc1 = ListaGet(server->canales, cTmp, (void *) CanalCmpxNombre);
                    if (cEnc1 != NULL) {
                        char cad[2000];
                        limpiarCadena(cad);
                        strcat(cad, ":");
                        strcat(cad, server->nombre);
                        strcat(cad, " ");
                        strcat(cad, cEnc1->nombre);
                        strcat(cad, " Conectados: ");
                        numero[0] = '\0';
                        numero[1] = '\0';
                        numero[2] = '\0';
                        sprintf(numero, "%d", ListaSize(cEnc1->clientes));
                        strcat(cad, numero);
                        strcat(cad, "\n");
                        for (nTmp2 = cEnc1->clientes->header; nTmp2 != NULL; nTmp2 = nTmp2->next) {
                            clTmp = nTmp2->cliente;
                            strcat(cad, clTmp->nickname);
                            if (nTmp2->next != NULL) {
                                strcat(cad, ", ");
                            }
                        }
                        strcat(cad, "\n");
                        write(cl->socket, cad, strlen(cad));
                    } else {
                        write(cl->socket, "ERR_NOSUCHCHANNEL\n", 18);
                    }
                }
            } else {
                write(cl->socket, "ERR_NOSUCHSERVER\n", 17);
            }
        } else {
            //Aqui funciona con Server1 (Por defecto Nuestro servidor)
            for (nTmp = lCanBusq->header; nTmp != NULL; nTmp = nTmp->next) {
                cTmp = nTmp->cliente;
                cEnc1 = ListaGet(server->canales, cTmp, (void *) CanalCmpxNombre);
                if (cEnc1 != NULL) {
                    char cad[2000];
                    limpiarCadena(cad);
                    strcat(cad, ":");
                    strcat(cad, server->nombre);
                    strcat(cad, " ");
                    strcat(cad, cEnc1->nombre);
                    strcat(cad, " Conectados: ");
                    numero[0] = '\0';
                    numero[1] = '\0';
                    numero[2] = '\0';
                    sprintf(numero, "%d", ListaSize(cEnc1->clientes));
                    strcat(cad, numero);
                    strcat(cad, "\n");
                    for (nTmp2 = cEnc1->clientes->header; nTmp2 != NULL; nTmp2 = nTmp2->next) {
                        clTmp = nTmp2->cliente;
                        strcat(cad, clTmp->nickname);
                        if (nTmp2->next != NULL) {
                            strcat(cad, ", ");
                        }
                    }
                    strcat(cad, "\n");
                    write(cl->socket, cad, strlen(cad));
                } else {
                    write(cl->socket, "ERR_NOSUCHCHANNEL\n", 18);
                }
            }
        }
    } else {
        /*Aqui sin parametros busca en todos los canales*/
        for (nTmp = server->canales->header; nTmp != NULL; nTmp = nTmp->next) {
            cEnc1 = nTmp->cliente;
            char cad[2000];
            limpiarCadena(cad);
            strcat(cad, ":");
            strcat(cad, server->nombre);
            strcat(cad, " ");
            strcat(cad, cEnc1->nombre);
            strcat(cad, " Conectados: ");
            numero[0] = '\0';
            numero[1] = '\0';
            numero[2] = '\0';
            sprintf(numero, "%d", ListaSize(cEnc1->clientes));
            strcat(cad, numero);
            strcat(cad, "\n");
            for (nTmp2 = cEnc1->clientes->header; nTmp2 != NULL; nTmp2 = nTmp2->next) {
                clTmp = nTmp2->cliente;
                strcat(cad, clTmp->nickname);
                if (nTmp2->next != NULL) {
                    strcat(cad, ", ");
                }
            }
            strcat(cad, "\n");
            write(cl->socket, cad, strlen(cad));
        }
    }
}