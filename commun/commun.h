#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#define MAX_CHEMINS_LONGUEUR 1000
#define MAX_CLIENTS 1000

struct image {
   char chemin[1024];
   int distance;
   uint64_t hash;  
};
struct client{
  unsigned int taille;
  char chemin[1024];
  char contenuImage[sizeof(char) * 1024 * 20];
  uint64_t hash;
};

struct to_compare_image{
  struct image librairie[34];
  struct client client;
  int amount_images;
};

struct client_data{
  struct client client;
  struct image meilleure_image;
  int chemins_longueur;
};

struct socket_for_client{
  struct to_compare_image to_compare[3];
  int new_sock;
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
