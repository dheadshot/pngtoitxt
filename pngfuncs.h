#ifndef __PNGFUNCS_H__

#define __PNGFUNCS_H__ 1

#include <png.h>

typedef struct picinfo_struct {
  int width;
  int height;
  png_byte colourtype;
  png_byte bitdepth;
  int numpasses;
  png_structp picptr;
  png_infop infoptr;
} picinfo;

typedef png_bytep * picdatarows;


int readpng(picinfo *info, picdatarows *dataptr, const char *filename);
void freepicdata(picdatarows rows, int numrows);
void freepicinfo(picinfo *info);
picinfo *newpicinfo();
png_bytep pixelptr(picinfo *info, picdatarows data, int x, int y);


#endif
