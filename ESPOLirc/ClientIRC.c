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

#include "ClientIRC.h"

ClientIRC *newClientIRC(int id) {
    ClientIRC *nuevo = malloc(sizeof (ClientIRC));
    nuevo->idArreglo = id;
    //nuevo->msj = "";
    return nuevo;
}

//http://stackoverflow.com/questions/3381080/reading-all-content-from-a-text-file-c
//falta lo del patch, lo del target server y el argumento

const char *info() {

    char *fcontent = NULL;
    int fsize = 0;
    FILE *fp;

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
}

const char *timee(){

    time_t t;
    char* ct;

    t = time(NULL);
    ct = ctime(&t);
  
    return ct;
}