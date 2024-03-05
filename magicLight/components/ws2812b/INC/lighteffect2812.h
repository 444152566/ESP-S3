#ifndef _LIGHTEFFECT2812_H_
#define _LIGHTEFFECT2812_H_

#include <stdio.h>


typedef enum{
    LIGHTEFFECT_IDLE = 0,
    LIGHTEFFECT_RAINBOW,
    LIGHTEFFECT_BREATH,
    LIGHTEFFECT_WATERFALL,
    LIGHTEFFECT_RAINBOW_BREATH,
    LIGHTEFFECT_RAINBOW_CYCLE,
    LIGHTEFFECT_USER_STATIC,
    LIGHTEFFECT_BULLET,
    LIGHTEFFECT_TEST,
}lightEffect_t;


void setLightEffect(lightEffect_t effect);
void lightEffectInit();

#endif // _LIGHTEFFECT2812_H_
