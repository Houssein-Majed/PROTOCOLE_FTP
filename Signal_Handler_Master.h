#ifndef __SIGNAL_HANDLER_Master_H__
#define __SIGNAL_HANDLER_Master_H__

#include "FTP_Structures.h"

/**
 * Gestionnaire du signal SIGINT côté maître.
 *
 * Cette fonction est déclenchée lorsque le serveur maître reçoit un signal SIGINT 
 * Pour nous afficher qu'il est mort
 */
void sigint_handler_master(int sig);

#endif
