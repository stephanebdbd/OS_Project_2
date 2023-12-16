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

static volatile sig_atomic_t signalRecu = 0;
struct image meilleure_image;

void ExempleSignaux(void);

void* compare_image(void *ptr) {
   struct to_compare_image* to_compare = (struct to_compare_image*)ptr;
   for (int j = 0; j < to_compare->longueur; j++) {
      sleep(0);
      int result = DistancePHash(to_compare->client.hash, to_compare->librairie[j].hash);
      if (result < meilleure_image.distance) {
         meilleure_image.distance = result;
         strcpy(meilleure_image.chemin, to_compare->librairie[j].chemin);
      }
   }
   return NULL;
}

int getPictures(struct to_compare_image* to_compare){
   FILE *listing = popen("./list-file img", "r");
   if (listing == NULL) {
      perror("Erreur lors de l'ouverture du processus");
      exit(EXIT_FAILURE);
   }
   int i=0;
   for (int j=0; j < 3; j++)
      to_compare[j].longueur = 0;
   while ((fgets(to_compare[i].librairie[to_compare[i].longueur].chemin, sizeof(to_compare[i].librairie[to_compare[i].longueur].chemin), listing) != NULL)){
      to_compare[i].librairie[to_compare[i].longueur].chemin[strlen(to_compare[i].librairie[to_compare[i].longueur].chemin)-1] = '\0';
      if (!PHash(to_compare[i].librairie[to_compare[i].longueur].chemin, &to_compare[i].librairie[to_compare[i].longueur].hash))
         return 0;
      to_compare[i].longueur++;
      i += (to_compare[i].longueur == 34) ? 1 : 0;
   }
   pclose(listing);
   return 1;
}

struct sockaddr_in create_socket(int* server_fd){
      *server_fd = checked(socket(AF_INET, SOCK_STREAM, 0));
      int opt = 1;
      setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
      struct sockaddr_in address;   
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = INADDR_ANY;
      address.sin_port = htons(5555);
      checked(bind(*server_fd, (struct sockaddr *)&address, sizeof(address))); 
      checked(listen(*server_fd, 10)); 
      return address;
      
}

void* connetToClient(void *arg) {
      struct socket_for_client*  sfc = (struct socket_for_client*) arg;
      struct client client;
      pthread_t t1, t2, t3;
      int lu;
      while ((lu = read(sfc->new_sock, &client, sizeof(struct client))) > 0) {
         meilleure_image.distance = 64;
         if (PHashRaw(client.contenuImage, client.taille, &client.hash)){
            for (int i=0; i < 3; i++)
               sfc->to_compare[i].client = client;
            pthread_create(&t1, NULL, compare_image, (void*)&sfc->to_compare[0]);
            pthread_create(&t2, NULL, compare_image, (void*)&sfc->to_compare[1]);
            pthread_create(&t3, NULL, compare_image, (void*)&sfc->to_compare[2]);
            pthread_join(t1, NULL);
            pthread_join(t2, NULL);
            pthread_join(t3, NULL);
         }
         checked_wr(write(sfc->new_sock, &meilleure_image, sizeof(meilleure_image)));
      }
      return NULL;
}

int main(){
   struct to_compare_image to_compare[3];
   if (!getPictures(to_compare))
      return EXIT_FAILURE;
   int server_fd, new_socket;
   struct sockaddr_in address=create_socket(&server_fd);
   size_t addrlen = sizeof(address);
   int clientfds[10];
   for(int i = 0; i < 10; i++){
      clientfds[i] = -1;
   }
   struct socket_for_client sfc;
   for(int j=0; j<3; j++)
      sfc.to_compare[j]=to_compare[j];
   while (1) {
      int new_socket = checked(accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen));
      if (new_socket == -1) {
         perror("accept");
         continue;
      }
      int i;
      for (i = 0; i < 10; i++) {
         if (clientfds[i] == -1) {
            clientfds[i] = new_socket;
            break;
         }
      }
      if (i < 10) {
         pthread_t thread;
         sfc.new_sock=new_socket;
         if (pthread_create(&thread, NULL, connetToClient, (void*)&sfc) != 0) {
               perror("pthread_create");
               close(new_socket);
         }
      } else {
         int nb_clients = 0;
         for (int j = 0; j < 10; j++) {
            if (clientfds[j] != -1) {
               nb_clients++;
            }
         }
         if (nb_clients == 1000) {
            close(new_socket);
         } 
      }
   }
   close(server_fd);
   close(new_socket);
   ExempleSignaux();
   return 0;
}

void SignalHandler(int sig) {
   signalRecu = sig;
}

void ExempleSignaux(void) {
   struct sigaction action;
   action.sa_handler = SignalHandler;
   sigemptyset(&action.sa_mask);
   if (sigaction(SIGINT, &action, NULL) < 0) {
      perror("sigaction()");
      return;
   }
   raise(SIGINT);
   sigset_t set;
   sigemptyset(&set);        
   sigaddset(&set, SIGINT);  
   sigaddset(&set, SIGUSR1); 
   if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0) {
      perror("pthread_sigmask()");
      return;
   }
}