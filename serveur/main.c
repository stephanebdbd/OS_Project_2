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
pthread_mutex_t mutex_compare, mutex_client;
sem_t client_semahpore;
pthread_t t1, t2, t3;
int thread_creation = 0, new_socket;


void* compare_image(void *ptr) {
   /**
   Fonction exécutée par chaque thread pour comparer les images.
   paramtres:
    ptr: Le pointeur vers la struct to_compare_image.
   return:
    NULL.
   */
   struct to_compare_image* to_compare = (struct to_compare_image*)ptr;
   for (int j = 0; j < to_compare->amount_images; j++) {
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
   /**
   Récupère la liste des images et les hashs depuis le programme externe "list-file".
   paramtres:
    to_compare: Le tableau de structs to_compare_image.
   return:
    1 en cas de succès, 0 en cas d'échec.
 */
   FILE *listing = popen("./list-file img", "r");
   if (listing == NULL) {
      perror("Erreur lors de l'ouverture du processus");
      exit(EXIT_FAILURE);
   }
   int i=0;
   for (int j=0; j < 3; j++)
      to_compare[j].amount_images = 0;
   // Boucle principale pour lire les chemins des images depuis le fichier de liste
   while ((fgets(to_compare[i].librairie[to_compare[i].amount_images].chemin, sizeof(to_compare[i].librairie[to_compare[i].amount_images].chemin), listing) != NULL)){
      // Supprime le caractère de nouvelle ligne à la fin de chaque chemin
      to_compare[i].librairie[to_compare[i].amount_images].chemin[strlen(to_compare[i].librairie[to_compare[i].amount_images].chemin)-1] = '\0';
      // Calcul du hash pour l'image
      if (!PHash(to_compare[i].librairie[to_compare[i].amount_images].chemin, &to_compare[i].librairie[to_compare[i].amount_images].hash))
         return 0;
      // Incrémente le compteur d'images pour la struct en cours
      to_compare[i].amount_images++;
      // Passage à la struct suivante après 34 images (selon la logique dans le code d'origine)
      i += (to_compare[i].amount_images == 34) ? 1 : 0;
   }
   pclose(listing);
   return 1;
}

struct sockaddr_in create_socket(int* server_fd){
   /**
   Crée un socket pour le serveur.
   paramtres:
    server_fd Pointeur vers le descripteur de fichier du socket.
   return:
    adress: La struct sockaddr_in représentant l'adresse du serveur.
   */
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
   /**
   Fonction exécutée par chaque thread pour servir un client.
   paramtres:
    arg : Pointeur vers la struct socket_for_client.
   return :
      NULL.
   */
   struct socket_for_client* sfc = (struct socket_for_client*) arg;
   struct client client;
   int lu;
   // Boucle principale pour lire les données du client et comparer les images
   while ((lu = read(sfc->new_sock, &client, sizeof(struct client))) > 0) {
      pthread_mutex_lock(&mutex_client);
      meilleure_image.distance = 64;
      // Vérifie si le hachage de l'image du client peut être calculé
      if (PHashRaw(client.contenuImage, client.taille, &client.hash)){
         // Initialise les structs pour la comparaison dans chaque thread
         for (int i = 0; i < 3; i++)
            sfc->to_compare[i].client = client;
         pthread_mutex_init(&mutex_compare, NULL);
         // Crée trois threads pour la comparaison des images
         pthread_create(&t1, NULL, compare_image, (void*)&sfc->to_compare[0]);
         pthread_create(&t2, NULL, compare_image, (void*)&sfc->to_compare[1]);
         pthread_create(&t3, NULL, compare_image, (void*)&sfc->to_compare[2]);

         // Attend la fin de chaque thread
         pthread_join(t1, NULL);
         pthread_join(t2, NULL);
         pthread_join(t3, NULL);
      }
      // Detruit mutex de comparaison
      pthread_mutex_destroy(&mutex_compare);
      // Envoie la meilleure image au client
      checked_wr(write(sfc->new_sock, &meilleure_image, sizeof(meilleure_image)));
      // Libère le mutex client
      pthread_mutex_unlock(&mutex_client);
   }
   // Libère le sémaphore client
   sem_post(&client_semahpore);
   return NULL;
}

void connetToClient(int *server_fd, struct sockaddr_in address, struct to_compare_image* to_compare){
   /**
   Accepte les connexions des clients et les traite.
   parametres:
    server_fd: Le descripteur de fichier du socket du serveur.
    address: La struct sockaddr_in représentant l'adresse du serveur.
    to_compare: Le tableau de structs to_compare_image.
   */
   size_t addrlen = sizeof(address);
   struct socket_for_client sfc;
   for(int j=0; j<3; j++) sfc.to_compare[j] = to_compare[j];
   sem_init(&client_semahpore, 0, MAX_CLIENTS);
   pthread_mutex_init(&mutex_client, NULL);
   // Initialise le sémaphore client avec un compteur initial de MAX_CLIENTS
   sem_init(&client_semahpore, 0, MAX_CLIENTS);
   pthread_mutex_init(&mutex_client, NULL);

   // Boucle principale pour accepter les connexions des clients
   while (1) {
      // Attends qu'un slot soit disponible dans le sémaphore client
      sem_wait(&client_semahpore);

      // Accepte une nouvelle connexion et obtient le descripteur de fichier du socket associé
      new_socket = checked(accept(*server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen));
      if (new_socket == -1) perror("accept");
      else {
         // Initialise la struct pour le socket du client
         sfc.new_sock = new_socket;
         for (int j = 0; j < 3; j++)
            sfc.to_compare[j] = to_compare[j];

         // Crée un thread pour servir le client
         pthread_t thread;
         pthread_create(&thread, NULL, serveClient, (void*)&sfc);
      }
   }
   // detruit mutex client
   pthread_mutex_destroy(&mutex_client);
   // Détruit le sémaphore client
   sem_destroy(&client_semahpore);
   // Ferme le socket du dernier client (cette ligne n'est jamais atteinte dans la boucle infinie)
   close(new_socket);
}

void SignalHandler(int sig) {
   switch (sig) {
      case SIGINT:
         if (thread_creation != 0) {
            pthread_kill(t1, SIGINT);
            pthread_kill(t2, SIGINT);
            pthread_kill(t3, SIGINT);
         }
         sem_post(&client_semahpore);
         exit(0);
         break;
      case SIGPIPE:
         if (thread_creation != 0) {
            pthread_kill(t1, SIGINT);
            pthread_kill(t2, SIGINT);
            pthread_kill(t3, SIGINT);
         }
         strcpy(meilleure_image.chemin , "SIGINT");
         checked_wr(write(new_socket, &meilleure_image, sizeof(meilleure_image)));
         sem_post(&client_semahpore);
         break;
      
   }
}

int main(){
   /**
   Fonction principale du serveur.
   return:
    Le code de sortie du programme.
    */
   signal(SIGINT, SignalHandler);
   signal(SIGPIPE, SignalHandler);
   struct to_compare_image* to_compare = malloc(sizeof(struct to_compare_image) * 3);
   if (!getPictures(to_compare))
      return EXIT_FAILURE;
   int server_fd;
   struct sockaddr_in address=create_socket(&server_fd);
   connetToClient(&server_fd, address, to_compare);
   close(server_fd);
   free(to_compare);
   return 0;
}