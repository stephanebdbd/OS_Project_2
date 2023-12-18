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
#include <semaphore.h>
#include "../commun/commun.h"
#include <asm-generic/socket.h>
#include "imgdist.h"

static volatile sig_atomic_t signalRecu = 0;
struct image meilleure_image;
pthread_mutex_t mutex_compare;
pthread_mutex_t mutex_client;
sem_t client_semahpore;


void ExempleSignaux(void);

void* compare_image(void *ptr) {
   struct to_compare_image* to_compare = (struct to_compare_image*)ptr;
   for (int j = 0; j < to_compare->amount_images; j++) {
      // printf("Current thread id: %lu\n", pthread_self());
      pthread_mutex_lock(&mutex_compare);
      int result = DistancePHash(to_compare->client.hash, to_compare->librairie[j].hash);
      if (result < meilleure_image.distance) {
         meilleure_image.distance = result;
         strcpy(meilleure_image.chemin, to_compare->librairie[j].chemin);
      }
      pthread_mutex_unlock(&mutex_compare);
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
      to_compare[j].amount_images = 0;
   while ((fgets(to_compare[i].librairie[to_compare[i].amount_images].chemin, sizeof(to_compare[i].librairie[to_compare[i].amount_images].chemin), listing) != NULL)){
      to_compare[i].librairie[to_compare[i].amount_images].chemin[strlen(to_compare[i].librairie[to_compare[i].amount_images].chemin)-1] = '\0';
      if (!PHash(to_compare[i].librairie[to_compare[i].amount_images].chemin, &to_compare[i].librairie[to_compare[i].amount_images].hash))
         return 0;
      to_compare[i].amount_images++;
      i += (to_compare[i].amount_images == 34) ? 1 : 0;
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

void* serveClient(void *arg) {
   struct socket_for_client* sfc = (struct socket_for_client*) arg;
   struct client client;
   pthread_t t1, t2, t3;
   int lu;
   while ((lu = read(sfc->new_sock, &client, sizeof(struct client))) > 0) {
      pthread_mutex_lock(&mutex_client);
      meilleure_image.distance = 64;
      if (PHashRaw(client.contenuImage, client.taille, &client.hash)){
         for (int i=0; i < 3; i++)
            sfc->to_compare[i].client = client;
         pthread_mutex_init(&mutex_compare, NULL);
         pthread_create(&t1, NULL, compare_image, (void*)&sfc->to_compare[0]);
         pthread_create(&t2, NULL, compare_image, (void*)&sfc->to_compare[1]);
         pthread_create(&t3, NULL, compare_image, (void*)&sfc->to_compare[2]);
         pthread_join(t1, NULL);
         pthread_join(t2, NULL);
         pthread_join(t3, NULL);
      }
      pthread_mutex_destroy(&mutex_compare);
      checked_wr(write(sfc->new_sock, &meilleure_image, sizeof(meilleure_image)));
      pthread_mutex_unlock(&mutex_client);
   }
   sem_post(&client_semahpore);
   return NULL;
}

void connetToClient(int *server_fd, struct sockaddr_in address, struct to_compare_image* to_compare){
   int new_socket;
   size_t addrlen = sizeof(address);
   struct socket_for_client sfc;
   for(int j=0; j<3; j++) sfc.to_compare[j] = to_compare[j];
   sem_init(&client_semahpore, 0, MAX_CLIENTS);
   pthread_mutex_init(&mutex_client, NULL);
   while (1) {
      sem_wait(&client_semahpore);
      new_socket = checked(accept(*server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen));
      if (new_socket == -1) perror("accept");
      else {
         sfc.new_sock=new_socket;
         pthread_t thread;
         pthread_create(&thread, NULL, serveClient, (void*)&sfc);
      }
   }
   pthread_mutex_destroy(&mutex_client);
   sem_destroy(&client_semahpore);
   close(new_socket);
}

int main(){
   struct to_compare_image* to_compare = malloc(sizeof(struct to_compare_image) * 3);
   if (!getPictures(to_compare))
      return EXIT_FAILURE;
   int server_fd;
   struct sockaddr_in address=create_socket(&server_fd);
   connetToClient(&server_fd, address, to_compare);
   close(server_fd);
   free(to_compare);
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