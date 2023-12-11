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

struct image {
   char imagePath[1024];
   char chemin[1000]; 
   int distance;
   uint64_t imageHash;  
   uint64_t hash2;  
   int Bestdistance;
   char Bestchemin[1000];
   };


void* compare_image(void *ptr) {
   struct image *toute_img_comp = (struct image*)ptr;

   toute_img_comp->distance = DistancePHash(toute_img_comp->imageHash, toute_img_comp->hash2);

   printf("La distance entre les images est : %u\n", toute_img_comp->distance);
   if (toute_img_comp->distance < toute_img_comp-> Bestdistance){
      toute_img_comp->Bestdistance=toute_img_comp->distance;
      strcpy(toute_img_comp->Bestchemin,toute_img_comp->chemin);
   }
   return NULL;
}







int main(int argc, char* argv[]){
    
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
   
   struct image uneImage;
   uneImage.Bestdistance=78;
   
   while ((lu = read(new_socket, uneImage.imagePath, 1024)) > 0) {
      printf("Je rentre dans le serveur.\n");
      printf("Le chemin est %s\n", uneImage.imagePath);
      
      if (!PHash(uneImage.imagePath, &uneImage.imageHash))
         return 1;

      FILE *listing = popen("./list-file ./img/", "r");
      if (listing == NULL) {
         perror("Erreur lors de l'ouverture du processus");
         exit(EXIT_FAILURE);
      }

      int taille =0;
      while (fgets(uneImage.chemin, sizeof(uneImage.chemin), listing) != NULL) {
         uneImage.chemin[strlen(uneImage.chemin)-1] = '\0';
         printf("Le chemin de l'image est : %s\n", uneImage.chemin);   //il faut encore ajouter chaque line à imagesBank

         if (!PHash(uneImage.chemin, &uneImage.hash2))
            return 1;

         if (taille % 3 == 0) {
               pthread_create(&t1, NULL,compare_image, (void*)&uneImage);
               pthread_join(t1, NULL);
         }else if (taille % 3 == 1) {
               pthread_create(&t2, NULL, compare_image, (void*)&uneImage);
               pthread_join(t2, NULL);
         }else {
               pthread_create(&t3, NULL, compare_image, (void*)&uneImage);
               pthread_join(t3, NULL);
         }
         taille++;
         
      }
      
      pclose(listing);
      ExempleSignaux();



      checked_wr(write(new_socket, uneImage.imagePath, lu) < 0);
      

      
      // Calcule du code de hachage perceptif de l'image reçue via le socket par le client et
      // conservation de celui-ci dans imageHash.
      
   }
   close(server_fd);
   close(new_socket);




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