#ifndef BMP_H__
#define BMP_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
   uint16_t width;
   uint16_t height;
   uint32_t* data;
} RgbImage;

/**
 * Charge une image BMP contenue dans le fichier `file` et la conserve
 * dans `image`.
 * 
 * @return true en cas de succès et false sinon (en plus d'afficher un
 *    message détaillant l'erreur sur stderr).
 **/
bool LoadBmp(RgbImage* image, const char file[]);

/**
 * Charge une image BMP dont le contenu est stocké entièrement dans
 * `rawImageBytes` et la conserve dans `image`.
 * 
 * @return true en cas de succès et false sinon (en plus d'afficher un
 *    message détaillant l'erreur sur stderr).
 **/
bool InitBmpFromRaw(RgbImage* image, const char rawImageBytes[], unsigned int size);

/**
 * Libère proprement la mémoire allouée pour l'image `image`.
 **/
void FreeImage(RgbImage* image);

/**
 * Affiche sur la console l'image `image`.
 **/
void DisplayBMP(RgbImage* image);

#endif
