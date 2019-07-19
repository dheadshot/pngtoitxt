#ifndef __MAIN_H__
#define __MAIN_H__ 1

typedef struct bigpixel_struct {
  unsigned long avr_r; //Red
  unsigned long avr_g; //Green
  unsigned long avr_b; //Blue
  unsigned long avr_a; //Alpha
  unsigned int bitdepth;
} bigpixel;

typedef char ** ntsa;

int getavrpixel(bigpixel *apix, picinfo *info, picdatarows data, int x, int y, int pwidth, int pheight);
int get8colavrpixel(picinfo *info, picdatarows data, int x, int y, int pwidth, int pheight);
int getbrightavrpixel(picinfo *info, picdatarows data, int x, int y, int pwidth, int pheight);


#endif
