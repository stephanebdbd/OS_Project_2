#ifndef PHASH_H__
#define PHASH_H__

#include <stdint.h>

#include "bmp.h"

/**
 * Calcule le code de hachage perceptif de l'image `image` et le retourne.
 * 
 * @return Code de hachage perceptif de `image`.
 **/
uint64_t pHash(RgbImage* image);

/**
 * Calcule la distance (de Hamming) entre deux codes de hachage perceptif.
 * 
 * @return La distance entre `pHash1` et `pHash2`.
 **/
unsigned int DistancePHash(uint64_t pHash1, uint64_t pHash2);


/**
 * Calcule le code de hachage perceptif de l'image située en `imagePath`
 * et le conserve dans `*hash`.
 * 
 * @return false en cas d'erreur et true sinon.
 **/
bool PHash(const char imagePath[], uint64_t* hash);

/**
 * Calcule le code de hachage perceptif de l'image dont le contenu, de
 * taille `size`, est entièrement dans `rawImage` puis le conserve dans
 * `*hash`.
 * 
 * @return false en cas d'erreur et true sinon.
 **/
bool PHashRaw(const char rawImage[], unsigned int size, uint64_t* hash);

#endif
