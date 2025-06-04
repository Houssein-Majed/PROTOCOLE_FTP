/* FTP_Slave.c */
#include "csapp.h"
#include "FTP_Structures.h"
#include "FTP_Service.h"
#include "Signal_Handler_Server.h"

char data_dir[500];  // Répertoire de données utilisé par cet esclave
pid_t children[NB_PROC]; // tableau des processus fils  

int main(int argc, char **argv) {

    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    int i;
    pid_t pid;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);

    // Définition du répertoire de stockage en fonction du port attribué
    if (port == 21212){
        strcpy(data_dir, "./server1/");
    }else if(port == 21213){
        strcpy(data_dir, "./server2/");
    }else if (port == 21214){
        strcpy(data_dir, "./server3/");
    }else{
        fprintf(stderr, "Port non valide.\n");
        exit(1);
    }

    Signal(SIGINT, sigint_handler);

    listenfd = Open_listenfd(port);
    printf("Serveur esclave en écoute sur le port %d\n", port);

    // Création d'un pool de NB_PROC processus
    for (i = 0; i < NB_PROC; i++) { 
        if ((pid = Fork()) == 0) {  /* Processus fils : sortie de la boucle de création */
            Signal(SIGINT, SIG_DFL);  // Rétablir le comportement par défaut pour SIGINT
            break;
        } else {       /* Processus père : stockage du PID du fils */
            children[i] = pid;
        }
    }

    if (i < NB_PROC) {  /* Code exécuté par un processus fils */
        while (1) {
            clientlen = sizeof(clientaddr); //car chaque fils peut avoir different @IP du server slave
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            if (connfd != -1) {
                ftp_service(connfd, &clientaddr);  // Traite la requête du client
                Close(connfd);
            }
            // En cas d'erreur sur Accept, on recommence la boucle
        }
    } else {  /* Code exécuté par le processus père */
        pause();
    }
    return 0;
}
