

#ifndef MYLCD_CLASS_H
#define MYLCD_CLASS_H
#include "mcp23008.h"

// Adafruit backpack I2C
//#ifdef MCP23008
#define PIN_RW 0        // output 0, no connection on 595, grounded on LCD input
#define PIN_RS 1        // output 1
#define PIN_E 2         // output 2
#define PIN_D4 3        // output 6
#define PIN_D5 4        // output 5
#define PIN_D6 5        // output 4
#define PIN_D7 6        // output 3
#define PIN_BKL 7       // output 7

#define DATA_MASK 0x87
//#endif // MCP23008

#define XXXBM 0x08
#define LeftBM 0x01
#define MiddleBM 0x02
#define RightBM 0x03
#define SatLeftBM 0x04
#define SatRightBM 0x05
#define HandBM 0x06
#define CheckBM 0x07

#define full 0x01
#define half_fill_upper 0x02
#define half_fill_lower 0x03
#define full_L_upper 0x04
//#define full_L_lower 0x04
#define half_L_lower 0x05
#define half_L_upper 0x06
//#define full_R_upper
#define half_R_lower 0x07
#define half_R_upper 0x08


// #include <stdint.h>

/** from stdint.h what about <cstdint> **/
/* Signed.  */
#ifndef __int8_t_defined
#define __int8_t_defined
typedef signed char     int8_t;
typedef short int       int16_t;
typedef int             int32_t;
#if __WORDSIZE == 64
typedef long int int64_t
#else
__extension__
typedef long long int int64_t
#endif // __WORDSIZE
#endif // __int8_t_defined



/* Unsigned.  */
typedef unsigned char	        uint8_t;
typedef unsigned short int	uint16_t;
//#ifndef __uint32_t_defined
#ifndef __uint32_t_defined
typedef unsigned int		uint32_t;
//# define __uint32_t_defined
#define __uint32_t_defined
#endif // __uint32_t_defined



    class lcddisplay
    {
    public:
        lcddisplay();           /**< default constructor */
        ~lcddisplay();          /**< default destructor */
        int lcd_open(void);     /**< lcd_init call within this */
        void lcd_clear(void);
        void lcd_home(void);
        uint8_t lcd_write(const char * message);
        uint8_t lcd_set_cursor(uint8_t col, uint8_t row);
        void lcd_set_cursor_address(uint8_t address);


    private:
        uint8_t height, width, cursor_col, cursor_row;
        uint8_t cursor_address;
        uint8_t entry_mode;
        uint8_t display_control;
        uint8_t display_shift;
        uint8_t function_set;
        mcp23008* LCDBus;


        void lcd_close(void);
        void lcd_init(void);
        uint8_t lcd_get_cursor_address(void);
        void lcd_display_on(void);
        void lcd_display_off(void);
        //void lcd_blink_on(void);
        //void lcd_blink_off(void);
        //void lcd_cursor_on(void);
        //void lcd_cursor_off(void);
        void lcd_backlight_on(void);
        void lcd_backlight_off(void);
        //void lcd_move_left(void);
        //void lcd_move_right(void);
        //void lcd_left_to_right(void);
        //void lcd_right_to_left(void);
        //void lcd_autoscroll_on(void);
        //void lcd_autoscroll_off(void);
        void setbmp(void);
        void lcd_write_custom_bitmap(uint8_t location);
        void lcd_store_custom_bitmap(uint8_t location, uint8_t bitmap[]);
        void lcd_set_backlight(uint8_t state);
        int set_hw_bit(uint8_t state, uint8_t pin_number);


        uint8_t colrow2address(uint8_t col, uint8_t row);
        uint8_t address2col(uint8_t address);
        uint8_t address2row(uint8_t address);


        void lcd_pulse_enable(void);
        void lcd_send_command(uint8_t command);
        void lcd_send_data(uint8_t data);
        void lcd_send_byte(uint8_t byte);
        void lcd_set_rs(uint8_t state);
        void lcd_set_enable(uint8_t state);
        uint8_t flip(uint8_t data);
        void lcd_send_word(uint8_t b);
        void lcd_send_command8(uint8_t command);
        void sleep_ns(long nanoseconds);


    };
#endif // MYLCD_CLASS_H
