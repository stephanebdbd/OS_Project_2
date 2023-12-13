#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h> // Added for exit()
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "../commun/commun.h"

#define MAX_CLIENTS 1001

struct le_client {
    int client_sockets[MAX_CLIENTS];
    int nombre_de_client;
};



void* client_socket(void* arg) {
   struct le_client* sock = (struct le_client*)arg;

   int socketss = checked(socket(AF_INET, SOCK_STREAM, 0));

   struct sockaddr_in address;
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(5555);
   inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
   checked(connect(socketss, (struct sockaddr*)&address, sizeof(address)));

   char chemin[1000];

   while (fgets(chemin, sizeof(chemin), stdin) != NULL) {
      chemin[strlen(chemin) - 1] = '\0';
      printf("Envoi...\n");
      checked_wr(write(socketss, chemin, strlen(chemin) + 1));
   }

   close(socketss);
   return NULL;
}

int main() {
   pthread_t threads[MAX_CLIENTS];
   struct le_client user;
   user.nombre_de_client = 0;

   while (user.nombre_de_client < MAX_CLIENTS) {
      if (pthread_create(&threads[user.nombre_de_client], NULL, client_socket, (void*)&user) != 0) {
         perror("Erreur lors de la création du thread");
         close(user.client_sockets[user.nombre_de_client]);
      } else {
         user.nombre_de_client++;
      }
   }

   for (int i = 0; i < user.nombre_de_client; ++i) {
      pthread_join(threads[i], NULL);
   }

   return 0;
}