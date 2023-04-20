#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tusb.h>   // TinyUSB tud_cdc_connected()
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "nrf_receiver.h"

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

void nrf_rx_init() { 
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
}

uint8_t *read_rx_report(int **rx_report) { 
    int any;
    any = 0; //Initialize any to 0 so we can loop
    *rx_report = (uint8_t *) malloc(6*sizeof(uint8_t));
    uint8_t data_in[6] = {1};
    
    // Receiver Loop
    nrf24_readReg(nrf24l01_CONFIG);
    any = !(nrf24_readReg(nrf24l01_FIFO_STATUS) & nrf24l01_FIFO_STATUS_RX_EMPTY);
    if (any) {
      cs_select();
      writeSPI1(nrf24l01_R_REGISTER | (nrf24l01_R_RX_PAYLOAD));
      data_in[0] = readSPI1(); // read received 6 byte payload
      data_in[1] = readSPI1(); 
      data_in[2] = readSPI1();
      data_in[3] = readSPI1();
      data_in[4] = readSPI1();
      data_in[5] = readSPI1();
      cs_deselect();
      // write RX_DR to clear receive flag
      nrf24_writeReg(nrf24l01_STATUS, nrf24l01_STATUS_RX_DR);
      return (uint32_t) 1;
    }
    else {
      return (uint32_t) 0;
    }
}
