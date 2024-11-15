
#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <pico/binary_info.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Escritor.h"
#include "font.h"



void Escritor_escribe(Escritor_t *p, uint8_t val);

inline static void swap(int32_t *a, int32_t *b) {
    int32_t *t=a;
    *a=*b;
    *b=*t;
}

inline static void fancy_write(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, char *name) {
    switch(i2c_write_blocking(i2c, addr, src, len, false)) {
    case PICO_ERROR_GENERIC:
        printf("[%s] addr not acknowledged!\n", name);
        break;
    case PICO_ERROR_TIMEOUT:
        printf("[%s] timeout!\n", name);
        break;
    default:
        //printf("[%s] wrote successfully %lu bytes!\n", name, len);
        break;
    }
}
inline static void Escritor_envia_comando(Escritor_t *p, uint8_t val);

void Escritor_escribe_pixel(Escritor_t *p, uint32_t x, uint32_t y) {
    if(x>=p->ancho || y>=p->alto) return;

    p->buffer[x+p->ancho*(y>>3)]|=0x1<<(y&0x07); // y>>3==y/8 && y&0x7==y%8
}

inline static void Escritor_escribe_comando(Escritor_t *p, uint8_t val) {
    uint8_t d[2]= {0x00, val};
    fancy_write(p->i2c_i, p->dire, d, 2, "Escritor_escribe");
}

bool Escritor_init(Escritor_t *p, uint16_t ancho, uint16_t alto, uint8_t dire, i2c_inst_t *i2c_instance) {
    p->ancho=ancho;
    p->alto=alto;
    p->pages=alto/8;
    p->dire=dire;

    p->i2c_i=i2c_instance;


    p->bufsize=(p->pages)*(p->ancho);
    if((p->buffer=malloc(p->bufsize+1))==NULL) {
        p->bufsize=0;
        return false;
    }


    ++(p->buffer);

    uint8_t cmds[]= {
        SET_DISP,
        // timing and driving scheme
        SET_DISP_CLK_DIV,
        0x80,
        SET_MUX_RATIO,
        alto - 1,
        SET_DISP_OFFSET,
        0x00,
        // resolution and layout
        SET_DISP_START_LINE,
        // charge pump
        SET_CHARGE_PUMP,
        p->external_vcc?0x10:0x14,
        SET_SEG_REMAP | 0x01,           // column addr 127 mapped to SEG0
        SET_COM_OUT_DIR | 0x08,         // scan from COM[N] to COM0
        SET_COM_PIN_CFG,
        ancho>2*alto?0x02:0x12,
        // display
        SET_CONTRAST,
        0xff,
        SET_PRECHARGE,
        p->external_vcc?0x22:0xF1,
        SET_VCOM_DESEL,
        0x30,                           // or 0x40?
        SET_ENTIRE_ON,                  // output follows RAM contents
        SET_NORM_INV,                   // not inverted
        SET_DISP | 0x01,
        // dire setting
        SET_MEM_ADDR,
        0x00,  // horizontal
    };

    for(size_t i=0; i<sizeof(cmds); ++i)
        Escritor_escribe_comando(p, cmds[i]);

    return true;
}

inline void Escritor_deinit(Escritor_t *p) {
    free(p->buffer-1);
}

void Escritor_escribe_square(Escritor_t *p, uint32_t x, uint32_t y, uint32_t ancho, uint32_t alto) {
    for(uint32_t i=0; i<ancho; ++i)
        for(uint32_t j=0; j<alto; ++j)
            Escritor_escribe_pixel(p, x+i, y+j);
}

void Escritor_escribe_char_with_font(Escritor_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, char c) {
    if (c < font[3] || c > font[4]) return;  // Fuera de rango en la tabla ASCII de la fuente

    uint8_t font_height = font[0]; // Altura de la fuente
    uint8_t font_width = font[1];  // Ancho de la fuente
    uint8_t parts_per_line = (font_height + 7) / 8; // Cantidad de bytes necesarios por columna de carácter
    uint32_t index = 5 + (c - font[3]) * font_width * parts_per_line; // Ajuste del índice para caracteres multibyte

    // Dibujado del carácter según la altura y el ancho
    for (uint8_t w = 0; w < font_width; ++w) {
        for (uint8_t h = 0; h < parts_per_line; ++h) {
            uint8_t line = font[index + w * parts_per_line + h];
            for (int8_t bit = 0; bit < 8; ++bit) {
                if (line & (1 << bit)) {
                    Escritor_escribe_square(p, x + w * scale, y + (h * 8 + bit) * scale, scale, scale);
                }
            }
        }
    }
}

void Escritor_escribe_string_with_font(Escritor_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s) {
    uint8_t char_spacing = font[2];
    uint8_t char_width = font [1];
    for(int32_t x_n=x; *s; x_n+=(char_width + char_spacing )*scale) {
        Escritor_escribe_char_with_font(p, x_n, y, scale, font, *(s++));
    }
}


inline void Escritor_poweroff(Escritor_t *p) {
    Escritor_escribe_comando(p, SET_DISP|0x00);
}

inline void Escritor_poweron(Escritor_t *p) {
    Escritor_escribe_comando(p, SET_DISP|0x01);
}

inline void Escritor_contrast(Escritor_t *p, uint8_t val) {
    Escritor_escribe(p, SET_CONTRAST);
    Escritor_escribe(p, val);
}

inline void Escritor_invert(Escritor_t *p, uint8_t inv) {
    Escritor_escribe(p, SET_NORM_INV | (inv & 1));
}

inline void Escritor_limpia (Escritor_t *p) {
    memset(p->buffer, 0, p->bufsize);
}

void Escritor_limpia_pixel (Escritor_t *p, uint32_t x, uint32_t y) {
    if(x>=p->ancho || y>=p->alto) return;

    p->buffer[x+p->ancho*(y>>3)]&=~(0x1<<(y&0x07));
}



void Escritor_escribe_string(Escritor_t *p, uint32_t x, uint32_t y, uint32_t scale, const char *s) {
    Escritor_escribe_string_with_font(p, x, y, scale, font_8x5, s);
                                                    // font = arreglo de mapeo
}

void Escritor_screen(Escritor_t *p) {
    uint8_t payload[]= {SET_COL_ADDR, 0, p->ancho-1, SET_PAGE_ADDR, 0, p->pages-1};
    if(p->ancho==64) {
        payload[1]+=32;
        payload[2]+=32;
    }

    for(size_t i=0; i<sizeof(payload); ++i)
        Escritor_escribe_comando(p, payload[i]);

    *(p->buffer-1)=0x40;

    fancy_write(p->i2c_i, p->dire, p->buffer-1, p->bufsize+1, "Escritor_mostrar");
}
inline void Escritor_limp(Escritor_t *p) {
    memset(p->buffer, 0, p->bufsize);
}

void Escritor_mostrar (Escritor_t *p) {
    uint8_t payload[]= {SET_COL_ADDR, 0, p->ancho-1, SET_PAGE_ADDR, 0, p->pages-1};
    if(p->ancho==64) {
        payload[1]+=32;
        payload[2]+=32;
     }

    for(size_t i=0; i<sizeof(payload); ++i)
        Escritor_escribe_comando(p, payload[i]);

    *(p->buffer-1)=0x40;

    fancy_write(p->i2c_i, p->dire, p->buffer-1, p->bufsize+1, "Escritor muestra"); 

}

