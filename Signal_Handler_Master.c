#include "Signal_Handler_Master.h"

void sigint_handler_master(int sig) {
    printf("\nSIGINT reçu Par le Master.\n");
    exit(0);
}
