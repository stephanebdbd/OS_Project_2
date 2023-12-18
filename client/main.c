#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "../commun/commun.h"
#include <asm-generic/socket.h>

int LectureImageBMP(struct client *client){
   /**
   Initialise la structure client_data pour stocker les informations du client
   paramtres:
      client: Un pointeur vers une structure client ou on stocke les informations de l'image BMP lue.
   */
   FILE *fichierBMP = fopen(client->chemin, "rb");
   if (fichierBMP == NULL)
      return 0;

   //  place le curseur du fichier à la fin de celui ci
   fseek(fichierBMP, 0, SEEK_END);
   //indique où se trouve le curseur, ce qui correspond à la taille du fichier en octets
   client->taille = ftell(fichierBMP);

   // Vérifie si la taille du fichier est supérieure à 20 Mo
   if (client->taille > 20.000 * 1024)
      return 0;
   // Réinitialise le curseur au debut du fichierpour le prochain traitement de celui-ci le fait de mettre son contenu dans le tableau de char
   rewind(fichierBMP);
   // Lit le contenu du fichier BMP dans client->contenuImage
   if (fread(client->contenuImage, 1, client->taille, fichierBMP) == client->taille)
      return 1;

   return 0;
}

void clientListener(int sock){
   /**
   La fonction s'exécute de manière itérative tant que des chemins d'images sont disponibles à partir de l'entrée standard. Elle communique avec le serveur pour comparer les images et affiche la meilleur distance.
   paramtres:
      sock: Un descripteur de fichier de socket. elle permet de commmunique entre le client et le serveur.
   */
   struct client_data client_data;
   client_data.chemins_longueur = 0;
   // Boucle de lecture des chemins d'images depuis l'entrée standard
   while ((fgets(client_data.client.chemin, sizeof(client_data.client.chemin), stdin) != NULL) && (client_data.chemins_longueur < MAX_CHEMINS_LONGUEUR)){
      client_data.client.chemin[strlen(client_data.client.chemin)-1] = '\0';
      // Vérifie si la lecture de l'image BMP est réussie
      if ((LectureImageBMP(&client_data.client))){
         client_data.chemins_longueur += strlen(client_data.client.chemin);
         // Vérifie si la longueur totale des chemins dépasse la limit de image
         if (client_data.chemins_longueur >= MAX_CHEMINS_LONGUEUR){
            printf("No similar image found (no comparison could be performed successfully).\n");
            break;
         }
         // Envoie la structure client contenant les données de l'image BMP au serveur
         checked_wr(write(sock, &client_data.client, sizeof(struct client)));
         int lu;
         // Lit la meilleure image du serveur
         if ((lu = read(sock, &client_data.meilleure_image, sizeof(struct image))) > 0){
            if (client_data.meilleure_image.distance < 64)
               printf("Most similar image found: '%s' with a distance of %d.\n", client_data.meilleure_image.chemin, client_data.meilleure_image.distance);
            else
               printf("No similar image found (no comparison could be performed successfully).\n");
         }
      }
      else
         printf("No similar image found (no comparison could be performed successfully).\n");
   }
}

int main(){
   int sock = checked(socket(AF_INET, SOCK_STREAM, 0));
   struct sockaddr_in address;
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(5555);
   inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
   // Tente de se connecter au serveur
   if (connect(sock, (struct sockaddr *)&address, sizeof(address)) == -1){
      perror("Le serveur n'est pas initialisé.\nVeuillez réessayer plus tard.\n");
      return 0;
   }
   // Fonction principale du client pour écouter les entrées et envoyer des données au serveur
   clientListener(sock);
   // Ferme la socket après l'écoute
   close(sock);
   return 1;
}
