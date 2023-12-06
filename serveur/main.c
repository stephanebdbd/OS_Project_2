/**
 * Code du serveur 
 * .
 **/

#include <stdio.h>
#include <inttypes.h>
#include <signal.h>
#include "imgdist.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../commun/commun.h"
#include <asm-generic/socket.h>



void ExempleSignaux(void);


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
  
   char imagePath[1024];
   int lu;
   uint64_t imageHash, hash2;


   while ((lu = read(new_socket, imagePath, 1024)) > 0) {
      printf("Je rentre dans le serveur.\n");
      printf("Le chemin est %s\n", imagePath);


      // Calcule du code de hachage perceptif de l'image reçue via le socket par le client et
      // conservation de celui-ci dans imageHash.
      if (!PHash(imagePath, &imageHash))
         return 1; // Échec dans le chargement de l'image (message sur stderr automatique)

        // Idem pour "img/2.bmp".
      if (!PHash("img/2.bmp", &hash2))
         return 1; 

      

      // Calculer la distance entre hash1 et hash2
      unsigned int distance = DistancePHash(imageHash, hash2); 

      // Afficher la distance entre les deux images (valeur de retour d'img-dist dans le projet 1
      // quand il n'y avait pas d'erreur).
      printf("La distance entre les images est : %d\n", distance);
   
      /// Exemple gestion de signaux (cf Annexe de l'énoncé & corrigé du projet 1) ///
   
      ExempleSignaux();
   
      /// ///


      checked_wr(write(new_socket, imagePath, lu) < 0);
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
   #include <asm-generic/signal-defs.h>
   #include <asm/signal.h>
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
