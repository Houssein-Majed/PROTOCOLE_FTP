#ifndef __FTP_SERVICE_H__
#define __FTP_SERVICE_H__

#include "Transfert_Fichier.h"

/**
 * Gère une session FTP avec un client connecté.
 *
 * Cette fonction prend en charge une connexion avec un client, traite les requêtes FTP
 * envoyées par ce client (GET, REGET, BYE) et exécute les opérations correspondantes
 * (envoi complet ou reprise de transfert d'un fichier). La connexion reste ouverte
 * jusqu'à la réception d'une requête BYE ou en cas d'erreur de communication.
 *
 * parametres:  connfd Descripteur de fichier de la socket connectée au client.
 *              clientaddr Structure contenant les informations réseau du client connecté.
 */
void ftp_service(int connfd, struct sockaddr_in *clientaddr);

/**
 * Exécute la commande "ls" et envoie le contenu du répertoire courant au client.
 * 
 * Cette fonction utilise popen pour exécuter la commande "ls" et lire la sortie
 * générée (la liste des fichiers et dossiers dans le répertoire serveur). Le contenu
 * est ensuite envoyé au client via le socket identifié par connfd, en découpant la
 * transmission en blocs de taille BLOCK_SIZE.
 * 
 * En cas d'erreur lors de l'ouverture du pipe avec popen, la fonction envoie un code
 * d'erreur (-1) au client.
 * 
 * Le descripteur de fichier de la connexion avec le client.
 */
void send_ls(int connfd);


#endif /* __FTP_SERVICE_H__ */