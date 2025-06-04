#include "Signal_Handler_Master.h"


slave_info_t slaves[NB_SLAVES]; // Tableau contenant les informations des serveurs esclaves

int main() {
    
    int next_slave = 0; // Indice pour l'algorithme de répartition Round-Robin
    Signal(SIGINT, sigint_handler_master);

    // Initialisation des informations des esclaves
    for (int i = 0; i < NB_SLAVES; i++) {
        strncpy(slaves[i].ip, "127.0.0.1", INET_ADDRSTRLEN); // Adresse locale pour tous les esclaves   
        slaves[i].port = SLAVE_PORT + i;  // Chaque esclave utilise un port différent (ex: 21212, 21213...)
        slaves[i].current_load = 0; // Initialisation de la charge des esclaves à 0
    }

    // Vérifier la disponibilité des serveurs esclaves
    for (int i = 0; i < NB_SLAVES; i++) {
        int fd = Open_clientfd(slaves[i].ip, slaves[i].port);
        if (fd < 0) {
            fprintf(stderr, "Erreur connexion à l'esclave %d (%s:%d)\n", i, slaves[i].ip, slaves[i].port);
            exit(1);
        }
        printf("Connecté à l'esclave %d (%s:%d)\n", i, slaves[i].ip, slaves[i].port);
        Close(fd);
    }
    printf("Tous les serveurs esclaves sont disponibles.\n");

    // Le maître écoute sur MASTER_PORT pour les connexions clients
    int listenfd = Open_listenfd(MASTER_PORT);
    printf("Serveur maître en écoute sur le port %d\n", MASTER_PORT);

    while (1) {
        int connfd;
        struct sockaddr_in clientaddr;
        socklen_t clientlen = sizeof(clientaddr);

        // Acceptation d'une nouvelle connexion client
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // Sélection en Round-Robin d'un serveur esclave
        slave_info_t chosen = slaves[next_slave];
        next_slave = (next_slave + 1) % NB_SLAVES;

        // Envoi de l'information au client :
        // D'abord la longueur de l'IP, puis l'IP elle-même, et enfin le port (entier).
        int ip_len = strlen(chosen.ip);
        Rio_writen(connfd, &ip_len, sizeof(int));
        Rio_writen(connfd, chosen.ip, ip_len);
        Rio_writen(connfd, &chosen.port, sizeof(int));

        printf("Redirection d'un client vers l'esclave %s:%d\n", chosen.ip, chosen.port);

        // Fermeture de la connexion avec le client après envoi des informations
        Close(connfd);
    }
    return 0;
}
