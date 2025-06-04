#include "Signal_Handler_Client.h"
#include "FTP_Request.h"

int main(int argc, char **argv) {
    int masterfd,clientfd; // Descripteurs de fichiers pour les connexions réseau
    char *master_host; // Adresse du serveur maître
    char command_line[500]; // Stockage de la commande saisie par l'utilisateur
    request_t req; // Structure pour stocker les requêtes du client
    struct timeval start, end;  // Variables pour mesurer le temps d'exécution
    double elapsed_time;  // Temps écoulé pour le transfert de fichiers

    // Vérification du nombre d'arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_host>\n", argv[0]);
        exit(1);
    }
    master_host = argv[1]; // Récupération de l'adresse du serveur maître

    // Connexion au maître pour obtenir les infos d'un esclave
    masterfd = Open_clientfd(master_host, MASTER_PORT);
    int ip_len;
    Rio_readn(masterfd, &ip_len, sizeof(int)); // Lecture de la longueur de l'IP
    char slave_ip[INET_ADDRSTRLEN];
    Rio_readn(masterfd, slave_ip, ip_len); // Lecture de l'adresse IP de l'esclave
    slave_ip[ip_len] = '\0';
    int slave_port;
    Rio_readn(masterfd, &slave_port, sizeof(int)); // Lecture du port de l'esclave
    Close(masterfd); // Fermeture de la connexion avec le maître

    printf("Connecté au maître. Redirigé vers l'esclave %s:%d\n", slave_ip, slave_port);

    // Connexion au serveur esclave choisi
    clientfd = Open_clientfd(slave_ip, slave_port);
    signal(SIGPIPE, sigpipe_handler);

    while (1) {
        printf("ftp> ");
        if (Fgets(command_line, sizeof(command_line), stdin) == NULL) {
            fprintf(stderr, "Erreur lecture de la commande.\n");
            break;
        }

        if (parse_command(command_line, &req) != 0)
            continue;

        if (req.type == BYE) {
            Rio_writen(clientfd, &req.type, sizeof(int));
            printf("Déconnexion.\n");
            break;
        }

        /* Traitement spécifique pour la commande LS */
        if (req.type == LS) {
            send_request(clientfd, &req);
            
            int ls_size;
            if (Rio_readn(clientfd, &ls_size, sizeof(int)) != sizeof(int)) {
                fprintf(stderr, "Erreur lecture de la taille du ls\n");
                Close(clientfd);
                continue;
            }
            if (ls_size < 0) {
                fprintf(stderr, "Erreur serveur lors de ls\n");
                Close(clientfd);
                continue;
            }
            
            char *ls_output = malloc(ls_size + 1);
            if (ls_output == NULL) {
                fprintf(stderr, "Erreur d'allocation mémoire pour ls\n");
                Close(clientfd);
                continue;
            }
            if (Rio_readn(clientfd, ls_output, ls_size) != ls_size) {
                fprintf(stderr, "Erreur lecture du contenu ls\n");
                free(ls_output);
                Close(clientfd);
                continue;
            }
            ls_output[ls_size] = '\0';
            printf("%s\n", ls_output);
            free(ls_output);
            continue;
        }

        /* Pour GET et REGET : envoi de la requête, transfert et chronométrage */
        send_request(clientfd, &req);

        // Début du chronométrage du transfert
        gettimeofday(&start, NULL);
        int downloaded = process_transfer(clientfd, &req);
        gettimeofday(&end, NULL);

        if(downloaded > 0) {
            printf("Transfert du fichier avec succes.\n");
        }

        // Calcul du temps écoulé
        elapsed_time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);

        /* Pour GET, une fois le transfert complet, effacer le log */
        if (req.type == GET) {
            char log_filename[MAX_NAME_LEN + 10];
            snprintf(log_filename, sizeof(log_filename), "%s.log", req.filename);
            remove(log_filename);
        }

        // Affichage des statistiques de téléchargement
        printf("%d bytes received in %.2f seconds (%.2f Kbytes/s).\n",
               downloaded, elapsed_time, (downloaded / 1024.0) / elapsed_time);
    }

    // Fermeture de la connexion avec le serveur esclave
    Close(clientfd);
    return 0;
}
