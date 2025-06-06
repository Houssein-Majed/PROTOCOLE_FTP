#ifndef __FTP_STRUCTURES_H__
#define __FTP_STRUCTURES_H__

#include "csapp.h"

#define NB_SLAVES   3               /* Nombre statique d'esclaves */
#define NB_PROC      4               /* Nombre de processus dans le pool */
#define MASTER_PORT  2121            /* Port d'écoute du serveur maitre */   
#define SLAVE_PORT   21212           /* Port de base pour les serveur esclaves */
#define MAX_NAME_LEN 265             /* Taille max du nom de fichier */
#define BLOCK_SIZE   8000            /* Taille du buffer pour le transfert */

/* Codes de retour */
#define SUCCESS             0
#define ERR_INVALID_REQUEST 1
#define ERR_FILE_NOT_FOUND  2

/* Définition de l'énumération pour les types de requête */
typedef enum {
    GET = 1, /* 1 pour GET */
    PUT = 2, /* 2 pour PUT */
    LS  = 3,  /* 3 pour LS */
    BYE = 4,  /* 4 pour BYE */
    REGET = 5 /* 5 pour REGET */
} typereq_t;

/* Structure de la requête client */
typedef struct {
    typereq_t type;
    char filename[MAX_NAME_LEN];
} request_t;

/* Structure de la réponse du serveur */
typedef struct {
    int code;        /* 0 = succès, autre valeur = code d'erreur */
    int filesize;    /* Taille du fichier en octets (valide si code == 0) */
} response_t;

typedef struct {
    char ip[INET_ADDRSTRLEN];
    int port;
    int current_load; //nombre de clients déjà connectés
} slave_info_t;

extern pid_t children[NB_PROC];
extern char data_dir[500];  // Répertoire de données utilisé par esclave

#endif /*__FTP_STRUCTURES_H__ */
