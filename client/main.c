#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h> // Added for exit()
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "../commun/commun.h"

#define MAX_CLIENTS 1001

struct le_client {
    int client_sockets[MAX_CLIENTS];
    int nombre_de_client;
};


void lectureSocket(socketss){
   struct image* meilleurImage;
   int i, ret;
   i = 0;
   while (i < sizeof(meilleurImage)){
      ret = read(socketss, meilleurImage, sizeof(meilleurImage) - i);
      if (ret <= 0) {
         if (ret < 0)
            perror("read");
         else
            printf("Déconnexion du serveur.\n");
         exit;
      }
      i += ret;
   }
   printf("Most similar image found: %s with a distance of %d.", meilleurImage->chemin, meilleurImage->distance);
;

}


bool tailleImageCheck(char chemin[1000]){
   FILE *file = fopen(chemin, "rb");
   if(file == NULL) {
      perror("Erreur lors de la lecture de l'image");
      return 1;
   }
   fseek(file, 0, SEEK_END);
   long fileSize = ftell(file);
   fclose(file);
   if(fileSize > 20 * 1024) { 
      printf("La taille de l'image dépasse 20 ko.\n");
      return false;
   }
   else{
      return true;
   }
}



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
   int i, ret;
   struct image* meilleurImage;

   while (fgets(chemin, sizeof(chemin), stdin) != NULL) {
      chemin[strlen(chemin) - 1] = '\0';
      if (tailleImageCheck(chemin)){
         printf("Envoi...\n");
         checked_wr(write(socketss, chemin, strlen(chemin) + 1));
      }
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