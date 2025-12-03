#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <png.h>

#include "PNGUtility.h"

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep *row_pointers;

void abort_(const char *s, ...)
{
    va_list args;

    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

void read_png_file(char *file_name, int *nWidth, int *nHeight)
{
    char header[8];             // 8 is the maximum size that can be checked

    *nWidth = 0;
    *nHeight = 0;

    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");

    if (!fp)
        abort_("[read_png_file] File %s could not be opened for reading",
               file_name);

    if (fread(header, 1, 8, fp) != 8)
        abort_("[read_png_file] File %s read header failed",
               file_name);
    if (png_sig_cmp((png_bytep) header, 0, 8))
        abort_("[read_png_file] File %s is not recognized as a PNG file",
               file_name);

    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        abort_("[read_png_file] png_create_read_struct failed");

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        abort_("[read_png_file] png_create_info_struct failed");

    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during init_io");

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    *nWidth = width;
    *nHeight = height;

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);


    /* read file */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during read_image");

    row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);

    for (int y = 0; y < height; y++)
        row_pointers[y] =
            (png_byte *) malloc(png_get_rowbytes(png_ptr, info_ptr));

    png_read_image(png_ptr, row_pointers);

    fclose(fp);
}

void process_file(unsigned char *tmpBlend, int nWidth, int nHeight)
{
    int nCurrWidth = width > nWidth ? nWidth : width;
	int nCurrHeight = height > nHeight ? nHeight : height;

    // PNG_COLOR_TYPE_GRAY
    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY)
    {
        for (int y = 0; y < nCurrHeight; y++)
        {
            png_byte *row = row_pointers[y];

            for (int x = 0; x < nCurrWidth; x++)
            {
                png_byte *ptr = &(row[x]);

                // printf("Pixel at position [ %d - %d ] has value %d\n", x,
                // y, ptr[0]);

                unsigned char tmp = ptr[0];

                //if (x < nWidth && y < nHeight)
                    tmpBlend[nCurrWidth * y + x] = tmp;
            }
        }
    }

    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
    {
        //width = width > nWidth ? nWidth : width;
        //height = height > nHeight ? nHeight : height;
        for (int y = 0; y < nCurrHeight; y++)
        {
            png_byte *row = row_pointers[y];

            for (int x = 0; x < nCurrWidth; x++)
            {
                png_byte *ptr = &(row[x * 3]);

                // printf("Pixel at position [ %d - %d ] has RGBA values: %d -
                // %d - %d - %d\n", x, y, ptr[0], ptr[1], ptr[2], ptr[3]);

                if (x < nWidth && y < nHeight)
                {
                    tmpBlend[nCurrWidth * 3 * y + 3 * x] = ptr[0];
                    tmpBlend[nCurrWidth * 3 * y + 3 * x + 1] = ptr[1];
                    tmpBlend[nCurrWidth * 3 * y + 3 * x + 2] = ptr[2];
                }
            }
        }
    }

    if(png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB_ALPHA)
    {
        //width = width > nWidth ? nWidth : width;
        //height = height > nHeight ? nHeight : height;
        for (int y = 0; y < nCurrHeight; y++)
        {
            png_byte *row = row_pointers[y];

            for (int x = 0; x < nCurrWidth; x++)
            {
                png_byte *ptr = &(row[x * 4]);

                // printf("Pixel at position [ %d - %d ] has RGBA values: %d -
                // %d - %d - %d\n", x, y, ptr[0], ptr[1], ptr[2], ptr[3]);

                if (x < nWidth && y < nHeight)
                {
                    tmpBlend[nCurrWidth * 3 * y + 3 * x] = ptr[0];
                    tmpBlend[nCurrWidth * 3 * y + 3 * x + 1] = ptr[1];
                    tmpBlend[nCurrWidth * 3 * y + 3 * x + 2] = ptr[2];
                }
            }
        }
    }

	// free malloc memory
    for (int y = 0; y < height; y++)
        free(row_pointers[y]);

	free(row_pointers);
}

/* Given "bitmap", this returns the pixel of bitmap at the point ("x", "y"). */

pixel_t *pixel_at(bitmap_t * bitmap, int x, int y)
{
    return bitmap->pixels + bitmap->width * y + x;
}

/* Write "bitmap" to a PNG file specified by "path"; returns 0 on success,
   non-zero on error. */

int save_png_to_file(bitmap_t * bitmap, const char *path)
{

    FILE *fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte **row_pointers = NULL;

    /* "status" contains the return value of this function. At first it is set
       to a value which means 'failure'. When the routine has finished its
       work, it is set to a value which means 'success'. */
    int status = -1;

    /* The following number is set by trial and error only. I cannot see where
       it it is documented in the libpng manual. */
    int pixel_size = 3;
    int depth = 8;

    fp = fopen(path, "wb");
    if (!fp)
    {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
    {
        goto png_create_write_struct_failed;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        goto png_create_info_struct_failed;
    }

    /* Set up error handling. */

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        goto png_failure;
    }

    /* Set image attributes. */

    png_set_IHDR(png_ptr,
                 info_ptr,
                 bitmap->width,
                 bitmap->height,
                 depth,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    /* Initialize rows of PNG. */

    row_pointers =
        (png_byte **) png_malloc(png_ptr, bitmap->height * sizeof(png_byte *));
    for (y = 0; y < bitmap->height; y++)
    {
        png_byte *row =
            (png_byte *) png_malloc(png_ptr,
                                    sizeof(uint8_t) * bitmap->width *
                                    pixel_size);
        row_pointers[y] = row;
        for (x = 0; x < bitmap->width; x++)
        {
            pixel_t *pixel = pixel_at(bitmap, x, y);

            *row++ = pixel->red;
            *row++ = pixel->green;
            *row++ = pixel->blue;
        }
    }

    /* Write the image data to "fp". */

    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* The routine has successfully written the file, so we set "status" to a
       value which indicates success. */

    status = 0;

    for (y = 0; y < bitmap->height; y++)
    {
        png_free(png_ptr, row_pointers[y]);
    }
    png_free(png_ptr, row_pointers);

  png_failure:
  png_create_info_struct_failed:
    png_destroy_write_struct(&png_ptr, &info_ptr);
  png_create_write_struct_failed:
    fclose(fp);
  fopen_failed:
    return status;
}

/* Given "value" and "max", the maximum value which we expect "value" to take,
   this returns an integer between 0 and 255 proportional to "value" divided
   by "max". */

int pix(int value, int max)
{
    if (value < 0)
    {
        return 0;
    }
    return (int)(256.0 * ((double)(value) / (double)max));
}



void generatePNG(unsigned char *data, int width, int height, char *fileName)
{


    bitmap_t fruit;
    int x;
    int y;
    int status;

    status = 0;

    /* Create an image. */

    fruit.width = width;
    fruit.height = height;

    fruit.pixels =
        (pixel_t *) calloc(fruit.width * fruit.height, sizeof(pixel_t));

    if (!fruit.pixels)
    {
        return;
    }

    int i = 0;

    for (y = 0; y < fruit.height; y++)
    {
        for (x = 0; x < fruit.width; x++)
        {
            pixel_t *pixel = pixel_at(&fruit, x, y);

            pixel->red = data[i];
            pixel->green = data[i + 1];
            pixel->blue = data[i + 2];
            i = i + 3;
        }
    }

    /* Write the image to a file 'fruit.png'. */

    if (save_png_to_file(&fruit, fileName))
    {
        fprintf(stderr, "Error writing file.\n");
        status = -1;
    }

    free(fruit.pixels);
}
