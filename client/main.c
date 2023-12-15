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


void lectureSocket(int socketss){
   struct image meilleurImage;
   long unsigned int i;
   int ret;
   i = 0;
   while (i < sizeof(meilleurImage)){
      ret = read(socketss, &meilleurImage, sizeof(meilleurImage) - i);
      if (ret <= 0) {
         if (ret < 0)
            perror("read");
         else{
            printf("Déconnexion du serveur.\n");
            exit(0);
         }
      }
      i += ret;
   }
   if (meilleurImage.distance < 64)
      printf("Most similar image found: '%s' with a distance of %d.", meilleurImage.chemin, meilleurImage.distance);
   else
      printf("No similar image found (no comparison could be performed successfully).");
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
   int* nombre_de_client = (int*)arg;
   int socketss = checked(socket(AF_INET, SOCK_STREAM, 0));
   struct sockaddr_in address;
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(5555);
   inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
   if (checked(connect(socketss, (struct sockaddr*)&address, sizeof(address)))){
      perror("Erreur lors de la création du thread");
      exit(0);
   }
   nombre_de_client++;
   char chemin[1000];
   while (fgets(chemin, sizeof(chemin), stdin) != NULL) {
      chemin[strlen(chemin) - 1] = '\0';
      if (tailleImageCheck(chemin)){
         printf("Envoi...\n");
         checked_wr(write(socketss, chemin, strlen(chemin) + 1));
         lectureSocket(socketss);
      }
   }
   close(socketss);
   return NULL;
}

int main() {
   int nombre_de_client = 0;
   pthread_t threads[MAX_CLIENTS];
   while (nombre_de_client < MAX_CLIENTS)
      pthread_create(&threads[nombre_de_client], NULL, client_socket, (void*)&nombre_de_client);
   for (int i = 0; i < nombre_de_client; ++i)
      pthread_join(threads[i], NULL);
   return 0;
}