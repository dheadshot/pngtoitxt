#include <png.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pngfuncs.h"

int readpng(picinfo *info, picdatarows *dataptr, const char *filename)
{
  char hdr[8];
  int y, yi;
  
  if (!info || !dataptr || !filename) return 0;
  
  FILE *afp = fopen(filename, "rb");
  if (!afp)
  {
    fprintf(stderr, "Could not open %s for reading!\n", filename);
    return -1;
  }
  fread(hdr, sizeof(char), 8, afp);
  if (png_sig_cmp(hdr,0,8))
  {
    fprintf(stderr, "File %s is not a PNG!\n", filename);
    fclose(afp);
    return -2;
  }
  
  info->picptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!info->picptr)
  {
    fprintf(stderr, "Could not create reading structure!\n");
    fclose(afp);
    return -3;
  }
  info->infoptr = png_create_info_struct(info->picptr);
  if (!info->infoptr)
  {
    fprintf(stderr, "Could not create information structure!\n");
    png_destroy_read_struct(&(info->picptr), (png_infopp) NULL, (png_infopp) NULL);
    fclose(afp);
    return -4;
  }
  
  if (setjmp(png_jmpbuf(info->picptr)))
  {
    fprintf(stderr, "Error during init_io!\n");
    png_destroy_read_struct(&(info->picptr), &(info->infoptr), (png_infopp) NULL);
    fclose(afp);
    return -5;
  }
  png_init_io(info->picptr, afp);
  png_set_sig_bytes(info->picptr, 8);
  png_read_info(info->picptr, info->infoptr);
  info->width = png_get_image_width(info->picptr, info->infoptr);
  info->height = png_get_image_height(info->picptr, info->infoptr);
  info->colourtype = png_get_color_type(info->picptr, info->infoptr);
  info->bitdepth = png_get_bit_depth(info->picptr, info->infoptr);
  info->numpasses = png_set_interlace_handling(info->picptr);
  png_read_update_info(info->picptr, info->infoptr);
  
  if (setjmp(png_jmpbuf(info->picptr)))
  {
    fprintf(stderr, "Error during image read!\n");
    png_destroy_read_struct(&(info->picptr), &(info->infoptr), (png_infopp) NULL);
    fclose(afp);
    return -6;
  }
  
  (*dataptr) = (picdatarows) malloc(sizeof(png_bytep) * info->height);
  if (!(*dataptr))
  {
    fprintf(stderr, "Out of Memory during image read!\n");
    png_destroy_read_struct(&(info->picptr), &(info->infoptr), (png_infopp) NULL);
    fclose(afp);
    return -7;
  }
  for (y=0; y<info->height; y++)
  {
    (*dataptr)[y] = (png_bytep) malloc(png_get_rowbytes(info->picptr, info->infoptr));
    if (!((*dataptr)[y]))
    {
      fprintf(stderr, "Out of Memory during image read!\n");
      for (yi=0;yi<y;yi++) free((*dataptr)[yi]);
      free((*dataptr));
      png_destroy_read_struct(&(info->picptr), &(info->infoptr), (png_infopp) NULL);
      fclose(afp);
      return -7;
    }
  }
  
  png_read_image(info->picptr, (*dataptr));
  fclose(afp);
  return 1;
}

void freepicdata(picdatarows rows, int numrows)
{
  int i;
  for (i=0;i<numrows;i++) free(rows[i]);
  free(rows);
}

void freepicinfo(picinfo *info)
{
  png_destroy_read_struct(&(info->picptr), &(info->infoptr), (png_infopp) NULL);
  free(info);
}

picinfo *newpicinfo()
{
  picinfo *info = malloc(sizeof(picinfo));
  if (!info) return NULL;
  memset(info, 0, sizeof(picinfo));
  return info;
}

png_bytep pixelptr(picinfo *info, picdatarows data, int x, int y)
{
  int numchans, pixeldepth;
  if (x>=info->width || y>=info->height || x<0 || y<0) return NULL;
  switch (info->colourtype) //(png_get_color_type(info->picptr, info->infoptr))
  {
    case PNG_COLOR_TYPE_RGB:
      numchans = 3;
    break;
    
    case PNG_COLOR_TYPE_RGBA:
      numchans = 4;
    break;
    
    default:
      fprintf(stderr, "Unknown colour type!\n");
      return NULL;
    break;
  }
  
  pixeldepth = (numchans * info->bitdepth);
  
  png_bytep datarow = data[y];
  png_bytep datum = &(datarow[((int) ((x*pixeldepth)/8))]);
  
  return datum;
}
