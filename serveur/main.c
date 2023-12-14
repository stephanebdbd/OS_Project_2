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

struct image meilleure_image;

void ExempleSignaux(void);

void* compare_image(void *ptr) {
   struct to_compare_image *to_compare = (struct to_compare_image*)ptr;
      for (int j = 0; j < 34; j++) {
         sleep(rand() % 3 + 1);
         unsigned int distance = DistancePHash(meilleure_image.hash, to_compare->librairie[j].hash);
         if (distance < meilleure_image.distance) {
            meilleure_image.distance = distance;
            strcpy(meilleure_image.chemin, to_compare->librairie[j].chemin);
         }
      }
   return NULL;
}



int main(){
   struct image client;
   struct to_compare_image to_compare[3];
   FILE *listing = popen("./list-file ./img", "r");
   if (listing == NULL) {
      perror("Erreur lors de l'ouverture du processus");
      exit(EXIT_FAILURE);
   }
   int i=0, j=0;
   while ((fgets(to_compare[i].librairie[j].chemin, sizeof(to_compare[i].librairie[j].chemin), listing) != NULL)){
      to_compare[i].librairie[j].chemin[strlen(to_compare[i].librairie[j].chemin)-1] = '\0';
      if (!PHash(to_compare[i].librairie[j].chemin, &to_compare[i].librairie[j].hash))
         return 1;
      i += (j == 33) ? 1 : 0;  
      j = (j == 33) ? 0 : j+1;
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
   pthread_t t1, t2, t3;
   int longueur = 0;
   printf("1\n");
   while (((lu = read(new_socket, client.chemin, 1024)) > 0) && (longueur < 1000)) {
      printf("2\n");
      meilleure_image.distance = 64;
      printf("3\n");
      printf("Le chemin est %s\n", client.chemin);
      int index = strlen(client.chemin)-1;
      longueur += index;
      client.chemin[index] = '\0';
      if (longueur >= 1000)
         break;
      if (!PHash(client.chemin, &client.hash))
         return 0;
      for (int i=0; i < 3; i++)
         to_compare[i].client = client;
      pthread_create(&t1, NULL, compare_image, (void*)&to_compare[0]);
      pthread_create(&t2, NULL, compare_image, (void*)&to_compare[1]);
      pthread_create(&t3, NULL, compare_image, (void*)&to_compare[2]);
      pthread_join(t1, NULL);
      pthread_join(t2, NULL);
      pthread_join(t3, NULL);
      checked_wr(write(new_socket, &meilleure_image, sizeof(meilleure_image)) < 0);
   }
   close(server_fd);
   close(new_socket);
   ExempleSignaux();
   return 0;
}


static volatile sig_atomic_t signalRecu = 0;
void SignalHandler(int sig) {
   signalRecu = sig;
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