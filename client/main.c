#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "../commun/commun.h"
#include <asm-generic/socket.h>

int LectureImageBMP(struct client *client){
   FILE *fichierBMP = fopen(client->chemin, "rb");
   if (fichierBMP == NULL) {
      perror("Erreur lors de l'ouverture du fichier");
      return 0;
   }
   fseek(fichierBMP, 0, SEEK_END);
   client->taille = ftell(fichierBMP);
   if (client->taille > 20.000 * 1024) {
      perror("Le fichier est trop volumineux");
      return 0;
   }
   rewind(fichierBMP);
   if (fread(client->contenuImage, 1, client->taille, fichierBMP) == client->taille)
      return 1;
   perror("Erreur lors de la lecture du fichier");
   return 0;
}


void clientListener(int sock){
   struct image meilleure_image;
   struct client client;
   while (fgets(client.chemin, sizeof(client.chemin), stdin) != NULL){
      client.chemin[strlen(client.chemin)-1] = '\0';
      if (LectureImageBMP(&client)){
         checked_wr(write(sock, &client, sizeof(struct client)));
         if (read(sock, &meilleure_image, sizeof(struct image)) == -1)
            perror("Erreur lors de la lecture de la meilleure image");
         if (meilleure_image.distance < 64)
            printf("Most similar image found: '%s' with a distance of %d.\n", meilleure_image.chemin, meilleure_image.distance);
         else
            printf("No similar image found (no comparison could be performed successfully).\n");
      }
      else
         printf("No similar image found (no comparison could be performed successfully).\n");
   }
}

void socket_search(int *sock){
   *sock = checked(socket(AF_INET, SOCK_STREAM, 0));
   struct sockaddr_in address;   //Adresse du serveur du socket
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = INADDR_ANY;
      address.sin_port = htons(5555);
   inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
   checked(connect(*sock, (struct sockaddr *)&address, sizeof(address)));
}

int main(){
   int sock;
   socket_search(&sock);
   clientListener(sock);
   close(sock);
   return 0;
}