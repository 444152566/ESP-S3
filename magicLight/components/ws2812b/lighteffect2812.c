#include "light2812.h"
#include "list.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lighteffect2812.h"




/* 呼吸灯曲线表 */
const uint16_t waveIndex[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 
                               3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 
                               12, 12, 13, 13, 14, 14, 15, 15, 16, 17, 17, 18, 19, 19, 20, 21, 22, 23, 24, 24, 25, 26, 27, 29, 30, 31, 32, 
                               33, 35, 36, 37, 39, 40, 42, 44, 45, 47, 49, 51, 53, 55, 57, 59, 61, 64, 66, 69, 71, 74, 77, 80, 83, 86, 89, 
                               93, 96, 100, 104, 108, 112, 116, 121, 125, 130, 135, 140, 145, 151, 157, 163, 169, 175, 182, 189, 196, 204, 
                               212, 220, 228, 237, 246, 246};

// 常用的颜色， 
const uint8_t colorBlack[3]    =	{0,   0,   0  };
const uint8_t colorWhite[3]    =	{255, 255, 255};

const uint8_t colorRed[3]      =	{255, 0,   0  };
const uint8_t colorOrange[3]   =	{255, 40, 0  };
const uint8_t colorYellow[3]   =	{255, 80, 0  };
const uint8_t colorGreen[3]    =	{0  , 255, 0  };
const uint8_t colorBlue[3]     =	{0  ,  0,  255};
const uint8_t colorIndigo[3]   =	{12,   164, 255};
const uint8_t colorPurple[3]   =	{178, 0,   255};

const uint8_t colorRoseRed[3]  =	{255,   10,   40};
const uint8_t colorGrassGreen[3]  =	{10, 250, 0};
const uint8_t colorSkyBlue[3]  =	{108, 164, 188};
const uint8_t colorPurpleRed[3]   =	{255, 0,   200};

const uint8_t colorTest[3] 	  =	{0  ,  0,  249};
const uint8_t colorTest2[3]    =	{249, 0, 178};

static uint8_t color[400][3];         // 灯珠颜色缓存
static List* colorList = NULL;        // 灯珠颜色缓存链表
static lightEffect_t curEffect = LIGHTEFFECT_IDLE;
static bool initFlag = true;

// 灯效运行任务
static TaskHandle_t lightEffTaskHandle = NULL;
static void lightEffRunTask(void *pvParam);

// 灯效
static void lightEffectCloseAll(void);
static void lightEffectRainbowBreath(void);
static void lightEffectRainbowCycle(void);
static void lightEffectBulletFall(void);

static void lightShift(int16_t shiftLen);
static void colorBrigthControl(uint8_t* color, uint8_t brigth);
static void lightEffectRefresh(void);



// 切换灯效
void setLightEffect(lightEffect_t effect)
{
    curEffect = effect;
    initFlag = true;
}

void lightEffectInit()
{
    curEffect = LIGHTEFFECT_BULLET;

    if(!colorList) {
        colorList = (List*)pvPortMalloc(sizeof(List));
        ListInit(colorList);
    }
    ListDataType colorTmp = {
        .lightColorArry = {
            .color = {[INDEX_R] = 0, [INDEX_G] = 0, [INDEX_B] = 0},
        },
    };
    for (int i = colorList->size; i < LIGHT_NUM; i++)        // LIGHT_NUM
    {
        ListInsertInTail(colorList, colorTmp);
    }
    xTaskCreate(lightEffRunTask, "lightEffRunTask", 1024 * 5, NULL, 1, &lightEffTaskHandle);    //传出任务句柄
}

// 灯效运行任务
static void lightEffRunTask(void *pvParam)  //用空指针就可以传递任何类型，传进来后再转化
{
	while(1){
        switch (curEffect)
        {
        case LIGHTEFFECT_IDLE:
            lightEffectCloseAll();
            vTaskDelay(500/portTICK_PERIOD_MS);
            break;
        
        case LIGHTEFFECT_RAINBOW_CYCLE:
            lightEffectRainbowCycle();
            vTaskDelay(10/portTICK_PERIOD_MS);
            break;
        
        case LIGHTEFFECT_RAINBOW_BREATH:
            lightEffectRainbowBreath();
            vTaskDelay(10/portTICK_PERIOD_MS);
            break;
        
        case LIGHTEFFECT_BULLET:
            lightEffectBulletFall();
            vTaskDelay(40/portTICK_PERIOD_MS);
            break;

        default:
            vTaskDelay(500/portTICK_PERIOD_MS);
            break;
        }

		// vTaskDelay(1000/portTICK_PERIOD_MS);//任务函数里必须有延时,也可用pdMS_TO_TICKS(1000)代替
	}
}

// 关灯
static void lightEffectCloseAll()
{
    ListNode* pLightNode = colorList->head;
    while (pLightNode)
    {
        pLightNode->data.lightColorArry.color[INDEX_R] = 0;
        pLightNode->data.lightColorArry.color[INDEX_G] = 0;
        pLightNode->data.lightColorArry.color[INDEX_B] = 0;
        pLightNode = pLightNode->next;
    }
    lightEffectRefresh();
}

// 彩虹呼吸
static void lightEffectRainbowBreath()
{
    static uint16_t lightSize = LIGHT_NUM/7;
    static uint8_t brigth;
    static bool flag;
    if(initFlag) {
        printf("RainBow Breaht init\n");
        initFlag = false;
        brigth = 0;
        flag = true;
    }

    ListNode* pLightNode = colorList->head;
    for (int i = 0; i < lightSize; i++)
    {
        pLightNode->data.lightColorArry.color[INDEX_R] = colorRed[INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = colorRed[INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = colorRed[INDEX_B];
        colorBrigthControl(pLightNode->data.lightColorArry.color, brigth);
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = colorOrange[INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = colorOrange[INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = colorOrange[INDEX_B];
        colorBrigthControl(pLightNode->data.lightColorArry.color, brigth);
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = colorYellow[INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = colorYellow[INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = colorYellow[INDEX_B];
        colorBrigthControl(pLightNode->data.lightColorArry.color, brigth);
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = colorGreen[INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = colorGreen[INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = colorGreen[INDEX_B];
        colorBrigthControl(pLightNode->data.lightColorArry.color, brigth);
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = colorBlue[INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = colorBlue[INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = colorBlue[INDEX_B];
        colorBrigthControl(pLightNode->data.lightColorArry.color, brigth);
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = colorIndigo[INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = colorIndigo[INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = colorIndigo[INDEX_B];
        colorBrigthControl(pLightNode->data.lightColorArry.color, brigth);
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = colorPurple[INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = colorPurple[INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = colorPurple[INDEX_B];
        colorBrigthControl(pLightNode->data.lightColorArry.color, brigth);
        pLightNode = pLightNode->next;
    }
    if(flag) {
        brigth++;
        if(brigth >= 149) flag = false;
    } else {
        brigth--;
        if(brigth < 1) flag = true;
    }
    lightEffectRefresh();
}

// 光谱循环
static void lightEffectRainbowCycle(void)
{
    static uint8_t cnt = 0;
    static int16_t shfitSize = 0;
    if(initFlag) {
        printf("RainBow Cycle init\n");
        uint16_t index = 0;
        for (uint8_t i = 0; i < 51; i++, index++)
        {
            color[index][INDEX_R] = (uint8_t)((int16_t)colorRed[INDEX_R] - ((int16_t)colorRed[INDEX_R] - (int16_t)colorOrange[INDEX_R]) * i / 50);
            color[index][INDEX_G] = (uint8_t)((int16_t)colorRed[INDEX_G] - ((int16_t)colorRed[INDEX_G] - (int16_t)colorOrange[INDEX_G]) * i / 50);
            color[index][INDEX_B] = (uint8_t)((int16_t)colorRed[INDEX_B] - ((int16_t)colorRed[INDEX_B] - (int16_t)colorOrange[INDEX_B]) * i / 50);
        }
        
        for (uint8_t i = 0; i < 51; i++, index++)
        {
            color[index][INDEX_R] = (uint8_t)((int16_t)colorOrange[INDEX_R] - ((int16_t)colorOrange[INDEX_R] - (int16_t)colorYellow[INDEX_R]) * i / 50);
            color[index][INDEX_G] = (uint8_t)((int16_t)colorOrange[INDEX_G] - ((int16_t)colorOrange[INDEX_G] - (int16_t)colorYellow[INDEX_G]) * i / 50);
            color[index][INDEX_B] = (uint8_t)((int16_t)colorOrange[INDEX_B] - ((int16_t)colorOrange[INDEX_B] - (int16_t)colorYellow[INDEX_B]) * i / 50);
        }
        
        for (uint8_t i = 0; i < 51; i++, index++)
        {
            color[index][INDEX_R] = (uint8_t)((int16_t)colorYellow[INDEX_R] - ((int16_t)colorYellow[INDEX_R] - (int16_t)colorGreen[INDEX_R]) * i / 50);
            color[index][INDEX_G] = (uint8_t)((int16_t)colorYellow[INDEX_G] - ((int16_t)colorYellow[INDEX_G] - (int16_t)colorGreen[INDEX_G]) * i / 50);
            color[index][INDEX_B] = (uint8_t)((int16_t)colorYellow[INDEX_B] - ((int16_t)colorYellow[INDEX_B] - (int16_t)colorGreen[INDEX_B]) * i / 50);
        }
        
        for (uint8_t i = 0; i < 51; i++, index++)
        {
            color[index][INDEX_R] = (uint8_t)((int16_t)colorGreen[INDEX_R] - ((int16_t)colorGreen[INDEX_R] - (int16_t)colorBlue[INDEX_R]) * i / 50);
            color[index][INDEX_G] = (uint8_t)((int16_t)colorGreen[INDEX_G] - ((int16_t)colorGreen[INDEX_G] - (int16_t)colorBlue[INDEX_G]) * i / 50);
            color[index][INDEX_B] = (uint8_t)((int16_t)colorGreen[INDEX_B] - ((int16_t)colorGreen[INDEX_B] - (int16_t)colorBlue[INDEX_B]) * i / 50);
        }
        
        for (uint8_t i = 0; i < 51; i++, index++)
        {
            color[index][INDEX_R] = (uint8_t)((int16_t)colorBlue[INDEX_R] - ((int16_t)colorBlue[INDEX_R] - (int16_t)colorIndigo[INDEX_R]) * i / 50);
            color[index][INDEX_G] = (uint8_t)((int16_t)colorBlue[INDEX_G] - ((int16_t)colorBlue[INDEX_G] - (int16_t)colorIndigo[INDEX_G]) * i / 50);
            color[index][INDEX_B] = (uint8_t)((int16_t)colorBlue[INDEX_B] - ((int16_t)colorBlue[INDEX_B] - (int16_t)colorIndigo[INDEX_B]) * i / 50);
        }
        
        for (uint8_t i = 0; i < 51; i++, index++)
        {
            color[index][INDEX_R] = (uint8_t)((int16_t)colorIndigo[INDEX_R] - ((int16_t)colorIndigo[INDEX_R] - (int16_t)colorPurple[INDEX_R]) * i / 50);
            color[index][INDEX_G] = (uint8_t)((int16_t)colorIndigo[INDEX_G] - ((int16_t)colorIndigo[INDEX_G] - (int16_t)colorPurple[INDEX_G]) * i / 50);
            color[index][INDEX_B] = (uint8_t)((int16_t)colorIndigo[INDEX_B] - ((int16_t)colorIndigo[INDEX_B] - (int16_t)colorPurple[INDEX_B]) * i / 50);
        }
        
        for (uint8_t i = 0; i < 51; i++, index++)
        {
            color[index][INDEX_R] = (uint8_t)((int16_t)colorPurple[INDEX_R] - ((int16_t)colorPurple[INDEX_R] - (int16_t)colorRed[INDEX_R]) * i / 50);
            color[index][INDEX_G] = (uint8_t)((int16_t)colorPurple[INDEX_G] - ((int16_t)colorPurple[INDEX_G] - (int16_t)colorRed[INDEX_G]) * i / 50);
            color[index][INDEX_B] = (uint8_t)((int16_t)colorPurple[INDEX_B] - ((int16_t)colorPurple[INDEX_B] - (int16_t)colorRed[INDEX_B]) * i / 50);
        }
        initFlag = false;
        cnt = 0;
        shfitSize = 0;
    }

    ListNode* pLightNode = colorList->head;
    uint8_t size = LIGHT_NUM/7;
    if(cnt >= 51) {
        cnt = 0;
        shfitSize--;
        if(shfitSize <= -7) shfitSize = 0;
    }
    for (uint8_t i = 0; i < size; i++)
    {
        pLightNode->data.lightColorArry.color[INDEX_R] = color[cnt + 51*0][INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = color[cnt + 51*0][INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = color[cnt + 51*0][INDEX_B];
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = color[cnt + 51*1][INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = color[cnt + 51*1][INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = color[cnt + 51*1][INDEX_B];
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = color[cnt + 51*2][INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = color[cnt + 51*2][INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = color[cnt + 51*2][INDEX_B];
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = color[cnt + 51*3][INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = color[cnt + 51*3][INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = color[cnt + 51*3][INDEX_B];
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = color[cnt + 51*4][INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = color[cnt + 51*4][INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = color[cnt + 51*4][INDEX_B];
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = color[cnt + 51*5][INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = color[cnt + 51*5][INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = color[cnt + 51*5][INDEX_B];
        pLightNode = pLightNode->next;
        pLightNode->data.lightColorArry.color[INDEX_R] = color[cnt + 51*6][INDEX_R];
        pLightNode->data.lightColorArry.color[INDEX_G] = color[cnt + 51*6][INDEX_G];
        pLightNode->data.lightColorArry.color[INDEX_B] = color[cnt + 51*6][INDEX_B];
        pLightNode = pLightNode->next;
    }
    cnt++;
    lightShift(shfitSize);
    lightEffectRefresh();
}

// 子弹流动
static void lightEffectBulletFall()
{
    if(initFlag) {
        ListNode* pLightNode = colorList->head;
        while(pLightNode) {
            pLightNode->data.lightColorArry.color[INDEX_R] = 0;
            pLightNode->data.lightColorArry.color[INDEX_G] = 0;
            pLightNode->data.lightColorArry.color[INDEX_B] = 0;
            pLightNode = pLightNode->next;
        }
        pLightNode = colorList->head;
        for (int i = 0; i < 2; i++) {
            if(pLightNode == NULL) break;
            pLightNode->data.lightColorArry.color[INDEX_R] = colorRoseRed[INDEX_R];
            pLightNode->data.lightColorArry.color[INDEX_G] = colorRoseRed[INDEX_G];
            pLightNode->data.lightColorArry.color[INDEX_B] = colorRoseRed[INDEX_B];
            pLightNode = pLightNode->next;
            if(pLightNode == NULL) break;
            pLightNode->data.lightColorArry.color[INDEX_R] = colorRoseRed[INDEX_R]/5;
            pLightNode->data.lightColorArry.color[INDEX_G] = colorRoseRed[INDEX_G]/5;
            pLightNode->data.lightColorArry.color[INDEX_B] = colorRoseRed[INDEX_B]/5;
            pLightNode = pLightNode->next;
            if(pLightNode == NULL) break;
            pLightNode->data.lightColorArry.color[INDEX_R] = colorRoseRed[INDEX_R]/15;
            pLightNode->data.lightColorArry.color[INDEX_G] = colorRoseRed[INDEX_G]/15;
            pLightNode->data.lightColorArry.color[INDEX_B] = colorRoseRed[INDEX_B]/15;
            pLightNode = pLightNode->next;
            if(pLightNode == NULL) break;
            pLightNode = pLightNode->next;
            if(pLightNode == NULL) break;
            pLightNode = pLightNode->next;
            if(pLightNode == NULL) break;
            pLightNode->data.lightColorArry.color[INDEX_R] = colorBlue[INDEX_R];
            pLightNode->data.lightColorArry.color[INDEX_G] = colorBlue[INDEX_G];
            pLightNode->data.lightColorArry.color[INDEX_B] = colorBlue[INDEX_B];
            pLightNode = pLightNode->next;
            if(pLightNode == NULL) break;
            pLightNode->data.lightColorArry.color[INDEX_R] = colorBlue[INDEX_R]/5;
            pLightNode->data.lightColorArry.color[INDEX_G] = colorBlue[INDEX_G]/5;
            pLightNode->data.lightColorArry.color[INDEX_B] = colorBlue[INDEX_B]/5;
            pLightNode = pLightNode->next;
            if(pLightNode == NULL) break;
            pLightNode->data.lightColorArry.color[INDEX_R] = colorBlue[INDEX_R]/8;
            pLightNode->data.lightColorArry.color[INDEX_G] = colorBlue[INDEX_G]/8;
            pLightNode->data.lightColorArry.color[INDEX_B] = colorBlue[INDEX_B]/8;
            pLightNode = pLightNode->next;
            if(pLightNode == NULL) break;
            pLightNode = pLightNode->next;
            if(pLightNode == NULL) break;
            pLightNode = pLightNode->next;
        }
        initFlag = false;
    }
    lightShift(-1);
    lightEffectRefresh();
}

// 灯珠整体移动n个单位
static void lightShift(int16_t shiftLen)
{
    if(shiftLen == 0) return;
    
    colorList->tail->next = colorList->head;
    colorList->head->before = colorList->tail;

    int16_t index = shiftLen > 0? colorList->size - shiftLen : -shiftLen;
    ListNode* pNode = colorList->head;
    for (int16_t i = 0; i < index; i++)
    {
        pNode = pNode->next;
    }

    colorList->head = pNode;
    colorList->tail = pNode->before;
    colorList->bTail = colorList->tail->before;

    colorList->tail->next = NULL;
    colorList->head->before = NULL;
}

//亮度调节	范围：0-15(0 -- 255)
static void colorBrigthControl(uint8_t* color, uint8_t brigth)
{
	uint8_t colorTemp[3];
    uint8_t mColor = (uint8_t)(brigth*brigth/100);
    // uint8_t mColor = waveIndex[brigth];
	colorTemp[INDEX_R] = color[INDEX_R];
	colorTemp[INDEX_G] = color[INDEX_G]; 
	colorTemp[INDEX_B] = color[INDEX_B];
    if(colorTemp[INDEX_R] >= colorTemp[INDEX_G] && colorTemp[INDEX_R] >= colorTemp[INDEX_B]) {
		color[INDEX_R] = mColor;
		color[INDEX_G] = (uint8_t)((uint16_t)color[INDEX_R] * colorTemp[INDEX_G] / colorTemp[INDEX_R]);
		color[INDEX_B] = (uint8_t)((uint16_t)color[INDEX_R] * colorTemp[INDEX_B] / colorTemp[INDEX_R]);
    } else if(colorTemp[INDEX_G] >= colorTemp[INDEX_R] && colorTemp[INDEX_G] >= colorTemp[INDEX_B]) {
		color[INDEX_G] = mColor;
		color[INDEX_R] = (uint8_t)((uint16_t)color[INDEX_G] * colorTemp[INDEX_R] / colorTemp[INDEX_G]);
		color[INDEX_B] = (uint8_t)((uint16_t)color[INDEX_G] * colorTemp[INDEX_B] / colorTemp[INDEX_G]);
    } else {
		color[INDEX_B] = mColor;
		color[INDEX_R] = (uint8_t)((uint16_t)color[INDEX_B] * colorTemp[INDEX_R] / colorTemp[INDEX_B]);
		color[INDEX_G] = (uint8_t)((uint16_t)color[INDEX_B] * colorTemp[INDEX_G] / colorTemp[INDEX_B]);
    }
}

// 刷新灯珠颜色
static void lightEffectRefresh()
{
    lightSetByList(colorList);
}
