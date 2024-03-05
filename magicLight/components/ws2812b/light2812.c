// #include <stdio.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "light2812.h"

// #define PIN_NUM_MISO    25
#define PIN_NUM_MOSI    48
// #define PIN_NUM_CLK     19
#define PIN_NUM_CS      22

static spi_device_handle_t spi;
static const uint8_t code[2]={0xE0,0xF8};
static uint8_t lightBuff[LIGHT_NUM + 20][24];      // 颜色解码后的数据缓存

static int lightTrans(uint8_t* pColor, uint16_t len);

// 将颜色解码并传输
void lightSet(uint8_t pColor[][3], uint16_t lightLen)
{
    // printf("light set 1\n");
    uint16_t len = lightLen > LIGHT_NUM ? LIGHT_NUM:lightLen;     // LIGHT_NUM
    for (uint16_t i = 0; i < len; i++)
    {
        for (uint16_t j = 0; j < 8; j++)
        {
            lightBuff[i][j] = code[(pColor[i][0]>>(8-j)) & 0x01];     // 红色
            lightBuff[i][j+8] = code[(pColor[i][2]>>(8-j)) & 0x01];   // 蓝色
            lightBuff[i][j+16] = code[(pColor[i][1]>>(8-j)) & 0x01];  // 绿色
        }
    }
    lightTrans((uint8_t*)lightBuff, 24*len);
    // printf("light set 2\n");
}

// 将颜色解码并传输
void lightSetByList(List* colorList)
{
    if(colorList == NULL) return;
    ListNode* curColor = colorList->head;
    uint16_t len = colorList->size > LIGHT_NUM ? LIGHT_NUM:colorList->size;
    for (uint16_t i = 0; i < len; i++, curColor=curColor->next)
    {
        uint8_t* color = curColor->data.lightColorArry.color;
        for (uint16_t j = 0; j < 8; j++)
        {
            lightBuff[i][j] = code[(color[0]>>(8-j)) & 0x01];     // 红色
            lightBuff[i][j+8] = code[(color[2]>>(8-j)) & 0x01];   // 蓝色
            lightBuff[i][j+16] = code[(color[1]>>(8-j)) & 0x01];  // 绿色
        }
    }
    lightTrans((uint8_t*)lightBuff, 24*len);
}

int lightInit(void)
{
    esp_err_t ret;
    spi_bus_config_t buscfg={
        .miso_io_num = -1,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LIGHT_NUM * 3 * 8,
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=8*1000*1000,           //Clock out at 8 MHz
        .mode=0,                                //SPI mode 0
        .spics_io_num=-1,      // PIN_NUM_CS,               //CS pin
        .queue_size=7,                          //We want to be able to queue 7 transactions at a time
        // .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
    for (uint16_t i = 0; i < LIGHT_NUM; i++)
    {
        for (uint16_t j = 0; j < 24; j++)
        {
            lightBuff[i][j] = code[0];
        }
    }
	return 0;
}

static int lightTrans(uint8_t* pColor, uint16_t len)
{
    int ret = 0;
    static spi_transaction_t trans = {
        .user = (void*)0,
    };
    trans.length = len * 8;
    trans.tx_buffer = pColor;
    ret = spi_device_polling_transmit(spi, &trans);
    // assert(ret);
    
    // printf("ret:%d\n", ret);
    if(ret != 0) {
        printf("trans fail, ret:%d\n", ret);
    }

    return ret;
}
