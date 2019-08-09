#include "factory.h"

int main(int argc, char *argv[])
{
    ARGS_CHECK(argc, 2);

    //读取文件 路径../conf/server.conf
    char ip[16] = {0};
    char port[10] = {0};
    int threadNum;
    int capacity;
    confInit(argv[1], ip, port, &threadNum, &capacity);
    printf("ip=%s, port=%s, threadNum=%d, capacity=%d\n", ip, port, threadNum, capacity);

    Factory_t threadMainData;
    factoryInit(&threadMainData, threadNum, capacity); 
    factoryStart(&threadMainData);

    int socketFd, newFd;
    tcpInit(&socketFd, ip, port);
    pQue_t pq = &threadMainData.que;
    pNode_t pNew;
    while(1)
    {
        newFd = accept(socketFd, NULL, NULL);
        pNew = (pNode_t)calloc(1, sizeof(Node_t)); 
        pNew->newFd = newFd;
        //放入队列
        pthread_mutex_lock(&pq->mutex);
        queInsert(pq, pNew);
        pthread_mutex_unlock(&pq->mutex);
        //通知子线程有任务
        pthread_cond_signal(&threadMainData.cond);
    }
    return 0;
}

