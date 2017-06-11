

#include<iostream>
#include "mcp23008.h"
//using namespace std;



//mcp23008::mcp23008(unsigned int bus, unsigned int address, std::string bname):I2CBus(bus,address)
mcp23008::mcp23008(unsigned int bus, unsigned int address) :I2CBus(bus,address)
{

    mcp23008bus=0;
    //int readdata = device_read(GPIO);
// now set the MCP registers ======

    const uint8_t ioconfig = SEQOP_OFF | DISSLW_OFF | HAEN_ON | ODR_OFF | INTPOL_LOW;
    //int res = myI2C_write_data(I2C_fp, IOCON, ioconfig);
    int res = device_write(IOCON, ioconfig);
    res = device_write(IODIR, 0x00);
    res = device_write(IPOL, 0x00);
    // disable interrupts and no need for DEFVAL, INTCON
    res = device_write(GPINTEN, 0x00);  // disable ints
    std::cout<<"ptrfile in mcp consrtutor "<<ptrfile<<std::endl;

}

mcp23008::~mcp23008()
{

}


void mcp23008::closemcp23008()
{
   // ::close(this->ptrfile);
}




void mcp23008::set_pin_state(uint8_t state, uint8_t pin_number)
{
    GPIO_state = device_read(GPIO);

    switch (state)
    {
        case 1:
            GPIO_state = GPIO_state | (1<< pin_number);
            device_write(GPIO, GPIO_state);
		break;
        case 0:
            GPIO_state &= 0xff ^ (1<< pin_number);
            device_write(GPIO, GPIO_state);
		break;
        default:
		std::cout<<"total lost on state"<<std::endl;
		break;
	}
}
void mcp23008::set_pin(uint8_t pin_number)    // set to 1
{
    GPIO_state = device_read(GPIO);
    GPIO_state = GPIO_state | (1<< pin_number);
    device_write(GPIO, GPIO_state);

}
void mcp23008::clear_pin(uint8_t pin_number)  // set to 0
{
    GPIO_state = device_read(GPIO);
    GPIO_state &= 0xff ^ (1<< pin_number);
    device_write(GPIO, GPIO_state);

}
void mcp23008::toggle_pin(uint8_t pin_number) // invert current state
{
    GPIO_state = device_read(GPIO);

}






