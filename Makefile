# Nom du compilateur
CC = gcc
# Options de compilation
CFLAGS = -Wall
# Bibliothèques à lier
LIBS = -lpthread

# Noms des exécutables
CLIENT = FTP_Client
MASTER = FTP_Master
SLAVE = FTP_Slaves

# Fichiers sources pour chaque programme
CLIENT_SRCS = FTP_Client.c csapp.c Signal_Handler_Client.c FTP_Request.c FTP_Log.c
MASTER_SRCS = FTP_Master.c csapp.c Signal_Handler_Master.c 
SLAVE_SRCS = FTP_Slaves.c csapp.c Signal_Handler_Server.c FTP_Service.c Transfert_Fichier.c LS.c

# Fichiers objets générés
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
MASTER_OBJS = $(MASTER_SRCS:.c=.o)
SLAVE_OBJS = $(SLAVE_SRCS:.c=.o)

# Règle principale
all: $(CLIENT) $(MASTER) $(SLAVE)

# Compilation du client
$(CLIENT): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_OBJS) $(LIBS)

# Compilation du serveur maître
$(MASTER): $(MASTER_OBJS)
	$(CC) $(CFLAGS) -o $(MASTER) $(MASTER_OBJS) $(LIBS)

# Compilation du serveur esclave
$(SLAVE): $(SLAVE_OBJS)
	$(CC) $(CFLAGS) -o $(SLAVE) $(SLAVE_OBJS) $(LIBS)

# Règle de compilation générique pour les fichiers .c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers compilés
clean:
	rm -f $(CLIENT_OBJS) $(MASTER_OBJS) $(SLAVE_OBJS) $(CLIENT) $(MASTER) $(SLAVE)
