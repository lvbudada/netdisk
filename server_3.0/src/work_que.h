#ifndef __WORK_QUE__
#define __WORK_QUE__
#include "head.h"

typedef struct tag_node{
    int newFd;
    struct tag_node *pNext;
}Node_t, *pNode_t;  //队列结点的结构体，存储实际的client fd

typedef struct{
    pNode_t queHead, queTail;
    int capacity;
    int size;
    pthread_mutex_t mutex;  //线程锁
}Que_t, *pQue_t;   //描述队列的结构体

void queInit(pQue_t pq, int capacity);
void queInsert(pQue_t pq, pNode_t pnew);
int queGet(pQue_t pq, pNode_t *p);
#endif
