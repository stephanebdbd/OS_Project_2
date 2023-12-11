#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "../commun/commun.h"

void client_socket(int client_Sock) {
    client_Sock = checked(socket(AF_INET, SOCK_STREAM, 0));
    printf("ca passe");
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(5555);
    
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    
    checked(connect(client_Sock, (struct sockaddr *)&address, sizeof(address)));
    
    char chemin[1001];

    while (fgets(chemin, sizeof(chemin), stdin) != NULL) {
        chemin[strlen(chemin) - 1] = '\0';
        printf("Envoi...\n");
        checked_wr(write(client_Sock, chemin, strlen(chemin) + 1));
    }

    close(client_Sock);
    return ;
}

int main() {
   const int MAX_CLIENTS = 1001;
   pthread_t threads; 
   int client_sockets[MAX_CLIENTS];
   int nombre_de_client = 0;
   printf("ca passe");
   while (nombre_de_client < MAX_CLIENTS) {
      printf("ca passe");
      if (pthread_create(&threads, NULL, client_socket, (void*)&client_sockets[nombre_de_client]) != 0) {
         perror("Erreur lors de la création du thread");
         close(client_sockets[nombre_de_client]);
      } else {
         nombre_de_client++;
      }
   }

   for (int i = 0; i < nombre_de_client; ++i) {
      pthread_join(threads, NULL);
   }

   return 0;
}