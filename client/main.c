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
   if (fichierBMP == NULL)
      return 0;
   fseek(fichierBMP, 0, SEEK_END);
   client->taille = ftell(fichierBMP);
   if (client->taille > 20.000 * 1024)
      return 0;
   rewind(fichierBMP);
   if (fread(client->contenuImage, 1, client->taille, fichierBMP) == client->taille)
      return 1;
   return 0;
}


void clientListener(int sock){
   struct client_data client_data;
   client_data.chemins_longueur = 0;
   while ((fgets(client_data.client.chemin, sizeof(client_data.client.chemin), stdin) != NULL) && (client_data.chemins_longueur < MAX_CHEMINS_LONGUEUR)){
      client_data.client.chemin[strlen(client_data.client.chemin)-1] = '\0';
      if ((LectureImageBMP(&client_data.client))){
         client_data.chemins_longueur += strlen(client_data.client.chemin);
         if (client_data.chemins_longueur >= MAX_CHEMINS_LONGUEUR){
            printf("No similar image found (no comparison could be performed successfully).\n");
            break;
         }
         checked_wr(write(sock, &client_data.client, sizeof(struct client)));
         if ((read(sock, &client_data.meilleure_image, sizeof(struct image)) == -1) || (client_data.meilleure_image.distance >= 64))
            printf("No similar image found (no comparison could be performed successfully).\n");
         else
            printf("Most similar image found: '%s' with a distance of %d.\n", client_data.meilleure_image.chemin, client_data.meilleure_image.distance);
      }
      else
         printf("No similar image found (no comparison could be performed successfully).\n");
   }
}

int main(){
   int sock = checked(socket(AF_INET, SOCK_STREAM, 0));
   struct sockaddr_in address;   //Adresse du serveur du socket
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = INADDR_ANY;
      address.sin_port = htons(5555);
   inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
   checked(connect(sock, (struct sockaddr *)&address, sizeof(address)));
   clientListener(sock);
   close(sock);
   return 0;
}