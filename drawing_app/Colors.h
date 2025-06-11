#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>

// Array of RGB565 values -- 60 colors max
const uint16_t colors[] = {
  0x0000, // Black - Let first be always black
  0xFFFF, // White
  0xF800, // Red
  0xFC00, // Orange
  0xFFE0, // Yellow
  0x0400, // Green
  0x001F, // Blue
  0x8010, // Purple
  0xA145, // Brown
  0xFDF9, // Pink
  0x8410, // Gray
  0x07FF, // Cyan
  0xF81F, // Magenta
  0x07E0, // Lime
  0x4013, // Indigo
  0xFEA0, // Gold
  0xC618, // Silver
  0xFED4, // Peach
  0x867F, // Sky Blue
  0x4ED9, // Turquoise
  0x8400, // Olive
  0x0010, // Navy
  0x8000, // Maroon
  0xDDBB  // Lavender
};
#undef X

#define NUM_COLORS (sizeof(colors) / sizeof(colors[0]))

#endif
