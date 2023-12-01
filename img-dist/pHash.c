#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "pHash.h"
#include "bmp.h"
#include "verbose.h"


/**
 * Calcule la transformée en cosinus discrets (DCT) à 2 dimensions
 * de la matrice carré `input` d'ordre `size` et stocke les coefficients
 * obtenus dans `input`.
 **/
static void DiscreteCosineTransform2D(uint32_t size, const float input[size][size], float output[size][size]) {
   uint32_t x, y, u, v;
   float sum, Cu, Cv;
   const float K = 2. / size; // Note : size == |size| == sqrt(size * size)

   for (u = 0; u < size; u++) {
      for (v = 0; v < size; v++) {
         sum = 0;
         for (x = 0; x < size; x++) {
            for (y = 0; y < size; y++) {
               sum = sum + input[x][y] * cos(((2. * x + 1.) * u * M_PI) / (2. * size))
                     * cos(((2. * y + 1.) * v * M_PI) / (2. * size));
            }
         }
         
         Cu = (u == 0 ? 1 / sqrt(2.) : 1);
         Cv = (v == 0 ? 1 / sqrt(2.) : 1);

         output[u][v] = K * Cu * Cv * sum;
      }
   }
}

/**
 * Calcule la couleur du pixel (`x`, `y`) de l'image obtenue en
 * redimensionnant `image` selon le ratio `wRatio` pour la largeur et
 * le ratio `hRatio` pour la hauteur.
 * 
 * Retourne ce pixel après l'avoir converti en niveaux de gris.
 **/
static inline float ScaleAsGray(RgbImage* image, uint32_t x, uint32_t y, float wRatio, float hRatio) {
   #define min(a, b) ((a) <= (b) ? (a) : (b))
   uint32_t maxY = min((uint32_t)(y * hRatio) + ceil(hRatio), image->height);
   uint32_t maxX = min((uint32_t)(x * wRatio) + ceil(wRatio), image->width);
   
   float averageGray = 0.f;
   for (uint32_t i = y * hRatio; i < maxY; ++i) {
      for (uint32_t j = x * wRatio; j < maxX; ++j) {
         #define BLUE(color) ((color) & 0x0FFu)
         #define GREEN(color) (((color) & 0x0FF00u) >> 8)
         #define RED(color) (((color) & 0x0FF0000u) >> 16)
         
         #define GRAY(color) \
            (0.3 * RED(color) + 0.6 * GREEN(color) + 0.1 * BLUE(color))
         
         uint32_t color = image->data[i * image->width + j];
         averageGray += GRAY(color);
         
         #undef GRAY
         #undef RED
         #undef GREEN
         #undef BLUE
      }
   }
   
   return averageGray / ((maxX - (uint32_t)(x * wRatio)) * (maxY - (uint32_t)(y * hRatio)));
}

/**
 * Redimensionne `image` dans les dimensions `newDimension`x`newDimension`
 * et la passant en niveau de gris.
 * 
 * L'image résultante de cette transformation est conservée dans le
 * tableau à deux dimensions `reducedGrayScaleImage`.
 **/
static void ReduceAndGray(RgbImage* image, uint32_t newDimension, float reducedGrayScaleImage[newDimension][newDimension]) {
   const float wRatio = image->width / (float)newDimension;
   const float hRatio = image->height / (float)newDimension;
   
   for (uint32_t i = 0; i < newDimension; ++i) {
      for (uint32_t j = 0; j < newDimension; ++j) {
         reducedGrayScaleImage[i][j] = ScaleAsGray(image, j, i, wRatio, hRatio);
      }
   }
}

/**
 * Affiche une version d'une image 32x32 encodée en niveaux de gris
 * dans le tableau `reducedGrayScaleImage`.
 **/
static inline void DisplayGrayMiniature(float reducedGrayScaleImage[32][32]) {
   uint32_t data[32 * 32];
   uint32_t color;
   
   if (!MODE_VERBOSE_ON)
      return;
   
   for (unsigned int i = 0; i < 32 * 32; ++i) {
      color = reducedGrayScaleImage[i / 32][i % 32];
      data[i] = color | (color << 8) | (color << 16);
   }
   
   RgbImage tmp = {
      .width = 32,
      .height = 32,
      .data = data
   };
   
   DisplayBMP(&tmp);
}

uint64_t pHash(RgbImage* image) {
   float reducedGrayScaleImage[32][32];
   float cosines[32][32];
   float average;
   uint64_t hash;
   
   ReduceAndGray(image, 32, reducedGrayScaleImage);
   
   DisplayGrayMiniature(reducedGrayScaleImage);
   
   DiscreteCosineTransform2D(32, reducedGrayScaleImage, cosines);
   
   average = -cosines[0][0]; // Première valeur ignorée
   for (unsigned int i = 0; i < 8; ++i) {
      for (unsigned int j = 0; j < 8; ++j) {
         average += cosines[i][j];
      }
   }
   
   average /= 64.f;
   
   hash = 0;
   for (unsigned int i = 0; i < 8; ++i) {
      for (unsigned int j = 0; j < 8; ++j) {
         if (cosines[i][j] >= average)
            hash |= ((uint64_t)1) << (i * 8 + j);
      }
   }
   
   return hash;
}

bool PHash(const char imagePath[], uint64_t* hash) {
   RgbImage image;
   
   if (!LoadBmp(&image, imagePath))
      return false; // Échec dans le chargement de l'image (message sur stderr automatique)
   
   *hash = pHash(&image); // Récupérer le code de hachage perceptif de l'image

   FreeImage(&image);
   return true;
}

bool PHashRaw(const char rawImage[], unsigned int size, uint64_t* hash) {
   RgbImage image;
   
   if (!InitBmpFromRaw(&image, rawImage, size))
      return false; // Échec dans le chargement de l'image (message sur stderr automatique)
   
   *hash = pHash(&image); // Récupérer le code de hachage perceptif de l'image

   FreeImage(&image);
   return true;
}

unsigned int DistancePHash(uint64_t pHash1, uint64_t pHash2) {
   unsigned int distance = 0;
   
   for (unsigned int i = 0; i < 64; ++i) {
      distance += ((pHash1 & ((uint64_t)1 << i)) != (pHash2 & ((uint64_t)1 << i)));
   }
   
   return distance;
}
