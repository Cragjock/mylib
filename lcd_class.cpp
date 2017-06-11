


#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include "lcd_class.h"
#include "myi2c.h"

#define DELAY_PULSE_NS 1000 // 1us
#define DELAY_SETTLE_NS 40000 // 40us
#define DELAY_CLEAR_NS 2600000L // 2.6ms
#define DELAY_SETUP_0_NS 15000000L // 15ms
#define DELAY_SETUP_1_NS 5000000L // 5ms
#define DELAY_SETUP_2_NS 1000000L // 1ms

// Adafruit backpack I2C
#ifdef MCP23008
#define PIN_RW 0        // output 0, no connection on 595, grounded on LCD input
#define PIN_RS 1        // output 1
#define PIN_E 2         // output 2
#define PIN_D4 3        // output 6
#define PIN_D5 4        // output 5
#define PIN_D6 5        // output 4
#define PIN_D7 6        // output 3
#define PIN_BKL 7       // output 7

#define DATA_MASK 0x87
#endif // MCP23008

#ifdef ADA595           // Adafruit backpack SPI
#define PIN_RW 0        // output 0, no connection on 595, grounded on LCD input
#define PIN_RS 1        // output 1
#define PIN_E 2         // output 2
#define PIN_D7 3        // output 3
#define PIN_D6 4        // output 4
#define PIN_D5 5        // output 5
#define PIN_D4 6        // output 6
#define PIN_BKL 7       // output 7
#define DATA_MASK 0x87
#endif // ADA595

#ifdef PFC8574T
#define PIN_RW 1        // output 0, no connection on 595, grounded on LCD input
#define PIN_RS 0        // output 1
#define PIN_E 2        // output 2
#define PIN_D7 7        // output 3
#define PIN_D6 6        // output 4
#define PIN_D5 5        // output 5
#define PIN_D4 4        // output 6
#define PIN_BKL 7       // output 7
#define DATA_MASK 0x0f
#endif // PFC8574T

#ifdef PICADMCP23S17
#define PIN_RW 0        // output 0, no connection on 595, grounded on LCD input
#define PIN_RS 1        // output 1
#define PIN_E 2         // output 2
#define PIN_D7 3        // output 3
#define PIN_D6 4        // output 4
#define PIN_D5 5        // output 5
#define PIN_D4 6        // output 6
#define PIN_BKL 7       // output 7
#define DATA_MASK 0xf0
#endif // PICADMCP23S17


#define Bit_Set 1
#define Bit_Clear 0

#define RS_on 0x04
#define RS_off 0x00
#define E_on 0x08
#define E_off 0x00

// LCD commands
#define LCD_CLEARDISPLAY 0x01       // RS = 0
#define LCD_RETURNHOME 0x02         // RS = 0
#define LCD_ENTRYMODESET 0x04       // RS = 0
#define LCD_DISPLAYCONTROL 0x08     // RS = 0
#define LCD_CURSORSHIFT 0x10        // RS = 0
#define LCD_FUNCTIONSET 0x20        // RS = 0
#define LCD_SETCGRAMADDR 0x40       // RS = 0
#define LCD_SETDDRAMADDR 0x80       // RS = 0

//#define LCD_WRITECGDDRAM            // RS = 1, no specific command data
#define LCD_NEWLINE 0xC0
#define LCD_SPACE 0x20

// entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// display control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// not a specific command
#define BKL_ON 0x80
#define BKL_OFF 0x00

// display shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5X10DOTS 0x04
#define LCD_5X8DOTS 0x00

//#define LCD_MAX_LINES 2
//#define LCD_WIDTH 16


#define LCD_MAX_LINES 4
#define LCD_WIDTH 20
#define LCD_RAM_WIDTH 80 // RAM is 80 wide

static const uint8_t ROW_OFFSETS[] = {0, 0x40, 0x14, 0x54};

/************************************************************
* LCD DDRAM space (hex address) and row-column
*     1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20
*    +---------+--------+--------+----------+--------------------+
* 1  |00|01|02|03|04|05|06|07|08|09|0A|0B|0C|0D|0E|0F|10|11|12|13|
*    +---------+--------+--------+----------+--------------------+
* 2  |40|41|42|43|44|45|46|47|48|49|4A|4B|4C|4D|4E|4F|50|51|52|53|
*    +---------+--------+--------+----------+--------------------+
* 3  |14|15|16|17|18|19|1A|1B|1C|1D|1E|1F|20|21|22|23|24|25|26|27|
*    +---------+--------+--------+----------+--------------------+
* 4  |54|55|56|57|58|59|5A|5B|5C|5D|5E|5F|60|61|62|63|64|65|67|68|
*    +---------+--------+--------+----------+--------------------+


*     1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20
*    +---------+--------+--------+----------+--------------------+
* 1  |00|01|02|03|04|05|06|07|08|09|10|11|12|13|14|15|16|17|18|19|
*    +---------+--------+--------+----------+--------------------+
* 2  |64|65|66|67|68|69|70|71|72|73|74|75|76|77|78|79|80|81|82|83|
*    +---------+--------+--------+----------+--------------------+
* 3  |20|21|22|23|24|25|26|27|28|29|30|31|32|33|34|35|36|37|38|39|
*    +---------+--------+--------+----------+--------------------+
* 4  |84|85|86|87|88|89|90|91|92|93|94|95|96|97|98|99|100|101|102|103|
*    +---------+--------+--------+----------+--------------------+

*********/


/************************************************************
* Adafruit 74HC595 backpack SPI Latch port looks like:
* +---------+----+----+----+--------------------+
* | 7       | 6  | 5  | 4  | 3  | 2 | 1  | 0  |
* +---------+----+----+----+--------------------+
* | bklight | db4|db5 |db6 |db7 | E | RS | N/A|
* +---------+----+----+----+--------------------+
*    NOTE: DB4-7 are in backwards order, need to flip

// ==================================================
* Adafruit backpack I2C MCP23008 port looks like:
* +---------+-----+----+----+--------------------+
* | GP7     | GP6 |GP5 |GP4 |GP3 |GP2 |GP1 |GP0  |
* +---------+-----+----+----+--------------------+
* | bklight | db7 |db6 |db5 |db4 | E  | RS | N/A|
* +---------+-----+----+----+--------------------+
*
//=======================================
* SainSmart I2C PFC8574T port looks like:
* +-----+-----+----+----+--------------------+
* | GP7 | GP6 |GP5 |GP4 |GP3 |GP2 |GP1 |GP0  |
* +-----+-----+----+----+--------------------+
* | db7 | db6 |db5 |db4 |BKL | E  | RW | RS |
* +---------+-----+----+----+--------------------+
*
//=======================================
*   PifaceCad SPI MCP23s17
    PORT A ====
* +-----+-----+----+----+--------------------+
* | GP7 | GP6 |GP5 |GP4 |GP3 |GP2 |GP1 |GP0  |
* +-----+-----+----+----+--------------------+
* | SWR | SWL |SWctr |SW5 |SW4 |SW3 |SW2|SW1|
* +---------+-----+----+----+----------------+
    PORT B =====
* +-----+-----+----+----+--------------------+
* | GP7 | GP6 |GP5 |GP4 |GP3 |GP2 |GP1 |GP0  |
* +-----+-----+----+----+--------------------+
* | BKL | RS |RW |E |db7 |db6 |db5 | db4 |
* +---------+-----+----+----+----------------+
*
***********************************************************/

lcddisplay::lcddisplay()

{
    LCDBus=new mcp23008(1,0x20);

    function_set = 0;
    entry_mode = 0;
    display_control = 0;



    this->lcd_open();
    setbmp();
}


lcddisplay::~lcddisplay()
{
    lcd_close();
}


//static void sleep_ns(long nanoseconds);
static int max(int a, int b);
static int min(int a, int b);


/*************************************************/
int lcddisplay::lcd_open(void)
{
    lcd_init();
    return 5;

    //return I2C_fp;
    //return SPI_fd;
}

void lcddisplay::lcd_close(void)
{
    lcd_clear();
    lcd_send_command(LCD_DISPLAYCONTROL | LCD_CURSOROFF);
    lcd_backlight_off();
    //close(I2C_fp);
    LCDBus->closemcp23008();
    //close(SPI_fd);
}

/*******************************************/
void lcddisplay::lcd_init(void)
{
    function_set = 0;
    //int res = myI2C_write_data(I2C_fp, GPIO, 0x00);
    int res = LCDBus->device_write(GPIO, 0x00);

// === setup sequence per HD44780 spec, page 46
    sleep_ns(DELAY_SETUP_0_NS);     // 15ms
/***** Special function case 1 ***************************************/
/*** Function set, 8 bit interface ***/
    function_set |= (LCD_8BITMODE | BKL_ON);                // TEST
    lcd_send_command8(LCD_FUNCTIONSET | function_set);      // TEST

    //res = myI2C_write_data(I2C_fp, GPIO, 0x98);
    //lcd_pulse_enable();

    sleep_ns(DELAY_SETUP_2_NS);     // 1ms
/***** Special function case 2 ***************************************/
/*** Function set, 8 bit interface ***/
    function_set |= (LCD_8BITMODE | BKL_ON);                // TEST
    lcd_send_command8(LCD_FUNCTIONSET | function_set);      // TEST

    //res = myI2C_write_data(I2C_fp, GPIO, 0x98);
    //lcd_pulse_enable();
    sleep_ns(DELAY_SETUP_2_NS);     // 1ms

/***** Special function case 3 **************************************/
/*** Function set, 8 bit interface ***/
    function_set |= (LCD_8BITMODE | BKL_ON);                // TEST
    lcd_send_command8(LCD_FUNCTIONSET | function_set);      // TEST

    //res = myI2C_write_data(I2C_fp, GPIO, 0x18);
    //lcd_pulse_enable();
    sleep_ns(DELAY_SETUP_2_NS);     // 1ms

/***** Initial function set for 4 bits etc **************************/
/*** Function set, 4 bit interface ***/
    function_set = LCD_4BITMODE | BKL_ON;   // TEST but had to change |= to =
    lcd_send_command8(LCD_FUNCTIONSET | function_set);      // TEST


    //res = myI2C_write_data(I2C_fp, GPIO, 0x10);
    //lcd_pulse_enable();

    sleep_ns(DELAY_SETUP_2_NS);     // 1ms

/**************** Now in 4 bit mode ********************************/
/*** and now normal operation can start ***/
    function_set |= LCD_4BITMODE | LCD_2LINE | LCD_5X8DOTS;
    //res = myI2C_write_data(I2C_fp, GPIO, 0x90);
    lcd_send_command(LCD_FUNCTIONSET | function_set); // 0x28 command
    sleep_ns(DELAY_SETTLE_NS);

    display_control |= LCD_DISPLAYOFF | LCD_CURSOROFF | LCD_BLINKOFF;
    lcd_send_command(LCD_DISPLAYCONTROL | display_control); // x08 command
    //res = myI2C_write_data(I2C_fp, GPIO, 0x80);

    lcd_clear();

    entry_mode |= LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    lcd_send_command(LCD_ENTRYMODESET | entry_mode);
    //SPI_write_reg(entry_mode, SPI_fd);
    sleep_ns(DELAY_SETTLE_NS);

    display_control |= LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON; //x0f
    lcd_send_command(LCD_DISPLAYCONTROL | display_control);
    //SPI_write_reg(display_control, SPI_fd);
    sleep_ns(DELAY_SETTLE_NS);

/*** Initialization done *****/

    lcd_write("LCD Initialization\ncomplete\n");

}

/***********************************************************/
uint8_t lcddisplay::lcd_write(const char * message)
{
    lcd_send_command(LCD_SETDDRAMADDR | cursor_address);

    // for each character in the message
    while (*message) {
        if (*message == '\n')
        {
            int newrow = address2row(cursor_address);
            //lcd_set_cursor(0, (newrow+1));   // lcd_set_cursor(0, 1);
            bool cur_test = cursor_address & 0x40;
            if(cur_test)
            {
                if(cursor_address>83)
                    newrow=0;
                else
                    newrow=2;
            }
            else
            {
                if(cursor_address>19)
                    newrow=3;
                else
                    newrow=1;
            }
            //lcd_set_cursor(ROW_OFFSETS[newrow]);
            lcd_set_cursor_address(ROW_OFFSETS[newrow]);
          }
        else
        {
            lcd_send_data(*message);
            cursor_address++;

/**< cursor position check due to main 154. it should work */
            if(cursor_address == 40)
            {
                cursor_address=84;
                lcd_send_command(LCD_SETDDRAMADDR | cursor_address);
            }
            else if(cursor_address == 20)
            {
                cursor_address=64;
                lcd_send_command(LCD_SETDDRAMADDR | cursor_address);
            }
            else if(cursor_address == 84)
            {
                cursor_address=20;
                lcd_send_command(LCD_SETDDRAMADDR | cursor_address);
            }
            else if(cursor_address == 104)
            {
                cursor_address=0;
                lcd_send_command(LCD_SETDDRAMADDR | cursor_address);
            }
/**< end cursor position check */

        }
        //printf("cursor: %d\ncol from cursor: %d\nrow from cursor: %d\n*****\n",cursor_address,address2col(cursor_address),address2row(cursor_address) );
        message++;
    }
    return cursor_address;
}

/**************************************************************/
uint8_t lcddisplay::lcd_set_cursor(uint8_t col, uint8_t row)
{
    col = max(0, min(col, (LCD_RAM_WIDTH / 2) - 1));
    row = max(0, min(row, LCD_MAX_LINES - 1));
    uint8_t whatever = colrow2address(col, row) ;
    lcd_set_cursor_address(whatever);
    //lcd_set_cursor_address(colrow2address(col, row));
    return cursor_address;
}

/************************************************************/
void lcddisplay::lcd_set_cursor_address(uint8_t address)
{
    //printf("cur address before mod x%x\n", address);
    cursor_address = address % 103;
    //cursor_address = address % LCD_RAM_WIDTH;
    //printf("cur address after mod x%x\n", cursor_address);
    lcd_send_command(LCD_SETDDRAMADDR | cursor_address);
}

uint8_t lcddisplay::lcd_get_cursor_address(void)
{
    return cursor_address;
}

 /*******************************************************************/
void lcddisplay::lcd_clear(void)
{
    lcd_send_command(LCD_CLEARDISPLAY);
    sleep_ns(DELAY_CLEAR_NS);		/* 2.6 ms  - added JW 2014/06/26 */
    cursor_address = 0;
}
/*******************************************************************/

void lcddisplay::lcd_home(void)
{
    lcd_send_command(LCD_RETURNHOME);
    sleep_ns(DELAY_CLEAR_NS);		/* 2.6 ms  - added JW 2014/06/26 */
    cursor_address = 0;
}

void lcddisplay::lcd_display_on(void)
{
    display_control |= LCD_DISPLAYON;
    lcd_send_command(LCD_DISPLAYCONTROL | display_control);
}

void lcddisplay::lcd_display_off(void)
{
    display_control &= 0xff ^ LCD_DISPLAYON;
    lcd_send_command(LCD_DISPLAYCONTROL | display_control);
}

//void lcddisplay::lcd_blink_on(void)
//{
//   display_control |= LCD_BLINKON;
//    lcd_send_command(LCD_DISPLAYCONTROL | display_control);
//}

//void lcddisplay::lcd_blink_off(void)
//{
//    display_control &= 0xff ^ LCD_BLINKON;
//    lcd_send_command(LCD_DISPLAYCONTROL | display_control);
//}

//void lcddisplay::lcd_cursor_on(void)
//{
//    display_control |= LCD_CURSORON;
//    lcd_send_command(LCD_DISPLAYCONTROL | display_control);
//}

//void lcddisplay::lcd_cursor_off(void)
//{
//    display_control &= 0xff ^ LCD_CURSORON;
//    lcd_send_command(LCD_DISPLAYCONTROL | display_control);
//}

void lcddisplay::lcd_backlight_on(void)
{
    LCDBus->set_pin_state(Bit_Set, PIN_BKL);
}

void lcddisplay::lcd_backlight_off(void)
{
    LCDBus->set_pin_state(Bit_Clear, PIN_BKL);
}

void lcddisplay::lcd_write_custom_bitmap(uint8_t location)
{
    lcd_send_command(LCD_SETDDRAMADDR | cursor_address);
    lcd_send_data(location);
    cursor_address++;
}

void lcddisplay::lcd_store_custom_bitmap(uint8_t location, uint8_t bitmap[])
{
    location &= 0x7; // we only have 8 locations 0-7
    lcd_send_command(LCD_SETCGRAMADDR | (location << 3));
    int i;
    for (i = 0; i < 8; i++)
    {
        lcd_send_data(bitmap[i]);
    }
}

void lcddisplay::setbmp(void)
{
/******** BITMAP SET UP ************************/
    lcd_send_command(LCD_DISPLAYCONTROL | LCD_DISPLAYOFF | LCD_CURSOROFF | LCD_BLINKOFF); // x08 command
    uint8_t bmLeft[]= {8,12,10,9,10,12,8,0};
    uint8_t bmMiddle[]={0,0,31,14,4,14,31,0};
    uint8_t bmRight[]={2,6,10,18,10,6,2,0};
    uint8_t bmSatLeft[]={0,20,21,21,31,21,20,20};
    uint8_t bmSatright[]= {0,5,21,21,31,21,5,5};
    uint8_t bmhand[]= {4,14,30,31,31,31,14,14};
    uint8_t bmCheck[] = {0,1,3,22,28,8,0,0};
    uint8_t bmXXX[] = {0,27,14,4,12,27,0,0};

/*****
    uint8_t filled[] = {31,31,31,31,31,31,31,31};
    uint8_t space[] = {0,0,0,0,0,0,0,0};
    uint8_t upper_half_filled[] = {31,31,31,31,0,0,0,0};
    uint8_t lower_half_filled[] = {0,0,0,0,31,31,31,31};
    uint8_t upperright[] = {24,28,30,31,31,31,31,0};
    uint8_t lowerright[] = {31,31,31,31,31,30,28,24};
    uint8_t half_upper_right[] = {31,30,28,24,0,0,0,0};
    uint8_t half_lower_right[] = {0,0,0,0,31,30,28,24};
    uint8_t upperleft[] = {3,7,15,31,31,31,31,0};
    uint8_t lowerleft[] = {31,31,31,31,15,7,3,0};
    uint8_t half_upper_left[] = {31,15,7,3,0,0,0,0};
    uint8_t half_lower_left[] = {0,0,0,0,31,31,15,7};

    lcd_store_custom_bitmap(1, filled);
    lcd_store_custom_bitmap(2, upper_half_filled);
    lcd_store_custom_bitmap(3, lower_half_filled);
    lcd_store_custom_bitmap(4, upperleft);
    lcd_store_custom_bitmap(5, half_lower_left);
    lcd_store_custom_bitmap(6, half_upper_left);
    lcd_store_custom_bitmap(7, half_lower_right);
    lcd_store_custom_bitmap(8, half_upper_right);

//#define full 0x01                   // need
//#define half_fill_upper 0x02        // need
//#define half_fill_lower 0x03        // need
//#define full_L_upper 0x04           // need
//#define full_L_lower
//#define half_L_lower 0x05             // need
//#define half_L_upper 0x06
//#define full_R_upper
//#define half_R_lower 0x07               // need
//#define half_R_upper 0x08

                                                // col row
    lcd_set_cursor(1,0);                        // 1, 0 col row
    lcd_write_custom_bitmap(full_L_upper);      // 1, 0 col row
    lcd_write_custom_bitmap(half_fill_upper);   // 2, 0 col row
    lcd_write_custom_bitmap(half_fill_upper);   // 2, 0 col row
    lcd_write_custom_bitmap(half_fill_upper);   // 4, 0 col row
    lcd_write_custom_bitmap(half_fill_upper);   // 5, 0 col row
    lcd_set_cursor(8,0);
    lcd_write_custom_bitmap(full_L_upper);      // 8, 0 col row
    lcd_write_custom_bitmap(half_fill_upper);   // 9, 0 col row
    lcd_write_custom_bitmap(half_fill_upper);   // 10, 0 col row
    lcd_write_custom_bitmap(half_fill_upper);   // 11, 0 col row
    lcd_write_custom_bitmap(full);   // 12, 0 col row

    lcd_set_cursor(1,1);// col row              // 1, 1 col row
    lcd_write_custom_bitmap(full);              // 1, 1 col row
    //lcd_write_custom_bitmap(half_fill_lower);   // 2, 1 col row
    //lcd_write_custom_bitmap(half_fill_lower);   // 3, 1 col row
    //lcd_write_custom_bitmap(half_fill_lower);   // 4, 1 col row
    //lcd_write_custom_bitmap(half_fill_lower);   // 5, 1 col row
    lcd_set_cursor(8,1);
    lcd_write_custom_bitmap(full);              // 8, 0 col row


    lcd_set_cursor(1,2);                        // 1, 2 col row
    lcd_write_custom_bitmap(half_L_upper);      // 1, 2 col row
    lcd_write_custom_bitmap(half_fill_upper);   // 2, 2 col row
    lcd_write_custom_bitmap(half_fill_upper);   // 3, 2 col row
    lcd_write_custom_bitmap(half_fill_upper);   // 4, 2 col row
    lcd_write_custom_bitmap(full);              // 5, 2 col row
    lcd_set_cursor(8,2);
    lcd_write_custom_bitmap(full);              // 8, 0 col row

    lcd_set_cursor(1,3);                        //1, 3 col row
    lcd_write_custom_bitmap(half_L_lower);      //1, 3 col row
    lcd_write_custom_bitmap(half_fill_lower);   //2, 3 col row
    lcd_write_custom_bitmap(half_fill_lower);   //3, 3 col row
    lcd_write_custom_bitmap(half_fill_lower);   //4, 3 col row
    lcd_write_custom_bitmap(full);              //5, 3 col row
    lcd_set_cursor(8,3);
    lcd_write_custom_bitmap(half_L_upper);      //1, 3 col row
    lcd_write_custom_bitmap(half_fill_upper);   //2, 3 col row
    lcd_write_custom_bitmap(half_fill_upper);   //3, 3 col row
    lcd_write_custom_bitmap(half_fill_upper);   //4, 3 col row
    lcd_write_custom_bitmap(half_fill_upper);              //5, 3 col row
***/

    lcd_store_custom_bitmap(1, bmLeft); // store
    lcd_store_custom_bitmap(2, bmMiddle); // store
    lcd_store_custom_bitmap(3, bmRight); // store
    lcd_store_custom_bitmap(4, bmSatLeft); // store
    lcd_store_custom_bitmap(5, bmSatright); // store
    lcd_store_custom_bitmap(6, bmhand); // store
    lcd_store_custom_bitmap(7, bmCheck); // store
    lcd_store_custom_bitmap(0, bmXXX); // store

    lcd_send_command(LCD_RETURNHOME);
    lcd_send_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON);

}












void lcddisplay::lcd_send_command(uint8_t command)
{
    LCDBus->set_pin_state(Bit_Clear, PIN_RS);

    lcd_send_byte(command);
    sleep_ns(DELAY_SETTLE_NS);
}

void lcddisplay::lcd_send_command8(uint8_t command)
{
    //set_hw_bit(Bit_Clear, PIN_RS, I2C_fp);
    lcd_send_word(command);
    sleep_ns(DELAY_SETTLE_NS);
}

void lcddisplay::lcd_send_data(uint8_t data)
{
    //lcd_set_rs(1);
    LCDBus->set_pin_state(Bit_Set, PIN_RS);
    //set_hw_bit(Bit_Set, PIN_RS);

    //printf("Set RS\n");
    lcd_send_byte(data);
    sleep_ns(DELAY_SETTLE_NS);
}

void lcddisplay::lcd_send_byte(uint8_t b)
{
    //uint8_t current_state = myI2C_read_data(I2C_fp, GPIO);
    uint8_t current_state = LCDBus->device_read(GPIO);
    current_state &= DATA_MASK; // clear the data bits

    //send high nibble (0bXXXX0000)
    uint8_t new_byte = current_state | ((b & 0xf0) >> 1);
    //SPI_write_reg(new_byte, SPI_fd);
    //int res = myI2C_write_data(I2C_fp, GPIO, new_byte);
    int res = LCDBus->device_write(GPIO, new_byte);


    //printf("high nibble 0x%x\n", new_byte);
    lcd_pulse_enable();

    //send low nibble (0b0000XXXX)
    new_byte = current_state | ((b & 0x0f)<<3);  // set nibble GOOD
    //printf("low nibble 0x%x\n", new_byte);
    //res = myI2C_write_data(I2C_fp, GPIO, new_byte);
    res = LCDBus->device_write(GPIO, new_byte);
    lcd_pulse_enable();

}
void lcddisplay::lcd_send_word(uint8_t mydata) /// for init only
{
    uint8_t current_state = mydata & DATA_MASK; // clear the data bits and preserve other settings
    uint8_t new_byte = current_state | ((mydata & 0x78) >> 1);
    int res = LCDBus->device_write(GPIO, new_byte);
    lcd_pulse_enable();
}

void lcddisplay::lcd_set_backlight(uint8_t state)
{
    int32_t rec = LCDBus->device_read(GPIO);
}

/* pulse the enable pin */
void lcddisplay::lcd_pulse_enable(void)
{
    LCDBus->set_pin_state(Bit_Set, PIN_E);
    //LCDBus->set_pin(PIN_E);

    sleep_ns(DELAY_PULSE_NS);

    LCDBus->set_pin_state(Bit_Clear, PIN_E);
    //LCDBus->clear_pin(PIN_E);

    sleep_ns(DELAY_PULSE_NS);
}


uint8_t lcddisplay::colrow2address(uint8_t col, uint8_t row)
{
    return col + ROW_OFFSETS[row];
}

uint8_t lcddisplay::address2col(uint8_t address)
{
    if(address > 0x40)
    {
        address = address - 0x40;
    }
    return address % LCD_WIDTH;   // #define LCD_WIDTH 20
    //return address % ROW_OFFSETS[1];
}

uint8_t lcddisplay::address2row(uint8_t address)
{
    int MASK = 0x40;
    int row=0;
    int i =0;
    int value;

    if((address & MASK)== MASK)
    {
        row=1;
        if(address>83)
            return row=3;
    }
    else
    {   row=0;
        if(address>13)
            return row=2;
    }



    if(address > ROW_OFFSETS[3])    // greater than 0x54
    {
        return row=3;
    }
    for(i=0; i<4; i++)
    {
        value = cursor_address - ROW_OFFSETS[i];
        if((value < 19) && (value >= 0) )
        {
            row = i;
            return row;
        }
        row=i; // ??????
    }
    address > ROW_OFFSETS[1] ? 1 : 0;
    return row;
    //return address > ROW_OFFSETS[1] ? 1 : 0;
}

void lcddisplay::sleep_ns(long nanoseconds)
{
    struct timespec time0, time1;
    time0.tv_sec = 0;
    time0.tv_nsec = nanoseconds;
    nanosleep(&time0 , &time1);
    //return 0;
}

static int max(int a, int b)
{
    return a > b ? a : b;
}

static int min(int a, int b)
{
    return a < b ? a : b;
}


uint8_t lcddisplay::flip(uint8_t data)
{
    char flip = data;  //   0b01100110; // starting data in
    char mirror=   0b00000000; // flipped data
    char mask =    0b00000001;

    mirror = ((mirror<<1) + (flip & mask));
    flip = flip >> 1;
    mirror = ((mirror<<1) + (flip & mask));
    flip = flip >> 1;
    mirror = ((mirror<<1) + (flip & mask));
    flip = flip >> 1;
    mirror = ((mirror<<1) + (flip & mask));
    flip = flip >> 1;
    mirror = ((mirror<<1) + (flip & mask));;
    flip = flip >> 1;
    mirror = ((mirror<<1) + (flip & mask));
    flip = flip >> 1;
    mirror = ((mirror<<1) + (flip & mask));
    flip = flip >> 1;
    mirror = ((mirror<<1) + (flip & mask));

    return mirror;

}
