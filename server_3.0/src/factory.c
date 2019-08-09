#include "factory.h"

int factoryInit(pFactory_t pf, int threadNum, int capacity)
{
    queInit(&pf->que, capacity);  //初始化队列
    pthread_cond_init(&pf->cond, NULL);
    pf->pthid = (pthread_t*)calloc(threadNum, sizeof(pthread_t));
    pf->threadNum = threadNum;
    pf->startFlag = 0;  //0代表未启动， 1代表启动
    return 0;
}

//子线程函数
void* threadFunc(void *p)
{
    pFactory_t pf = (pFactory_t)p;
    pQue_t pq = &pf->que;
    pNode_t pCur;
    int getSuccess;
    while(1)
    {
        pthread_mutex_lock(&pq->mutex);
        if(!pq->size)
        {
            pthread_cond_wait(&pf->cond, &pq->mutex);  //队列为空，就睡觉
        }
        getSuccess = queGet(pq, &pCur);  //从队列中拿任务 
        pthread_mutex_unlock(&pq->mutex);
        if(!getSuccess)
        {
            //tranFile(pCur->newFd);  //发文件 
            //close(pCur->newFd);
            //相应客户端的各种命令
            cmdHandle(pCur->newFd);
            free(pCur);
        }
        pCur = NULL;
    }
}

//启动工场
int factoryStart(pFactory_t pf)
{
    int i;
    if(!pf->startFlag)
    {
        for(i=0; i<pf->threadNum; i++)
        {
            pthread_create(pf->pthid+i, NULL, threadFunc, pf);
        }
        pf->startFlag = 1;
    }
    return 0;
}

//读取配置文件
int confInit(char *path, char *ip, char *port, int *threadNum, int *capacity)
{
    FILE *fp = fopen(path, "rb");
    ERROR_CHECK(fp, NULL, "fopen");

    char line[128] = {0};
    int lineLen;
    int i;
    while(fgets(line, sizeof(line), fp)!=NULL)
    {
        lineLen = strlen(line);
        line[lineLen-1] = '\0';
        for(i=0; i<lineLen; i++)
        {
            if('=' == line[i])
            {
                line[i] = '\0';
                break;
            }
        }
        if(!strcmp("ip", line))
        {
            strcpy(ip, line + i + 1);
        }
        if(!strcmp("port", line))
        {
            strcpy(port, line + i + 1);
        }
        if(!strcmp("threadNum", line))
        {
            *threadNum = atoi(line + i + 1);
        }
        if(!strcmp("queCapacity", line))
        {
            *capacity = atoi(line + i + 1);
        }
    }

    fclose(fp);
    return 0;
}
