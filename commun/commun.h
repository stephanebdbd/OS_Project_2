#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
  int longueur;
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
