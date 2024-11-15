

#ifndef _inc_Escritor
#define _inc_Escritor
#include <pico/stdlib.h>
#include <hardware/i2c.h>


typedef enum {
    SET_CONTRAST = 0x81,
    SET_ENTIRE_ON = 0xA4,
    SET_NORM_INV = 0xA6,
    SET_DISP = 0xAE,
    SET_MEM_ADDR = 0x20,
    SET_COL_ADDR = 0x21,
    SET_PAGE_ADDR = 0x22,
    SET_DISP_START_LINE = 0x40,
    SET_SEG_REMAP = 0xA0,
    SET_MUX_RATIO = 0xA8,
    SET_COM_OUT_DIR = 0xC0,
    SET_DISP_OFFSET = 0xD3,
    SET_COM_PIN_CFG = 0xDA,
    SET_DISP_CLK_DIV = 0xD5,
    SET_PRECHARGE = 0xD9,
    SET_VCOM_DESEL = 0xDB,
    SET_CHARGE_PUMP = 0x8D
} Escritor_command_t;

typedef struct {
    uint8_t ancho; 		/**< ancho of display */
    uint8_t alto; 	/**< alto of display */
    uint8_t pages;	
    uint8_t dire; 	/**< i2c dires of display*/
    i2c_inst_t *i2c_i; 	/**< i2c connection instance */
    bool external_vcc; 	/**< whether display uses external vcc */ 
    uint8_t *buffer;	/**< display buffer */
    size_t bufsize;		/**< buffer size */
} Escritor_t;



bool Escritor_init(Escritor_t *p, uint16_t ancho, uint16_t alto, uint8_t dire, i2c_inst_t *i2c_instance);

void Escritor_deinit(Escritor_t *p);

void Escritor_poweroff(Escritor_t *p);


void Escritor_poweron(Escritor_t *p);


void Escritor_contrast(Escritor_t *p, uint8_t val);



void Escritor_invert(Escritor_t *p, uint8_t inv);


void Escritor_mostrar(Escritor_t *p);


void Escritor_limp(Escritor_t *p);



void Escritor_limp_pixel(Escritor_t *p, uint32_t x, uint32_t y);


void Escritor_escribe_pixel(Escritor_t *p, uint32_t x, uint32_t y);





void Escritor_escribe_char(Escritor_t *p, uint32_t x, uint32_t y, uint32_t scale, char c);

void Escritor_escribe_char_with_font(Escritor_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, char c);

/**
	@brief draw string with given font

	@param[in] p : instance of display
	@param[in] x : x starting position of text
	@param[in] y : y starting position of text
	@param[in] scale : scale font to n times of original size (default should be 1)
	@param[in] font : pointer to font
	@param[in] s : text to draw
*/
void Escritor_escribe_string_with_font(Escritor_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s );

/**
	@brief draw string with builtin font

	@param[in] p : instance of display
	@param[in] x : x starting position of text
	@param[in] y : y starting position of text
	@param[in] scale : scale font to n times of original size (default should be 1)
	@param[in] s : text to draw
*/
void Escritor_escribe_string(Escritor_t *p, uint32_t x, uint32_t y, uint32_t scale, const char *s);

void Escritor_escribe_square(Escritor_t *p, uint32_t x, uint32_t y, uint32_t ancho, uint32_t alto);

#endif

