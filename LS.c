#include "FTP_Service.h"

// Déclaration externe de la variable data_dir, définie dans FTP_Slave.c
extern char data_dir[];

/**
 * Exécute la commande "ls" sur le répertoire défini par data_dir et envoie le résultat au client.
 *
 * La fonction construit dynamiquement la commande ls en utilisant la variable data_dir,
 * lit toute la sortie dans un tampon dynamique, puis envoie d'abord la taille totale en octets,
 * suivie du contenu lui-même.
 *
 * Le descripteur de fichier de la connexion client.
 */
void send_ls(int connfd) {
    FILE *fp;
    char buffer[BLOCK_SIZE];
    size_t total_size = 0;
    size_t capacity = BLOCK_SIZE;
    char cmd[1024];  // Commande à exécuter

    // Construire la commande "ls" en utilisant le répertoire spécifique
    snprintf(cmd, sizeof(cmd), "ls %s", data_dir);

    char *output = malloc(capacity);
    if (output == NULL) {
        int error = -1;
        Rio_writen(connfd, &error, sizeof(int));
        return;
    }
    
    /* Exécuter la commande ls sur le répertoire spécifié */
    if ((fp = popen(cmd, "r")) == NULL) { 
        fprintf(stderr, "Erreur lors de l'exécution de %s\n", cmd);
        int error = -1;
        Rio_writen(connfd, &error, sizeof(int));
        free(output);
        return;
    }
    
    /* Lecture de la sortie de ls dans le tampon dynamique */
    size_t n;
    while ((n = fread(buffer, 1, BLOCK_SIZE, fp)) > 0) {
        if (total_size + n > capacity) {
            capacity = (total_size + n) * 2;
            char *temp = realloc(output, capacity);
            if (temp == NULL) {
                fprintf(stderr, "Erreur d'allocation mémoire\n");
                int error = -1;
                Rio_writen(connfd, &error, sizeof(int));
                free(output);
                pclose(fp);
                return;
            }
            output = temp;
        }
        memcpy(output + total_size, buffer, n);
        total_size += n;
    }
    pclose(fp);
    
    /* Envoi de la taille du résultat */
    int size_int = (int)total_size;  // On suppose que total_size tient dans un int
    Rio_writen(connfd, &size_int, sizeof(int));
    
    /* Envoi du contenu du ls */
    if (total_size > 0)
        Rio_writen(connfd, output, total_size);
    
    free(output);
}
