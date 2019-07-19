#include <png.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pngfuncs.h"
#include "main.h"

const char charscale[9] = " .:=*#%@";

int getavrpixel(bigpixel *apix, picinfo *info, picdatarows data, int x, int y, int pwidth, int pheight)
{
  int xi, yi, n = 0, pixdepth;
  png_bytep apixel;
  unsigned long r = 0, g = 0, b = 0, a = 0;
  
  if (((info->bitdepth >> 3) << 3) != info->bitdepth)
  {
    fprintf(stderr, "Bit depth %d not a multiple of 8!  Support for this has not yet been \nimplemented!", info->bitdepth);
    return -2;
  }
  
  switch (info->colourtype)
  {
    case PNG_COLOR_TYPE_RGB:
      pixdepth = info->bitdepth * 3;
    break;
    
    case PNG_COLOR_TYPE_RGBA:
      pixdepth = info->bitdepth * 4;
    break;
    
    default:
      fprintf(stderr, "Error: Unknown colour type %d!\n", info->colourtype);
      return -1;
    break;
  }
  
  /* Get average colour for the rectangle */
  for (yi=y;yi<y+pheight;yi++)
  {
    for (xi=x;xi<x+pwidth;xi++)
    {
      apixel = pixelptr(info, data, xi, yi);
      if (apixel)
      {
        n++;
        
        r += apixel[0];
        g += apixel[(1*info->bitdepth) >> 3];
        b += apixel[(2*info->bitdepth) >> 3];
        switch (info->colourtype)
        {
          case PNG_COLOR_TYPE_RGBA:
            a += apixel[(3*info->bitdepth) >> 3];
          break;
        }
        
      }
    }
  }
  
  r /= n;
  g /= n;
  b /= n;
  a /= n;
  
  apix->avr_r = r;
  apix->avr_g = g;
  apix->avr_b = b;
  apix->avr_a = a;
  apix->bitdepth = info->bitdepth;
  return 1;
}

int get8colavrpixel(picinfo *info, picdatarows data, int x, int y, int pwidth, int pheight)
{
  /* Returns average pixel 4bit colour if >=0 or error if not */
  unsigned long r, g, b, a;
  bigpixel apixel;
  int ret;
  
  /* Get average pixel */
  if ((ret = getavrpixel(&apixel, info, data, x, y, pwidth, pheight)) != 1) return ret;
  
  r = apixel.avr_r;
  g = apixel.avr_g;
  b = apixel.avr_b;
  a = apixel.avr_a;
  
  /* Make it a 1-bit colour */
  /*unsigned long n = 1;
  n = n << apixel.bitdepth; //This was the old algorithm - it didn't work very well...
  n = n >> 1;
  r -= n;
  g -= n;
  b -= n;
  a -= n;
  */
  
  int n = (apixel.bitdepth - 1);
  if (n)
  {
    r = r >> n;
    g = g >> n;
    b = b >> n;
    a = a >> n;
  }
  
  int retcol = 0;
  if (r>0) retcol |= 4;
  if (g>0) retcol |= 2;
  if (b>0) retcol |= 1;
  if (a>0) retcol |= 8;
  
  return retcol;
}

int getbrightavrpixel(picinfo *info, picdatarows data, int x, int y, int pwidth, int pheight)
{
  /* Returns average pixel brightness if >=0 or error if not */
  bigpixel apixel;
  int ret;
  
  /* Get average pixel */
  if ((ret = getavrpixel(&apixel, info, data, x, y, pwidth, pheight)) != 1) return ret;
  
  /* Get average brightness */
  unsigned long bright = (apixel.avr_r + apixel.avr_g + apixel.avr_b) / 3;
  /* unsigned long n = 1;
  n = n << apixel.bitdepth; //Another failed algorithm
  n = n >> 3;
  n--;
  n = ~n;
  bright &= n; */
  
  int n = apixel.bitdepth - 3;
  if (n > 0) bright = bright >> n;
  else if (n < 0)
  {
    n = -n;
    bright << n;
  }
  return bright;
}

int main(int argc, char *argv[])
{
  picinfo *info;
  picdatarows data;
  
  if (argc != 5)
  {
    printf("PNGToTxtImg - Converts a PNG to a coloured ASCII art image.\n");
    printf("2019 DHeadshot's Software Creations\n");
    printf("Usage:\n  %s <PNGFile> <OutputFile> <HPixelsPerChar> <VPixelsPerChar>\n", argv[0]);
    printf("Where:\n  <PNGFile> is a PNG file.\n  <OutputFile> is the file to write to.\n");
    printf("  <HPixelsPerChar> is the number of horizontal pixels to use per character.\n");
    printf("  <VPixelsPerChar> is the number of vertical pixels to use per character.\n");
    return 1;
  }
  
  int hppc, vppc;
  hppc = atoi(argv[3]);
  vppc = atoi(argv[4]);
  if (hppc<1)
  {
    fprintf(stderr, "Horizontal Pixels Per Character must be at least 1 (not %d)!\n", hppc);
    return 2;
  }
  if (vppc<1)
  {
    fprintf(stderr, "Vertical Pixels Per Character must be at least 1 (not %d)!\n", vppc);
    return 2;
  }
  
  info = newpicinfo(); /*(picinfo *) malloc(sizeof(picinfo));*/
  if (!info)
  {
    fprintf(stderr, "Out of Memory!\n");
    return 3;
  }
  
  int ret = readpng(info, &data, argv[1]);
  if (ret < 1)
  {
    free(info);
    return 4;
  }
  
  FILE *afp;
  afp = fopen(argv[2], "w");
  if (!afp)
  {
    fprintf(stderr, "Could not open output file \"%s\"!\n", argv[2]);
    freepicdata(data, info->height);
    freepicinfo(info);
    return 5;
  }
  
  
  int txtw, txth;
  
  /* Width and height rounded up */
  txtw = ((int) ((((double) info->width) / ((double) hppc)) + 0.99));
  txth = ((int) ((((double) info->height) / ((double) vppc)) + 0.99));
  
  ntsa txtimg = (ntsa) malloc(sizeof(char *)*(1+txth));
  if (!txtimg)
  {
    fprintf(stderr, "Out of Memory!\n");
    freepicdata(data, info->height);
    freepicinfo(info);
    fclose(afp);
    return 3;
  }
  
  int i, j;
  for (i=0;i<txth;i++)
  {
    txtimg[i] = (char *) malloc(sizeof(char)*(1+(2*txtw)));
    if (!txtimg)
    {
      for (j=0;j<i;j++) free(txtimg[j]);
      free(txtimg);
      fprintf(stderr, "Out of Memory for text image!\n");
      freepicdata(data, info->height);
      freepicinfo(info);
      fclose(afp);
      return 3;
    }
    memset(txtimg[i], 0, (sizeof(char)*(1+(2*txtw))));
  }
  txtimg[i] = NULL;
  
  char colchar = 0x80, brtchar = ' ';
  int brt = 0;
  ret = 0;
  
  for (j = 0; j<txth; j++)
  {
    for (i=0; i<(2*txtw); i+=2)
    {
      colchar = (char) get8colavrpixel(info, data, ((i>>1)*hppc), (j*vppc), hppc, vppc);
      if ((colchar & 0x80) != 0)
      {
        ret = -1;
        break;
      }
      colchar |= 0x80;
      brt = getbrightavrpixel(info, data, ((i>>1)*hppc), (j*vppc), hppc, vppc);
      if (brt<0) brt = 0;
      if (brt>7) brt = 7;
      brtchar = charscale[brt];
      txtimg[j][i] = colchar;
      txtimg[j][i+1] = brtchar;
    }
    if (ret<0) break;
  }
  if (ret<0)
  {
    for (i=0;i<txth;i++) free(txtimg[i]);
    free(txtimg);
    freepicdata(data, info->height);
    freepicinfo(info);
    fclose(afp);
    return 6;
  }
  
  for (i=0; i<txth; i++)
  {
    fprintf(afp, "%s\r\n", txtimg[i]);
  }
  
  fclose(afp);
  printf("%dx%d PNG file converted to %dx%d Text Image.\n", info->width, info->height, txtw, txth);
  
  for (i=0;i<txth;i++) free(txtimg[i]);
  free(txtimg);
  freepicdata(data, info->height);
  freepicinfo(info);
  
  return 0;
}
