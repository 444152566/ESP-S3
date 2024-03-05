#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "components_test.h"

void myTask(void *pvParam){   //用空指针就可以传递任何类型，传进来后再转化
	while(1){//任务函数里必须是死循环
		//实际任务
        components_test();      // 组件测试
		vTaskDelay(1000/portTICK_PERIOD_MS);//任务函数里必须有延时,也可用pdMS_TO_TICKS(1000)代替
	}
	vTaskDelete(NULL);//在本函数中删除，不需要传递句柄
}


void app_main(void)
{
    // uint32_t cnt = 100000;
    // while (1)
    // {
    //     components_test();
    //     cnt = 20000000;
    //     while(cnt--);
    // }
	//创建任务句柄
    TaskHandle_t myHandle = NULL;
    //         任务指针，        堆栈大小，   优先级
    xTaskCreate(myTask, "myTask", 10240, NULL, 1, &myHandle);    //传出任务句柄
    //                  任务标签       传入参数     任务句柄

    #if 0
    //通过任务句柄删除
    if(myHandle != NULL){
        vTaskDelete(myHandle);//在任务函数外删除任务
    }
    #endif
    
}
