#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#define MAX_CHEMINS_LONGUEUR 1000
#define MAX_CLIENTS 1000


struct image {
  // Structure représentant une image avec son chemin, sa distance et son hash.
   char chemin[1024];
   int distance;
   uint64_t hash;  // Hash de l'image représenté par un entier non signé de 64 bits
};


struct client {
  // Structure représentant un client avec la taille de l'image, son chemin, son contenu et son hash.
  unsigned int taille;
  char chemin[1024];
  char contenuImage[sizeof(char) * 1024 * 20];  // Contenu de l'image (taille maximale de 20 Ko)
  uint64_t hash;  // Hash de l'image représenté par un entier non signé de 64 bits
};


struct to_compare_image {
  // Structure représentant les images à comparer  .
  struct image librairie[34];  // Tableau de 34 images représentant la librairie à comparer
  struct client client;  // Client à comparer avec la librairie
  int amount_images;  // Nombre d'images dans la librairie
};


struct client_data {
  // Structure représentant les données d'un client, la meilleure image trouvée et la longueur totale des chemins.
  struct client client;  // Structure représentant un client avec la taille de l'image, son chemin, son contenu et son hash.
  struct image meilleure_image;  // Meilleure image trouvée lors de la comparaison
  int chemins_longueur;  // Longueur totale des chemins
};


struct socket_for_client {
  //?
  struct to_compare_image to_compare[3];  // Trois ensembles d'images à comparer avec le client
  int new_sock;  // Nouvelle socket pour la communication avec le client
};


int _checked(int ret, char* calling_function) {
  if (ret < 0) {
    perror(calling_function);
    exit(EXIT_FAILURE);
  }
  return ret;
}

// The macro allows us to retrieve the name of the calling function
#define checked(call) _checked(call, #call)

// Même macro que checked mais pour write() (où 0 signifie
// aussi une erreur).
#define checked_wr(call) _checked(((call) - 1), #call)

#endif  // __COMMON_H
