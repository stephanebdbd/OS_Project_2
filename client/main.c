#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "../commun/commun.h"
#include <asm-generic/socket.h>


int main(){
   sleep(10);
   int sock = checked(socket(AF_INET, SOCK_STREAM, 0));

   struct sockaddr_in address;   //Adresse du serveur du socket
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = INADDR_ANY;
      address.sin_port = htons(5555);

   inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
   checked(connect(sock, (struct sockaddr *)&address, sizeof(address)));

   char chemin[1000];
   struct image meilleure_image;


   while (fgets(chemin, sizeof(chemin), stdin) != NULL){
      chemin[strlen(chemin)-1] = '\0';    
      checked_wr(write(sock, chemin, strlen(chemin)+1));
      if (read(sock, &meilleure_image, sizeof(struct image)) == -1)
         perror("Erreur lors de la lecture de la meilleure image");
      if (meilleure_image.distance < 64)
         printf("Most similar image found: '%s' with a distance of %d.\n", meilleure_image.chemin, meilleure_image.distance);
      else
         printf("No similar image found (no comparison could be performed successfully).\n");
   }
   close(sock);
   return 0;
}