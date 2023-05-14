#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include "ssd1306.h"

#define I2C_SDA 8
#define I2C_SCL 9
#define I2C_PORT i2c0
#define I2C_CLK 400 * 1000 // 400Khz
#define I2C_SSD1306_ADDRESS 0x3C

int main()
{
    stdio_init_all();

    // useful information for picotool
    bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
    bi_decl(bi_program_description("SSD1306 OLED driver I2C example for the Raspberry Pi Pico"));

    // I2C Initialisation.
    i2c_init(I2C_PORT, I2C_CLK);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    SSD1306Disp disp;

    // run through the complete initialization process
    ssd1306_init(&disp, SSD1306_WIDTH_128, SSD1306_HEIGHT_64,
                 I2C_SSD1306_ADDRESS, I2C_PORT);
    ssd1306_clear(&disp);
    ssd1306_show(&disp);
    sleep_ms(1000);

    ssd1306_set_pixel(&disp, 10, 10, true);
    ssd1306_show(&disp);
    sleep_ms(1000);

    return 0;
}
