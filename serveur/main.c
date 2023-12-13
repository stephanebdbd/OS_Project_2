/**
 * Code du serveur 
 * .
 **/

#include <stdio.h>
#include <inttypes.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "../commun/commun.h"
#include <asm-generic/socket.h>
#include "imgdist.h"

void ExempleSignaux(void);

int main(int argc, char* argv[]){

   struct image* librairie[3][34];
   
   FILE *listing = popen("./list-file ./img/", "r");
   if (listing == NULL) {
      perror("Erreur lors de l'ouverture du processus");
      exit(EXIT_FAILURE);
   }
   int i=0, j=0;
   
   while ((fgets(librairie[i][j]->chemin, sizeof(librairie[i][j]->chemin), listing) != NULL) || (j==32 && i == 2)){
      if (!PHash(librairie[i][j]->chemin, &librairie[i][j]->hash))
         return 0;
      
      j++;
   }

   pclose(listing);

   int server_fd = checked(socket(AF_INET, SOCK_STREAM, 0));
   
   int opt = 1;
   setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
   
   struct sockaddr_in address;   //Adresse du serveur du socket
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(5555);
   
   
   checked(bind(server_fd, (struct sockaddr *)&address, sizeof(address))); //Lie l'adresse au socket
   
   checked(listen(server_fd, 10)); // mise en écoute de l'utilisateur
   
   
   size_t addrlen = sizeof(address);
   int new_socket = checked(accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen));
   
   int lu;
   // pthread_t t1, t2, t3;
   
   struct image* meilleure_image;
   struct image* client;
   while ((lu = read(new_socket, client->chemin, 1024)) > 0) {
      meilleure_image->distance = 64;
      printf("Le chemin est %s\n", client->chemin);
      client->chemin[strlen(client->chemin)-1] = '\0';
      // On regarde si l'image est en dessous de 20ko ensuite on commence le processus
      /*if (!PHash(client->chemin, &client->hash))
         return 0;
      pthread_create(&t1, NULL, compare_image, (void*)&uneImage);
      pthread_create(&t2, NULL, compare_image, (void*)&uneImage);
      pthread_create(&t3, NULL, compare_image, (void*)&uneImage);
      pthread_join(t1, NULL);
      pthread_join(t2, NULL);
      pthread_join(t3, NULL);
      checked_wr(write(new_socket, meilleure_image, lu) < 0);*/
   }
   close(server_fd);
   close(new_socket);
   ExempleSignaux();
   return 0;
}


static volatile sig_atomic_t signalRecu = 0;
void SignalHandler(int sig) {
   signalRecu = 1;
}

void ExempleSignaux(void) {
   
   /// Exemple gestion de signaux (cf Annexe de l'énoncé & corrigé du projet 1) ///
   
   // Forcer l'interruption des appels systèmes lors de la réception de SIGINT
   struct sigaction action;
   action.sa_handler = SignalHandler;
   sigemptyset(&action.sa_mask);

   if (sigaction(SIGINT, &action, NULL) < 0) {
      perror("sigaction()");
      return;
   }
   
   
   // Gestion idéale (court et sans risque d'accès concurrents) d'un signal
   // (cf SignalHandler() également).
   printf("Signal recu : %d.\n", signalRecu);
   raise(SIGINT);
   printf("Signal recu : %d.\n", signalRecu);
   
   
   // Bloquer des signaux pour le thread courant
   sigset_t set;
    
   sigemptyset(&set);        // Ensemble vide de signaux
   sigaddset(&set, SIGINT);  // Ajouter le signal SIGINT
   sigaddset(&set, SIGUSR1); // Ajouter le signal SIGUSR1
    
   if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0) {
      perror("pthread_sigmask()");
      return;
   }
   
   /// ///
}