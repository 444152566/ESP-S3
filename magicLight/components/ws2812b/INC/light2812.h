#ifndef _LIGHT2812_H_
#define _LIGHT2812_H_

#include <stdio.h>
#include "list.h"

#define SPI_SCL_PIN             GPIO_P02
#define SPI_SDA_PIN             GPIO_P03
#define SPI_MISO_PIN            GPIO_DUMMY
#define SPI_MOSI_PIN            SPI_SDA_PIN

#define SPI_CS_PIN              GPIO_P00    //  GPIO_DUMMY

#define LIGHT_NUM      CONFIG_LIGHT_NUM

#define INDEX_R     2
#define INDEX_G     0
#define INDEX_B     1

void lightSet(uint8_t pColor[][3], uint16_t lightLen);
void lightSetByList(List* colorList);
int lightInit(void);

#endif // _LIGHT2812_H_
