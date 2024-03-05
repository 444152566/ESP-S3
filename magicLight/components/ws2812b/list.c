// #include <stdio.h>
// #include <stdbool.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"

#include "list.h"

// 初始化链表
void ListInit(List* list)
{
	//断言判断指针有效性
	assert(list);

	//初始化
	list->head = list->tail = list->bTail = NULL;
	list->size = 0;
}
// 插入元素
void ListInsertInTail(List* list, ListDataType data)
{
	//断言判断指针有效性
	assert(list);

	//开辟结点
	// ListNode* newnode = (ListNode*)osal_mem_alloc(sizeof(ListNode));
	ListNode* newnode = pvPortMalloc(sizeof(ListNode));
	if (NULL == newnode)
	{
		printf("malloc fail\n");
		return;
	}
	newnode->next = NULL;
	newnode->before = NULL;
	newnode->data = data;

	//插入数据
	//空链表插入数据
	if (list->head == NULL)
	{
		list->head = list->tail = newnode;
	}
	//非空队列插入数据
	else
	{
        list->bTail = list->tail;
        list->tail = newnode;
		list->bTail->next = list->tail;
        list->tail->before = list->bTail;
	}

	list->size++;
}
// 移除指定序号元素
void ListRemoveAt(List* list, uint16_t index)
{

}
// 获取指定序号元素
ListDataType ListAt(List* list, uint16_t index)
{
	assert(list);
    ListNode* curdata = list->head;
    for (uint16_t i = 0; i < index; i++)
    {
	    assert(curdata);
        curdata = curdata->next;
    }
	assert(curdata);
    return curdata->data;
}
// 获取链表头元素
ListDataType ListHead(List* list)
{
	assert(list);
    return list->head->data;
}
// 获取链表尾元素
ListDataType ListBack(List* list)
{
	assert(list);
    return list->tail->data;
}
// 获取链表中有效元素个数
uint16_t ListSize(List* list)
{
	assert(list);
    return list->size;
}
// 检测链表是否为空，如果为空返回非零结果，如果非空返回0
bool ListEmpty(List* list)
{
	return list->size == 0;
}
// 销毁链表
void ListDestroy(List* list)
{
	assert(list);

	//释放结点
	while (list->head)
	{
		ListNode* next = list->head->next;
		if (list->tail != list->head)
		{
			vPortFree(list->head);
			list->head = next;
		}
		else
		{
			//避免野指针
			vPortFree(list->head);
			list->head=list->tail = NULL;
		}
	}
	//手动置零
	list->size = 0;
}
