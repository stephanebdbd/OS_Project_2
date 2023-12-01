#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "bmp.h"
#include "bmp-endian.h"
#include "verbose.h"

// Boutisme utilisé pour les .bmp : petit boutisme

struct BitmapFileHeader {
   uint16_t bmp;
   uint32_t size;
   uint32_t reserved;
   uint32_t offsetData;
};

struct DibHeader {
   uint32_t headerSize;
   uint32_t width;
   uint32_t height;
   uint16_t colorPlanes; // 1
   uint16_t bitsPerPixel; // expect 24 or 32
   uint32_t compressionMethod; // 0 = aucune compression
   uint32_t imageRawSize;
   int32_t horizontalResolution; // px/m
   int32_t verticalResolution; // px/m
   uint32_t nbColorsPalette;
   uint32_t importantColors;
};

struct BmpFileHeader {
   struct BitmapFileHeader bmpHeader;
   struct DibHeader dibHeader;
};

/**
 * Lit l'en-tête de l'image contenu dans le fichier ouvert dans `f` et
 * initialise les dimensions de `image` à celle de l'image du fichier
 * en plus de précisé le nombre de bits par pixels dans `*bitsPerPixel`.
 * 
 * @return true en cas de succès, false sinon.
 **/
static bool ReadBmpHeader(RgbImage* image, uint16_t* bitsPerPixel, FILE* f) {
   struct BmpFileHeader header;
   uint32_t offset;

   if (fread(&header.bmpHeader.bmp, sizeof(uint16_t), 1, f) != 1)
      return false;
   
   // Étant donné que les valeurs sont encodées directement en binaire,
   // il est important de s'assurer qu'elles soient récupérer dans le
   // boutisme local de la machine pour laquelle ce code a été compilé.
   // Les macros TO_LOCAL_ENDIAN16() et TO_LOCAL_ENDIAN32() sont utilisées
   // à cette fin.
   header.bmpHeader.bmp = TO_LOCAL_ENDIAN16(header.bmpHeader.bmp);
   if (header.bmpHeader.bmp != ('M' << 8 | 'B'))
      return false;
   
   if (fread(&header.bmpHeader.size, sizeof(uint32_t), 3, f) != 3)
      return false;
   
   
   offset = TO_LOCAL_ENDIAN32(header.bmpHeader.offsetData);
   
   // DibHeader
   if (fread(&header.dibHeader, sizeof(header.dibHeader), 1, f) != 1)
      return false;
   
   image->width = TO_LOCAL_ENDIAN32(header.dibHeader.width);
   image->height = TO_LOCAL_ENDIAN32(header.dibHeader.height);
   *bitsPerPixel = TO_LOCAL_ENDIAN16(header.dibHeader.bitsPerPixel);
   
   if (fseek(f, offset, SEEK_SET) != 0)
      return false;
   
   return true;
}

static bool LoadBmpFromFILE(RgbImage* image, FILE* f) {
   uint16_t bitsPerPixel;
   
   if (!ReadBmpHeader(image, &bitsPerPixel, f)) {
      fputs("Impossible de lire l'en-tête du fichier .bmp.\n", stderr);
      fclose(f);
      return false;
   }
   
   if (bitsPerPixel == 24 || bitsPerPixel == 32) {
      image->data = (uint32_t*)malloc(sizeof(uint32_t) * image->width * image->height);
      if (image->data == NULL) {
         perror("LoadBmp::malloc()");
         fclose(f);
         return false;
      }
   } else {
      fprintf(stderr, "BPP de %u non supporté (seuls des BPP de 24 et 32 le sont).\n", bitsPerPixel);
      fclose(f);
      return false;
   }
   
   switch(bitsPerPixel) {
      case 24:
         for (unsigned int i = 0; i < image->width * image->height; ++i) {
            if (fread(&image->data[i], 3, 1, f) != 1) {
               perror("Lecture des données");
               return false;
            }
         }
         break;
      case 32:
         if (fread(image->data, sizeof(uint32_t), image->width * image->height, f) != image->width * image->height) {
            perror("Lecture des données");
            return false;
         }
            
         break;
   }
   
   uint32_t color, end = image->width * image->height - 1;
   for (unsigned int i = 0; i < image->width * image->height / 2; ++i) {
      color = TO_LOCAL_ENDIAN32(image->data[i]);
      image->data[i] = TO_LOCAL_ENDIAN32(image->data[end - i]);
      image->data[end - i] = color;
   }
   
   fclose(f);
   return true;
}

bool InitBmpFromRaw(RgbImage* image, const char rawImageBytes[], unsigned int size) {
   FILE* f = fmemopen((char*)rawImageBytes, size, "r");
   
   if (f == NULL) {
      perror("InitBmpFromRaw::fmemopen()");
      return false;
   }
   
   return LoadBmpFromFILE(image, f);
}

bool LoadBmp(RgbImage* image, const char file[]) {
   FILE* f = fopen(file, "rb");
   
   if (f == NULL) {
      perror("LoadBmp::fopen()");
      return false;
   }
   
   return LoadBmpFromFILE(image, f);
}

void FreeImage(RgbImage* image) {
   free(image->data);
}

void DisplayBMP(RgbImage* image) {
   const uint32_t pixels = image->width * image->height;
   const uint32_t width = image->width;
   uint32_t c;
   
   if (!MODE_VERBOSE_ON)
      return;
   
   for (uint32_t i = 0; i < pixels;) {
      #define BLUE(color) ((color) & 0x0FFu)
      #define GREEN(color) (((color) & 0x0FF00u) >> 8)
      #define RED(color) (((color) & 0x0FF0000u) >> 16)
      
      c = image->data[i];
      
      printf("\x1B[38;2;%d;%d;%dm X", RED(c), GREEN(c), BLUE(c));
      
      ++i;
      if (i % width == 0)
         putchar('\n');
   }
   printf("\x1B[0m\n");
}
