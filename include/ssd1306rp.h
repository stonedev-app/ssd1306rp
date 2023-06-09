#ifndef __SSD1306_RP_H
#define __SSD1306_RP_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define SSD1306_HEIGHT_64 64
#define SSD1306_HEIGHT_32 32
#define SSD1306_WIDTH_128 128

typedef struct
{
    uint8_t width;
    uint8_t height;
    uint8_t pages;
    uint8_t address;
    i2c_inst_t *i2c_i;
    uint8_t *buffer;
    size_t bufsize;
} SSD1306Disp;

bool ssd1306_init(SSD1306Disp *p, uint8_t width, uint8_t height,
                  uint8_t address, i2c_inst_t *i2c_instance);
void ssd1306_deinit(SSD1306Disp *p);
void ssd1306_clear(SSD1306Disp *p);
void ssd1306_show(SSD1306Disp *p);
void ssd1306_set_pixel(SSD1306Disp *p, int x, int y, bool on);
void ssd1306_draw_line(SSD1306Disp *p, int x0, int y0, int x1, int y1, bool on);
void ssd1306_write_string(SSD1306Disp *p, int x, int y, char *str);

#endif