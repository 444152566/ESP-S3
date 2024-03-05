#ifndef _LIST_H_
#define _LIST_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdbool.h>

typedef struct
{
    // uint16_t id;
	uint8_t color[3];
}lightColor_t;

// typedef lightColor_t ListDataType;
typedef union
{
    lightColor_t lightColorArry;
}ListDataType;

typedef struct ListNode
{
	struct ListNode* next;
	struct ListNode* before;
	ListDataType data;
}ListNode;

typedef struct
{
	ListNode* head;
	ListNode* tail;
	ListNode* bTail;
	uint16_t size;
}List;


// 初始化链表
void ListInit(List* list);
// 插入元素(尾插)
void ListInsertInTail(List* list, ListDataType data);
// 移除指定序号元素
void ListRemoveAt(List* list, uint16_t index);
// 获取指定序号元素
ListDataType ListAt(List* list, uint16_t index);
// 获取链表队头元素
ListDataType ListHead(List* list);
// 获取链表队尾元素
ListDataType ListBack(List* list);
// 获取链表中有效元素个数
uint16_t ListSize(List* list);
// 检测链表是否为空，如果为空返回非零结果，如果非空返回0
bool ListEmpty(List* list);
// 销毁链表
void ListDestroy(List* list);







#ifdef __cplusplus
}
#endif

#endif // _LIST_H_
