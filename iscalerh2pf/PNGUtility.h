#ifndef COREPNGUTILITY_H
#define COREPNGUTILITY_H

// Cindy 20190227-212736
#include <png.h>
#include <stdint.h>
#include <stdlib.h>


/* A coloured pixel. */

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
}
pixel_t;

/* A picture. */

typedef struct
{
    pixel_t *pixels;
    size_t width;
    size_t height;
}
bitmap_t;

void read_png_file(char *file_name, int *nWidth, int *nHeight);
void process_file(unsigned char *tmpBlend, int nWidth, int nHeight);

pixel_t *pixel_at(bitmap_t * bitmap, int x, int y);
int save_png_to_file(bitmap_t * bitmap, const char *path);
int pix(int value, int max);
void generatePNG(unsigned char *data, int width, int height, char *fileName);

#endif /* COREPNGUTILITY_H */
