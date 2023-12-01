#ifndef __LIB_IMG_DIST_H
#define __LIB_IMG_DIST_H

#ifdef __cplusplus
   #include <cstdint>
   extern "C" {
#else
   #include <stdint.h>
   #include <stdbool.h>
#endif

/**
 * Calcule le code de hachage perceptif de l'image dont le chemin est
 * `imagePath` et le stocke de `*hash` en cas de succès.
 * 
 * @return false en cas d'erreur et true en cas du succès.
 **/
bool PHash(const char imagePath[], uint64_t* hash);

/**
 * Calcule le code de hachage perceptif de l'image dont le contenu, de
 * taille `size`, est entièrement dans `rawImage` puis le conserve dans
 * `*hash`.
 * 
 * @return false en cas d'erreur et true en cas du succès.
 **/
bool PHashRaw(const char rawImage[], unsigned int size, uint64_t* hash);

/**
 * Calcule la distance (de Hamming) entre deux codes de hachage perceptif.
 * 
 * @return La distance entre `pHash1` et `pHash2`.
 **/
unsigned int DistancePHash(uint64_t pHash1, uint64_t pHash2);

#ifdef __cplusplus
   }
#endif

#endif
