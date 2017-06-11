#include "ads1015.h"
#include "myi2c.h"

static char buf[10];

static char* mux_type[]=
{
    "Ain_p=Ain0 & Ain_n=Ain1",
    "Ain_p=Ain0 & Ain_n=Ain3",
    "Ain_p=Ain1 & Ain_n=Ain3",
    "Ain_p=Ain2 & Ain_n=Ain3",
    "Ain_p=Ain0 & Ain_n=gnd",
    "Ain_p=Ain1 & Ain_n=gnd",
    "Ain_p=Ain2 & Ain_n=gnd",
    "Ain_p=Ain3 & Ain_n=gnd"
};

/*** Samples per second ***/
static const unsigned int data_rates[8] = {128, 250, 490, 920, 1600, 2400, 3300, 3300};

/*** PGA fullscale voltages in mV ***/
static const unsigned int fullscale[8] = {6144, 4096, 2048, 1024, 512, 256, 256, 256 };

char* c_mode[]={"Continuous convert", "Single-shot convert"};

enum channel {CH_1, CH_2, CH_3, CH_4, MUX4, MUX5, MUX6, MUX7};
enum D_R {DR128SPS, DR250SPS, DR490SPS, DR920SPS, DR1600SPS, DR2400SPS, DR3300SPS, DR3301SPS};
enum FScale {mv6144, mv4096, mv2048, mv1024, mv512, mv256};

//myADS1015 ADC = {0x48, mv2048, 1, MUX4, DR250SPS};
myADS1015 ADC = {mv4096, 0, CH_1, DR250SPS};

myADS1015 arrADC[4]=  {
                    {mv4096, 0, CH_1, DR250SPS},
                    {mv2048, 1, CH_2, DR250SPS},
                    {mv1024, 1, CH_4, DR1600SPS},
                    {0, 0, 0, 0},
                };


/// this can be for single ended or diff ended, CR bit 14 selects which to use.
/// CR bit 14 = 0 for diff, =1 for single
ads_channel_data MUX_channels[4]= {
                    {CH_1, mv4096, DR250SPS},
                    {CH_2, mv2048, DR250SPS},
                    {CH_3, mv1024, DR1600SPS},
                    {CH_4, mv4096, DR250SPS},
                };



int16_t ads1015::read_config_reg()
{
    int16_t result = 0x1234;
    result = device_read_swap(Config_Reg);
    CR_Value = result;
    printf("read config register: %x \n", result);

    myADS1015 current_CR;
    current_CR.data_rate = (result & 0x0e0) >> CR_DR0;
    current_CR.Mux = (result & 0x7000) >> CR_MUX0;    current_CR.PGA = (result &0x0E00 ) >> CR_PGA0;

    arrADC[3].data_rate = (result & 0x0e0) >> CR_DR0;
    arrADC[3].Mux = (result & 0x7000) >> CR_MUX0;
    arrADC[3].PGA = (result &0x0E00 ) >> CR_PGA0;

    printf("current config reg is: %x\ndata rate is: %x\nmux is: %x\nPGA is: %x\n",
                    result,
                    arrADC[3].data_rate,
                    arrADC[3].Mux,
                    arrADC[3].PGA);

    printf("Mux number is: %s \n", mux_type[arrADC[3].Mux]);
    printf("PGA setting is: %d \n", fullscale[arrADC[3].PGA]);
    printf("Data rate is: %d \n", data_rates[arrADC[3].data_rate]);

       /* setup and start single conversion */
	//config &= 0x001f;
	//config |= (1 << 15) | (1 << 8);
	//config |= (channel & 0x0007) << 12;
	//config |= (pga & 0x0007) << 9;
	//config |= (data_rate & 0x0007) << 5;

    return result;
}


ads1015::ads1015(unsigned int bus, unsigned int address) :I2CBus(bus,address)
{
    ads1015_Init();
    s_ended = true; /// set to single ended
}

/**************************************
    Initialize bus and ADC
**************************************/
int ads1015::ads1015_Init()
{
    uint16_t init_config_reg = 0;
    //init_config_reg = mux_single_1 | PGA_2048 | DR_250sps | MODE_CONTINUOUS | COMP_QUE_DISABLE;
    init_config_reg = mux_single_1 | PGA_4096 | DR_250sps | MODE_CONTINUOUS | COMP_QUE_DISABLE;
    CR_Value = init_config_reg;
    int result = device_write_swap(Config_Reg, init_config_reg);
    return result;
}

/**< KEEP THIS ************** */
float ads1015 :: read_convert_register_volts(int channel)
{
    int t_pga = MUX_channels[channel].pga;
    int t_data = MUX_channels[channel].data_date;
    int t_channel = MUX_channels[channel].ch_numb;
    CR_Value &= 0x001f;
    CR_Value |= (0 << 15) | (MODE_CONTINUOUS << 8);
    CR_Value |= (t_channel & 0x007) << CHANNEL_SHIFT;
    CR_Value |= (t_data & 0x007) << DR_SHIFT;
    CR_Value |= (t_pga & 0x007) << PGA_SHIFT;
    CR_Value |= (s_ended << DIFF_SINGLE_SHIFT);
    printf("===CR from new way: x%x \n",CR_Value);
    int cr_result = device_write_swap(Config_Reg, CR_Value);
    setPGA(t_pga);
    usleep(4000); /// needs to be usecs
    unsigned int testdr = data_rates[t_data];


    int16_t result = 0x1234; // due to __s32 i2c_smbus_read_word_data(int file, __u8 command)
    result = device_read_swap(Convert_Reg);                /// read the ADC data
    printf("raw convert register count is: 0x%x\n", result);

    /************************************
        conversion is:
        (conversion register count)/16 (right shift 4 bits)
        then times the gain, this gives volt count times 1000
        then divide 1000 to set decimal place correctly
    ************************************/

    // check if negative
    if((result & SIGN_MASK) == SIGN_MASK)
        result = -(~(result)+1);

    //result = (result>>4)*2;         // assumes gain is 2
    result = (result>>4)*1;
    printf("===================");
    printf("the voltage (x1) is: %2.3f\n", (float)(result*volts_FS)/1000);
    float fresult = (float)(result*volts_FS)/1000;
    result = device_read_swap(Config_Reg);
    printf("the config register is: x%x\n", result);

    return fresult; //this returns voltage

}

void ads1015::setPGA(int range)
{
    uint16_t dataCR = read_config_reg();

    switch (range)
    {
        case mv6144:
            volts_FS = 3.0;
            printf("++++FS is 6144\n");
        break;
        case mv4096:
            volts_FS = 2.0; // each count represents 2.0 mV
            printf("+++FS is 4096\n");
        break;
        case mv2048:
            volts_FS = 1.0; // each count represents 1.0 mV
            printf("++++FS is 2048\n");
        break;
        case mv1024:
            volts_FS = 0.5;
            printf("++++FS is 1024\n");
        break;
        case mv512:
            volts_FS = 0.25;
        break;
        case mv256:
            volts_FS = 0.125;
        break;
        default:
            volts_FS = 1.0;
	  break;
  }
}

/*************/
int ads1015::ads1015_op_init()   // maybe not needed
{
    //unsigned int i = 1;
    //char *c = (char*)&i;
    //if (*c)
    //   printf("Little endian\n");
    //else
    //   printf("Big endian\n");;

    UINT result = 0x1234; // BS number
    UINT res = 0x4823;
    UINT result1 = 0x5678;

    //result=i2c_smbus_read_word_data(file, Config_Reg);
    result = device_read_swap(Config_Reg);
    printf("the op intit config register is x%x: \n", result);

    result1 = (result & 0x1000); // check if still converting or not
    if((result & 0x1000) == 0x1000)             // if (x & MASK) = MASK, ok
        printf("Not doing a conversion\n");
    else
        printf("conversion in process\n");

    result1 = ((result & 0x7000)>> 12);
    printf("Mux number is :%s \n", mux_type[result1]);       // which MUX type

    result1 = ((result >>5) & 0x0007);
    printf("Data rate is :%d \n", data_rates[result1]);        //data rate
    result1 = ((result >>9) & 0x0007);
    printf("PGA setting is :%d \n\n\n\n", fullscale[result1]);

	UINT config = res;
	config &= 0x001f;                   // strip for COMP items
	config |= (0 << 15) | (0 << 8);     // set for continuous
	config |= (ADC.Mux & 0x0007) << 12;
	config |= (ADC.PGA & 0x0007) << 9;
	config |= (ADC.data_rate & 0x0007) << 5;

    result1 = ((config & 0x7000)>> 12);
    printf("Mux number is :%s \n", mux_type[result1]);       // which MUX type

    result1 = (config >>5)& 0x0007;
    printf("Data rate is: %d SPS\n", data_rates[result1]);        //data rate

    result1 = (config >>9) & 0x0007;
    printf("PGA setting is: %d mv\n", fullscale[result1]);

    result1 = (config >>8)& 0x0001;
    printf("Current mode is: %s\n", c_mode[result1]);

    //=== get the byte order correct ====
    buf[2]=HBYTE(config);
    buf[3]=LBYTE(config);
    result=(buf[3]<<8) | buf[2];    //x = lobyte << 8 | hibyte;
    printf("byte order correct config reg: 0x%x\n", result);

    //result=i2c_smbus_write_word_data(file, Config_Reg, result);
    //result=myI2C_write_swap(file, Config_Reg, result);

    //result=i2c_smbus_write_word_data(file, Config_Reg,0x4302);
    //result=i2c_smbus_write_word_data(file, Config_Reg,config);

    //result=i2c_smbus_read_word_data(file, Config_Reg);
    //result=read_config_reg(file);
    //result = myI2C_read_swap(file, Config_Reg);

    //uint16_t sdc = bswap_16(result); // grt the right order to display
    printf("the op intit config register is x%x: \n", result);

    return 0;
}


