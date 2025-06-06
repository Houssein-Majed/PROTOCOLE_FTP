#include "FTP_Request.h"

#include "FTP_Request.h"

/* Analyse la ligne de commande saisie par l'utilisateur. */
int parse_command(char *command_line, request_t *req) {
    char cmd[1024], fname[MAX_NAME_LEN];
    int nb_args = sscanf(command_line, "%s %s", cmd, fname);
    if (nb_args < 1) {
        fprintf(stderr, "Commande invalide\n");
        return -1;
    }
    
    /* Validation du type de commande */
    if (strcasecmp(cmd, "bye") == 0) {
        req->type = BYE;
    } else if (strcasecmp(cmd, "get") == 0 && nb_args == 2) {
        req->type = GET;
    } else if (strcasecmp(cmd, "reget") == 0 && nb_args == 2) {
        req->type = REGET;
    } else if (strcasecmp(cmd, "ls") == 0) {
        req->type = LS;
    } else {
        fprintf(stderr, "Commande non supportée. Usage: get <filename>, reget <filename>, ls ou bye\n");
        return -1;
    }
    
    /* Copie du nom de fichier pour les commandes GET/REGET */
    if (req->type != BYE && req->type != LS) {
        strncpy(req->filename, fname, MAX_NAME_LEN);
        req->filename[MAX_NAME_LEN - 1] = '\0';
    }
    return 0;
}

/* Envoie la requête au serveur.
   Pour REGET, envoie aussi l'offset enregistré dans le log. */
void send_request(int clientfd, request_t *req) {
    int type = req->type;
    Rio_writen(clientfd, &type, sizeof(int));
    
    /* Pour GET et REGET, on envoie ensuite le nom du fichier */
    if (type == GET || type == REGET) {
        int filename_size = strlen(req->filename);
        Rio_writen(clientfd, &filename_size, sizeof(int));
        Rio_writen(clientfd, req->filename, filename_size);
        
        /* Cas particulier de REGET */
        if (type == REGET) {
            int offset = get_offset_from_log(req->filename);
            Rio_writen(clientfd, &offset, sizeof(int));
            printf("Reget : reprise du téléchargement à partir de l'offset %d\n", offset);
        }
    }
}

/* Réception pour un transfert complet (GET).
   Lit la taille totale envoyée par le serveur et télécharge le fichier.
   Retourne le nombre total d'octets téléchargés ou -1 en cas d'erreur. */
int receive_file_get(int clientfd, request_t *req) {
    /* Réception de la taille du fichier */
    int file_size;
    if (Rio_readn(clientfd, &file_size, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "Erreur lecture de la taille du fichier.\n");
        return -1;
    }
    if (file_size < 0) {
        fprintf(stderr, "Erreur serveur : fichier non trouvé ou autre erreur.\n");
        return -1;
    }

    /* Préparation du fichier de destination */
    char client_file_path[MAX_NAME_LEN + 10];
    snprintf(client_file_path, sizeof(client_file_path), "./client/%s", req->filename);
    int fd = Open(client_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    /* Réception par blocs */
    char buffer[BLOCK_SIZE];
    int total_received = 0;

    while (total_received < file_size) {
        int to_read = (file_size - total_received > BLOCK_SIZE ? BLOCK_SIZE : file_size - total_received);
        int n = Rio_readn(clientfd, buffer, to_read);
        if (n <= 0) {
            fprintf(stderr, "Erreur ou fin de réception prématurée, n = %d\n", n);
            break;
        }
        Rio_writen(fd, buffer, n);
        total_received += n;
        update_log(req->filename, total_received);
    }
    Close(fd);
    return total_received;
}

/* Réception pour une reprise (REGET). */
int receive_file_reget(int clientfd, request_t *req) {
    /* Réception de la taille restante */
    int file_size;
    if (Rio_readn(clientfd, &file_size, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "Erreur lecture de la taille restante du fichier.\n");
        return -1;
    }
    if (file_size < 0) {
        fprintf(stderr, "Erreur serveur : fichier non trouvé ou autre erreur.\n");
        return -1;
    }
    /* Récupération de l'offset précédent */
    int offset = get_offset_from_log(req->filename);

    char client_file_path[MAX_NAME_LEN + 10];
    snprintf(client_file_path, sizeof(client_file_path), "./client/%s", req->filename);
    int fd = Open(client_file_path, O_WRONLY | O_CREAT, 0644);
    lseek(fd, offset, SEEK_SET);

    /* Réception des données manquantes */
    char buffer[BLOCK_SIZE];
    int new_received = 0;

    while (new_received < file_size) {
        int to_read = (file_size - new_received > BLOCK_SIZE ? BLOCK_SIZE : file_size - new_received);
        int n = Rio_readn(clientfd, buffer, to_read);
        if (n <= 0) {
            fprintf(stderr, "Erreur ou fin de réception prématurée, n = %d\n", n);
            break;
        }
        Rio_writen(fd, buffer, n);
        new_received += n;
        update_log(req->filename, offset + new_received);
    }
    Close(fd);
    printf("Reget terminé, total téléchargé = %d octets\n", offset + new_received);
    return offset + new_received;
}

/* Traite le transfert en appelant la fonction adéquate selon le type de requête. */
int process_transfer(int clientfd, request_t *req) {
    if (req->type == GET)
        return receive_file_get(clientfd, req);
    else if (req->type == REGET)
        return receive_file_reget(clientfd, req);
    else
        return -1;
}
