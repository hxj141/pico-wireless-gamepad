#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include <tusb.h>   // TinyUSB tud_cdc_connected()

#define PIN_MISO   4
#define PIN_CS     14
#define PIN_CE     17
#define PIN_SCK    6
#define PIN_MOSI   7
 
#define SPI_PORT   spi0
#define READ_BIT   0x00
#define WRITE_BIT  0x20

// SPI Commands
#define nrf24l01_R_REGISTER		0x00
#define nrf24l01_W_REGISTER		0x20
#define nrf24l01_R_REGISTER_WID 0x61
#define nrf24l01_R_RX_PL_WID    0x60
#define nrf24l01_R_RX_PAYLOAD	0x61
#define nrf24l01_W_TX_PAYLOAD	0xA0
#define nrf24l01_FLUSH_TX		0xE1
#define nrf24l01_FLUSH_RX		0xE2
#define nrf24l01_REUSE_TX_PL	0xE3
#define nrf24l01_NOP			0xFF

// Register definitions
#define nrf24l01_CONFIG			0x00
#define nrf24l01_EN_AA			0x01
#define nrf24l01_EN_RXADDR		0x02
#define nrf24l01_SETUP_AW		0x03
#define nrf24l01_SETUP_RETR		0x04
#define nrf24l01_RF_CH			0x05
#define nrf24l01_RF_SETUP		0x06
#define nrf24l01_STATUS			0x07
#define nrf24l01_OBSERVE_TX		0x08
#define nrf24l01_RPD            0x09
#define nrf24l01_RX_ADDR_P0		0x0A 
#define nrf24l01_RX_ADDR_P1		0x0B
#define nrf24l01_RX_ADDR_P2		0x0C
#define nrf24l01_RX_ADDR_P3		0x0D
#define nrf24l01_RX_ADDR_P4		0x0E
#define nrf24l01_RX_ADDR_P5		0x0F
#define nrf24l01_TX_ADDR		0x10
#define nrf24l01_RX_PW_P0		0x11
#define nrf24l01_RX_PW_P1		0x12
#define nrf24l01_RX_PW_P2		0x13
#define nrf24l01_RX_PW_P3		0x14
#define nrf24l01_RX_PW_P4		0x15
#define nrf24l01_RX_PW_P5		0x16
#define nrf24l01_FIFO_STATUS	0x17
#define nrf24l01_DYNPD          0x1C
#define nrf24l01_FEATURE        0x1D

//CONFIG register bitwise definitions
#define nrf24l01_CONFIG_RESERVED	0x80
#define	nrf24l01_CONFIG_MASK_RX_DR	0x40
#define	nrf24l01_CONFIG_MASK_TX_DS	0x20
#define	nrf24l01_CONFIG_MASK_MAX_RT	0x10
#define	nrf24l01_CONFIG_EN_CRC		0x08
#define	nrf24l01_CONFIG_CRCO		0x04
#define	nrf24l01_CONFIG_PWR_UP		0x02
#define	nrf24l01_CONFIG_PRIM_RX		0x00

//EN_AA register bitwise definitions
#define nrf24l01_EN_AA_RESERVED		0xC0
#define nrf24l01_EN_AA_ENAA_ALL		0x3F
#define nrf24l01_EN_AA_ENAA_P5		0x20
#define nrf24l01_EN_AA_ENAA_P4		0x10
#define nrf24l01_EN_AA_ENAA_P3		0x08
#define nrf24l01_EN_AA_ENAA_P2		0x04
#define nrf24l01_EN_AA_ENAA_P1		0x02
#define nrf24l01_EN_AA_ENAA_P0		0x01
#define nrf24l01_EN_AA_ENAA_NONE	0x00

//EN_RXADDR register bitwise definitions
#define nrf24l01_EN_RXADDR_RESERVED	0xC0
#define nrf24l01_EN_RXADDR_ERX_ALL	0x3F
#define nrf24l01_EN_RXADDR_ERX_P5	0x20
#define nrf24l01_EN_RXADDR_ERX_P4	0x10
#define nrf24l01_EN_RXADDR_ERX_P3	0x08
#define nrf24l01_EN_RXADDR_ERX_P2	0x04
#define nrf24l01_EN_RXADDR_ERX_P1	0x02
#define nrf24l01_EN_RXADDR_ERX_P0	0x01
#define nrf24l01_EN_RXADDR_ERX_NONE	0x00

//SETUP_AW register bitwise definitions
#define nrf24l01_SETUP_AW_RESERVED	0xFC
#define nrf24l01_SETUP_AW			0x03
#define nrf24l01_SETUP_AW_5BYTES	0x03
#define nrf24l01_SETUP_AW_4BYTES	0x02
#define nrf24l01_SETUP_AW_3BYTES	0x01
#define nrf24l01_SETUP_AW_ILLEGAL	0x00

//SETUP_RETR register bitwise definitions
#define nrf24l01_SETUP_RETR_ARD			0xF0
#define nrf24l01_SETUP_RETR_ARD_4000	0xF0
#define nrf24l01_SETUP_RETR_ARD_3750	0xE0
#define nrf24l01_SETUP_RETR_ARD_3500	0xD0
#define nrf24l01_SETUP_RETR_ARD_3250	0xC0
#define nrf24l01_SETUP_RETR_ARD_3000	0xB0
#define nrf24l01_SETUP_RETR_ARD_2750	0xA0
#define nrf24l01_SETUP_RETR_ARD_2500	0x90
#define nrf24l01_SETUP_RETR_ARD_2250	0x80
#define nrf24l01_SETUP_RETR_ARD_2000	0x70
#define nrf24l01_SETUP_RETR_ARD_1750	0x60
#define nrf24l01_SETUP_RETR_ARD_1500	0x50
#define nrf24l01_SETUP_RETR_ARD_1250	0x40
#define nrf24l01_SETUP_RETR_ARD_1000	0x30
#define nrf24l01_SETUP_RETR_ARD_750		0x20
#define nrf24l01_SETUP_RETR_ARD_500		0x10
#define nrf24l01_SETUP_RETR_ARD_250		0x00
#define nrf24l01_SETUP_RETR_ARC			0x0F
#define nrf24l01_SETUP_RETR_ARC_15		0x0F
#define nrf24l01_SETUP_RETR_ARC_14		0x0E
#define nrf24l01_SETUP_RETR_ARC_13		0x0D
#define nrf24l01_SETUP_RETR_ARC_12		0x0C
#define nrf24l01_SETUP_RETR_ARC_11		0x0B
#define nrf24l01_SETUP_RETR_ARC_10		0x0A
#define nrf24l01_SETUP_RETR_ARC_9		0x09
#define nrf24l01_SETUP_RETR_ARC_8		0x08
#define nrf24l01_SETUP_RETR_ARC_7		0x07
#define nrf24l01_SETUP_RETR_ARC_6		0x06
#define nrf24l01_SETUP_RETR_ARC_5		0x05
#define nrf24l01_SETUP_RETR_ARC_4		0x04
#define nrf24l01_SETUP_RETR_ARC_3		0x03
#define nrf24l01_SETUP_RETR_ARC_2		0x02
#define nrf24l01_SETUP_RETR_ARC_1		0x01
#define nrf24l01_SETUP_RETR_ARC_0		0x00

//RF_CH register bitwise definitions
#define nrf24l01_RF_CH_RESERVED	0x80
#define nrf24l01_RF_CH_CH1      0x01

//RF_SETUP register bitwise definitions
#define nrf24l01_RD_SETUP_CONT_WAVE 0x80
#define nrf24l01_RF_SETUP_RESERVED	0xE0
#define nrf24l01_RF_SETUP_PLL_LOCK	0x10
#define nrf24l01_RF_SETUP_RF_DR		0x08
#define nrf24l01_RF_SETUP_RF_PWR	0x06
#define nrf24l01_RF_SETUP_RF_PWR_0	0x06
#define nrf24l01_RF_SETUP_RF_PWR_6 	0x04
#define nrf24l01_RF_SETUP_RF_PWR_12	0x02
#define nrf24l01_RF_SETUP_RF_PWR_18	0x00
#define nrf24l01_RF_SETUP_LNA_HCURR	0x01

//STATUS register bitwise definitions
#define nrf24l01_STATUS_RESERVED					0x80
#define nrf24l01_STATUS_RX_DR						0x40
#define nrf24l01_STATUS_TX_DS						0x20
#define nrf24l01_STATUS_MAX_RT						0x10
#define nrf24l01_STATUS_RX_P_NO						0x0E
#define nrf24l01_STATUS_RX_P_NO_RX_FIFO_NOT_EMPTY	0x0E
#define nrf24l01_STATUS_RX_P_NO_UNUSED				0x0C
#define nrf24l01_STATUS_RX_P_NO_5					0x0A
#define nrf24l01_STATUS_RX_P_NO_4					0x08
#define nrf24l01_STATUS_RX_P_NO_3					0x06
#define nrf24l01_STATUS_RX_P_NO_2					0x04
#define nrf24l01_STATUS_RX_P_NO_1					0x02
#define nrf24l01_STATUS_RX_P_NO_0					0x00
#define nrf24l01_STATUS_TX_FULL						0x01

//OBSERVE_TX register bitwise definitions
#define nrf24l01_OBSERVE_TX_PLOS_CNT	0xF0
#define nrf24l01_OBSERVE_TX_ARC_CNT		0x0F

//CD register bitwise definitions
#define nrf24l01_CD_RESERVED	0xFE
#define nrf24l01_CD_CD			0x01

//RX_PW_P0 register bitwise definitions
#define nrf24l01_RX_PW_P0_RESERVED	0xC0

//RX_PW_P0 register bitwise definitions
#define nrf24l01_RX_PW_P0_RESERVED	0xC0

//RX_PW_P1 register bitwise definitions
#define nrf24l01_RX_PW_P1_RESERVED	0xC0

//RX_PW_P2 register bitwise definitions
#define nrf24l01_RX_PW_P2_RESERVED	0xC0

//RX_PW_P3 register bitwise definitions
#define nrf24l01_RX_PW_P3_RESERVED	0xC0

//RX_PW_P4 register bitwise definitions
#define nrf24l01_RX_PW_P4_RESERVED	0xC0

//RX_PW_P5 register bitwise definitions
#define nrf24l01_RX_PW_P5_RESERVED	0xC0

//FIFO_STATUS register bitwise definitions
#define nrf24l01_FIFO_STATUS_RESERVED	0x8C
#define nrf24l01_FIFO_STATUS_TX_REUSE	0x40
#define nrf24l01_FIFO_STATUS_TX_FULL	0x20
#define nrf24l01_FIFO_STATUS_TX_EMPTY	0x10
#define nrf24l01_FIFO_STATUS_RX_FULL	0x02
#define nrf24l01_FIFO_STATUS_RX_EMPTY	0x01

// RF power definitions
#define nrf24l01_DR_LOW 0x20
#define nrf24l01_DR_MED 0x00
#define nrf24l01_DR_HIGH 0x08

// Send no data for when you read data from the radio
#define nrf24l01_SEND_CLOCK             0x00

#define LED_PIN  25

static inline void cs_select(){
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0); // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(){
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1); 
    asm volatile("nop \n nop \n nop");
}

static inline void ce_enable(){
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CE, 1); // Active high
    asm volatile("nop \n nop \n nop");
}

static inline void ce_disable(){
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CE, 0); 
    asm volatile("nop \n nop \n nop");
}

uint8_t writeSPI1(uint8_t byte_in){
    uint8_t *byte_out;
    spi_write_read_blocking(SPI_PORT, &byte_in, byte_out, 1);
    
    return byte_out;
}

uint8_t readSPI1(void){
    uint8_t *byte_out;
    spi_read_blocking(SPI_PORT, 2, &byte_out, 1);
    
    return byte_out;
}

uint8_t nrf24_readReg(uint8_t reg){
    uint8_t *buf;
    reg |= READ_BIT;
    cs_select();
    spi_write_blocking(SPI_PORT, &reg, 1);
    sleep_us(25);
    spi_read_blocking(SPI_PORT, 0, &buf, 1);
    cs_deselect();
    sleep_us(25);
    
    return buf;
}

static void nrf24_writeReg(uint8_t reg, uint8_t data){
    reg |= WRITE_BIT;
    cs_select();
    spi_write_blocking(SPI_PORT, &reg, 1);
    sleep_us(25);
    spi_write_blocking(SPI_PORT, &data, 1);
    cs_deselect();
    sleep_us(25);
}

void nrf24_Init(){
    uint8_t config;
    config = nrf24_readReg(nrf24l01_CONFIG); 
    
    //~ nrf24_writeReg(nrf24l01_CONFIG, config & ~nrf24l01_CONFIG_PWR_UP); // PWR-DWN while writing registers
    nrf24_writeReg(nrf24l01_CONFIG, 0x00); // PWR-DWN while writing registers
    nrf24_writeReg(nrf24l01_EN_AA, 0x3F); // enable auto-acknowledgements ShockBurst
    nrf24_writeReg(nrf24l01_EN_RXADDR, 0x03);
    nrf24_writeReg(nrf24l01_SETUP_AW, 0x03);
    nrf24_writeReg(nrf24l01_SETUP_RETR, 0x00);  // disable retransmit
    nrf24_writeReg(nrf24l01_RF_CH, 0x01);
    nrf24_writeReg(nrf24l01_RF_SETUP, 0x0F); // set RF to 2Mbps and 0 dB
    nrf24_writeReg(nrf24l01_STATUS,0x0E); 
    nrf24_writeReg(nrf24l01_OBSERVE_TX,0x00); 
    nrf24_writeReg(nrf24l01_RPD,0x00); 
    nrf24_writeReg(nrf24l01_RX_ADDR_P0,0xD2); 
    nrf24_writeReg(nrf24l01_RX_ADDR_P1,0xE1); 
    nrf24_writeReg(nrf24l01_RX_ADDR_P2,0xC3); 
    nrf24_writeReg(nrf24l01_RX_ADDR_P3,0xC4); 
    nrf24_writeReg(nrf24l01_RX_ADDR_P4,0xC5); 
    nrf24_writeReg(nrf24l01_RX_ADDR_P5,0xC6); 
    nrf24_writeReg(nrf24l01_TX_ADDR,0xD2);
    nrf24_writeReg(nrf24l01_RX_PW_P0,0x06); 
    nrf24_writeReg(nrf24l01_RX_PW_P1,0x06);
    nrf24_writeReg(nrf24l01_RX_PW_P2,0x00); 
    nrf24_writeReg(nrf24l01_RX_PW_P3,0x00); 
    nrf24_writeReg(nrf24l01_RX_PW_P4,0x00); 
    nrf24_writeReg(nrf24l01_RX_PW_P5,0x00); 
    nrf24_writeReg(nrf24l01_FIFO_STATUS,0x11); 
    nrf24_writeReg(nrf24l01_DYNPD,0x00);
    nrf24_writeReg(nrf24l01_FEATURE,0x00); 
    nrf24_writeReg(nrf24l01_CONFIG,0x0F); 
 
    sleep_us(1500); // delay for 1.5 ms to PWR-UP

}

void nrf24_flushTx(){
    writeSPI1(nrf24l01_FLUSH_TX);
    
    ce_enable();
    sleep_us(15);
    ce_disable();
}

void nrf24_flushRx(){
    writeSPI1(nrf24l01_FLUSH_RX);
    
    ce_enable();
    sleep_us(15);
    ce_disable();
}


void nrf24_writeTX_ADDR(){
    cs_select();
    writeSPI1(nrf24l01_W_REGISTER | nrf24l01_TX_ADDR); // combine address with W_REGISTER command
    writeSPI1(0xD2); // send address of data pipe for TX (should equal RX listener pipe)
    writeSPI1(0xF0); 
    writeSPI1(0xF0);
    writeSPI1(0xF0);
    writeSPI1(0xF0);
    cs_deselect();
}

void nrf24_writeRX_ADDR(){
    cs_select();
    writeSPI1(nrf24l01_W_REGISTER | nrf24l01_RX_ADDR_P0); // combine address with W_REGISTER command
    writeSPI1(0xD2); // send address of data pipe for TX (should equal RX listener pipe)
    writeSPI1(0xF0); 
    writeSPI1(0xF0);
    writeSPI1(0xF0);
    writeSPI1(0xF0);
    cs_deselect();
}

void open_RX_pipe(){
    cs_select();
    writeSPI1(nrf24l01_W_REGISTER | (nrf24l01_RX_ADDR_P0 + 1 ));
    writeSPI1(0xE1); // send address of data pipe for TX (should equal RX listener pipe)
    writeSPI1(0xF0); 
    writeSPI1(0xF0);
    writeSPI1(0xF0);
    writeSPI1(0xF0);
    cs_deselect();
}

void read_RX_pipe(){
    cs_select();
    writeSPI1(nrf24l01_R_REGISTER | (nrf24l01_TX_ADDR));
    writeSPI1(0x00); // send address of data pipe for TX (should equal RX listener pipe)
    writeSPI1(0x00); 
    writeSPI1(0x00);
    writeSPI1(0x00);
    writeSPI1(0x00);
    cs_deselect();
    sleep_ms(10);
}

void open_TX_pipe(){
    nrf24_writeTX_ADDR();
    nrf24_writeRX_ADDR();
    nrf24_writeReg(nrf24l01_SETUP_AW, nrf24l01_SETUP_AW_5BYTES); // set address width to 5 bytes
}

void start_listening(){
        open_RX_pipe();
        nrf24_flushRx();
        nrf24_flushTx();
        nrf24_writeReg(nrf24l01_CONFIG, 0x0F);
        nrf24_writeReg(nrf24l01_STATUS, 0x0E);
        ce_enable();
        sleep_us(135);
}

int main() {
    stdio_init_all();
      //wait until the CDC ACM (serial port emulation) is connected
      while (!tud_cdc_connected()) 
      {
         sleep_ms(10);
      }
  
    
    printf("nRF24L01 Receiver Program \n");
    
    // Uses SPI0 at 0.5MHz
    spi_init(SPI_PORT, 100*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    bi_decl(bi_3pins_with_func(PIN_MISO, PIN_MOSI, PIN_SCK, GPIO_FUNC_SPI));
    
    // Chip select is active-low, so we'll initialize it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // Chip enable is active-high, so we'll initialize it to a driven-low state
    gpio_init(PIN_CE);
    gpio_set_dir(PIN_CE, GPIO_OUT);
    gpio_put(PIN_CE, 0);
    // Make the CS and CE pin available to picotool
    bi_decl(bi_1pin_with_name(PIN_CS, "SPI CS"));
    bi_decl(bi_1pin_with_name(PIN_CE, "SPI CE"));
    
    // Initialize LED pin
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // ensures the nRF24 is on and in a defined state before configuring/initializing
    sleep_ms(100); 
    // Initialize nRF24L01+ module
    nrf24_Init();
    
    // put into receiver mode
    open_TX_pipe();
    open_RX_pipe();
    start_listening();
    
    int any;
    uint8_t data_in[6] = {1};
    
    // Receiver Loop
    while(1){
        nrf24_readReg(nrf24l01_CONFIG);
        any = !(nrf24_readReg(nrf24l01_FIFO_STATUS) & nrf24l01_FIFO_STATUS_RX_EMPTY);
        if (any){
            printf("\nData Received\n");
            if (spi_is_readable (spi1)) {
                printf("SPI is readable\n");
            }
             cs_select();
            writeSPI1(nrf24l01_R_REGISTER | (nrf24l01_R_RX_PAYLOAD));
            data_in[0] = readSPI1(); // read received 6 byte payload
            data_in[1] = readSPI1(); 
            data_in[2] = readSPI1();
            data_in[3] = readSPI1();
            data_in[4] = readSPI1();
            data_in[5] = readSPI1();
            cs_deselect();
            sleep_ms(10);
            printf("\nByte0: %u", data_in[0]);
            printf("\nByte1: %u", data_in[1]);
            printf("\nByte2: %u", data_in[2]);
            printf("\nByte3: %u", data_in[3]);
            printf("\nByte4: %u", data_in[4]);
            printf("\nByte5: %u", data_in[5]);
            // write RX_DR to clear receive flag
            nrf24_writeReg(nrf24l01_STATUS, nrf24l01_STATUS_RX_DR);
            sleep_ms(1);
            
            
        }
    }
    
    return 0;
}
