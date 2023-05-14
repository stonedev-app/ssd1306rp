#include "ssd1306.h"
#include "ssd1306_font.h"

// commands (see datasheet)
#define SSD1306_SET_MEM_MODE _u(0x20)
#define SSD1306_SET_COL_ADDR _u(0x21)
#define SSD1306_SET_PAGE_ADDR _u(0x22)
#define SSD1306_SET_HORIZ_SCROLL _u(0x26)
#define SSD1306_SET_SCROLL _u(0x2E)

#define SSD1306_SET_DISP_START_LINE _u(0x40)

#define SSD1306_SET_CONTRAST _u(0x81)
#define SSD1306_SET_CHARGE_PUMP _u(0x8D)

#define SSD1306_SET_SEG_REMAP _u(0xA0)
#define SSD1306_SET_ENTIRE_ON _u(0xA4)
#define SSD1306_SET_ALL_ON _u(0xA5)
#define SSD1306_SET_NORM_DISP _u(0xA6)
#define SSD1306_SET_INV_DISP _u(0xA7)
#define SSD1306_SET_MUX_RATIO _u(0xA8)
#define SSD1306_SET_DISP _u(0xAE)
#define SSD1306_SET_COM_OUT_DIR _u(0xC0)
#define SSD1306_SET_COM_OUT_DIR_FLIP _u(0xC0)

#define SSD1306_SET_DISP_OFFSET _u(0xD3)
#define SSD1306_SET_DISP_CLK_DIV _u(0xD5)
#define SSD1306_SET_PRECHARGE _u(0xD9)
#define SSD1306_SET_COM_PIN_CFG _u(0xDA)
#define SSD1306_SET_VCOM_DESEL _u(0xDB)

#define SSD1306_PAGE_HEIGHT _u(8)

#define SSD1306_WRITE_MODE _u(0xFE)
#define SSD1306_READ_MODE _u(0xFF)

static uint8_t reversed[sizeof(font)] = {0};

static void send_cmd(SSD1306Disp *p, uint8_t cmd);
static void send_cmd_list(SSD1306Disp *p, uint8_t *buf, int num);
static void send_buf(SSD1306Disp *p);
static void write_char(SSD1306Disp *p, int x, int y, uint8_t ch);
static int get_font_index(uint8_t ch);
static uint8_t reverse(uint8_t b);
static void fill_reversed_cache();

bool ssd1306_init(SSD1306Disp *p, uint8_t width, uint8_t height,
                  uint8_t address, i2c_inst_t *i2c_instance)
{
    p->width = width;
    p->height = height;
    p->pages = height / SSD1306_PAGE_HEIGHT;
    p->address = address;
    p->i2c_i = i2c_instance;
    p->bufsize = (p->pages) * (p->width);
    p->bufsize = (p->pages) * (p->width);
    if ((p->buffer = malloc(p->bufsize + 1)) == NULL)
    {
        p->bufsize = 0;
        return false;
    }
    ++(p->buffer);

    // Some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like
    // Some configuration values are recommended by the board manufacturer
    uint8_t cmds[] = {
        SSD1306_SET_DISP, // set display off
        /* memory mapping */
        SSD1306_SET_MEM_MODE, // set memory address mode 0 = horizontal, 1 = vertical, 2 = page
        0x00,                 // horizontal addressing mode
        /* resolution and layout */
        SSD1306_SET_DISP_START_LINE,    // set display start line to 0
        SSD1306_SET_SEG_REMAP | 0x01,   // set segment re-map, column address 127 is mapped to SEG0
        SSD1306_SET_MUX_RATIO,          // set multiplex ratio
        p->height - 1,                  // Display height - 1
        SSD1306_SET_COM_OUT_DIR | 0x08, // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
        SSD1306_SET_DISP_OFFSET,        // set display offset
        0x00,                           // no offset
        SSD1306_SET_COM_PIN_CFG,        // set COM (common) pins hardware configuration. Board specific magic number.
                                        // 0x02 Works for 128x32, 0x12 Possibly works for 128x64. Other options 0x22, 0x32
        p->height == SSD1306_HEIGHT_64 ? 0x12 : 0x02,
        /* timing and driving scheme */
        SSD1306_SET_DISP_CLK_DIV, // set display clock divide ratio
        0x80,                     // div ratio of 1, standard freq
        SSD1306_SET_PRECHARGE,    // set pre-charge period
        0xF1,                     // Vcc internally generated on our board
        SSD1306_SET_VCOM_DESEL,   // set VCOMH deselect level
        0x30,                     // 0.83xVcc
        /* display */
        SSD1306_SET_CONTRAST, // set contrast control
        0xFF,
        SSD1306_SET_ENTIRE_ON,     // set entire display on to follow RAM content
        SSD1306_SET_NORM_DISP,     // set normal (not inverted) display
        SSD1306_SET_CHARGE_PUMP,   // set charge pump
        0x14,                      // Vcc internally generated on our board
        SSD1306_SET_SCROLL | 0x00, // deactivate horizontal scrolling if set. This is necessary as memory writes will corrupt if scrolling was enabled
        SSD1306_SET_DISP | 0x01,   // turn display on
    };

    send_cmd_list(p, cmds, count_of(cmds));

    return true;
}

void ssd1306_deinit(SSD1306Disp *p)
{
    free(p->buffer - 1);
}

void ssd1306_clear(SSD1306Disp *p)
{
    memset(p->buffer, 0, p->bufsize);
}

void ssd1306_show(SSD1306Disp *p)
{
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR,
        0,
        p->width - 1,
        SSD1306_SET_PAGE_ADDR,
        0,
        p->pages - 1};

    send_cmd_list(p, cmds, count_of(cmds));
    send_buf(p);
}

void ssd1306_set_pixel(SSD1306Disp *p, int x, int y, bool on)
{
    if (x < 0 || p->width <= x || y < 0 || p->height <= y)
        return;

    const int BytesPerRow = p->width; // x pixels, 1bpp, but each row is 8 pixel high, so (x / 8) * 8

    int byte_idx = (y / 8) * BytesPerRow + x;
    uint8_t byte = *(p->buffer + byte_idx);

    if (on)
        byte |= 1 << (y % 8);
    else
        byte &= ~(1 << (y % 8));

    *(p->buffer + byte_idx) = byte;
}

void ssd1306_draw_line(SSD1306Disp *p, int x0, int y0, int x1, int y1, bool on)
{

    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (true)
    {
        ssd1306_set_pixel(p, x0, y0, on);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void ssd1306_write_string(SSD1306Disp *p, int x, int y, char *str)
{
    // Cull out any string off the screen
    if (x > p->width - 8 || y > p->height - 8)
        return;

    while (*str)
    {
        write_char(p, x, y, *str++);
        x += 8;
    }
}

static void send_cmd(SSD1306Disp *p, uint8_t cmd)
{
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(p->i2c_i, (p->address & SSD1306_WRITE_MODE), buf, 2, false);
}

static void send_cmd_list(SSD1306Disp *p, uint8_t *buf, int num)
{
    for (int i = 0; i < num; i++)
        send_cmd(p, buf[i]);
}

static void send_buf(SSD1306Disp *p)
{
    *(p->buffer - 1) = 0x40;
    i2c_write_blocking(p->i2c_i, (p->address & SSD1306_WRITE_MODE),
                       p->buffer - 1, p->bufsize + 1, false);
}

static void write_char(SSD1306Disp *p, int x, int y, uint8_t ch)
{
    if (reversed[0] == 0)
        fill_reversed_cache();

    if (x > p->width - 8 || y > p->height - 8)
        return;

    // For the moment, only write on Y row boundaries (every 8 vertical pixels)
    y = y / 8;

    ch = toupper(ch);
    int idx = get_font_index(ch);
    int fb_idx = y * p->width + x;

    for (int i = 0; i < 8; i++)
    {
        *(p->buffer + fb_idx++) = reversed[idx * 8 + i];
    }
}

static int get_font_index(uint8_t ch) {
    if (ch >= 'A' && ch <='Z') {
        return  ch - 'A' + 1;
    }
    else if (ch >= '0' && ch <='9') {
        return  ch - '0' + 27;
    }
    else return  0; // Not got that char so space.
}

static uint8_t reverse(uint8_t b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

static void fill_reversed_cache()
{
    // calculate and cache a reversed version of fhe font, because I defined it upside down...doh!
    for (int i = 0; i < sizeof(font); i++)
        reversed[i] = reverse(font[i]);
}