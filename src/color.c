/*
 * Copyright (c) 2015 Javier Escalada Gómez
 */

#include "hex.h"


/*
 * Init colors
 */
bool color_enabled;

void init_colors(void)
{
    color_enabled = has_colors();
    if(color_enabled && color_level>0)
    {
        start_color();
        init_pair(1, COLOR_BLACK,   COLOR_BLACK);
        init_pair(2, COLOR_RED,     COLOR_BLACK);
        init_pair(3, COLOR_GREEN,   COLOR_BLACK);
        init_pair(4, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(5, COLOR_BLUE,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_CYAN,    COLOR_BLACK);
        init_pair(8, COLOR_WHITE,   COLOR_BLACK);

    }
}

/*
 * Bytes colors
 */

int get_byte_color(intmax_t address, char c)
{
    UNUSED(address);
    if ((unsigned char) c == 0x00)
        return ((color_level>1) ? COLOR_PAIR(5) : A_NORMAL);

    if (color_level>2)
      if (((unsigned char) c >= 0x20) && ((unsigned char) c <= 0x7E)) {
          return COLOR_PAIR(8);
      } else if ((unsigned char) c == 0xFF) {
          return COLOR_PAIR(2);
      } else return COLOR_PAIR(7);
    else return A_NORMAL;
}

void byte_color_on(intmax_t address, char c)
{
  if (color_enabled) {
      int attr = get_byte_color(address, c);
      wattron(windows->ascii, attr);
      wattron(windows->hex, attr);
  }
}

void byte_color_off(intmax_t address, char c)
{
  if (color_enabled) {
      int attr = get_byte_color(address, c);
      wattroff(windows->ascii, attr);
      wattroff(windows->hex, attr);
  }
}

/*
 * Address colors
 */

int get_address_color(intmax_t address)
{
  UNUSED(address);
  return ((color_level>0) ? COLOR_PAIR(4) : A_NORMAL);
}

void address_color_on(intmax_t address)
{
  if(color_enabled) {
      wattron(windows->address, get_address_color(address));
  }
}

void address_color_off(intmax_t address)
{  
  if(color_enabled) {
      wattron(windows->address, get_address_color(address));
  }
}
