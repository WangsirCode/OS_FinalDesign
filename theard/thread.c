#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_THREAD_NUM 100

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned long result[MAX_THREAD_NUM];
int finishedThreadNum = 0;
int threadNum;

typedef struct SumInfo
{
    unsigned long start;
    unsigned long end;
}SumInfo;

/*用于从文件中读取线程数和num的函数*/
static void getSumInfo(int *threadNum,unsigned long *num);

/*用于将结果写进文件中*/
static void writeToFile(unsigned long result);

void increse_num(SumInfo* info);

int main(int argc,char *argv[])
{
    unsigned long finalNum;
    int i;
    SumInfo info[MAX_THREAD_NUM];
    unsigned long length;
    unsigned long start = 1;
    unsigned long end;
    pthread_t thread[MAX_THREAD_NUM];
    /*获取线程数和累加终值*/
    getSumInfo(&threadNum, &finalNum);
    
   /* 自动测试时使用下面方式获取参数*/
    /*sscanf(argv[1],"%d",&threadNum);*/
    /*sscanf(argv[2],"%lu",&finalNum);    */
    length = finalNum / threadNum;
    end = start + length;
    
    
    for(i= 0;i < threadNum;i++)
    {
        int ret;
        info[i].start = start;
        info[i].end = end;
        ret = pthread_create(&thread[i], NULL, (void *)increse_num, &info[i]);
        start = end + 1;
        
        if (i == (threadNum - 2))
        {
            end = finalNum;
        }
        else
        {
            end += length;
        }

    }
    
    for(i = 0; i < threadNum; i++)
    {
        pthread_join(thread[i], NULL);
    }
    return 1;
}
void increse_num(SumInfo* info)
{
    unsigned long start = info->start;
    unsigned long end = info->end;
    unsigned long sumResult = 0;
    unsigned long index = start;
    
    while(index <= end)
    {
        sumResult += index;
        index ++;

    }
    if (pthread_mutex_lock(&mutex) != 0)
    {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }
    result[finishedThreadNum] = sumResult;
    finishedThreadNum += 1;
    if (finishedThreadNum == threadNum)
    {
        unsigned long finalResult = 0;
        for (index = 0; index < threadNum; index ++)
        {
            finalResult += result[index];
        }
        writeToFile(finalResult);
        printf("final reslt is %lu \n",finalResult);
    }
    
    if (pthread_mutex_unlock(&mutex) != 0)
    {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }
}

/*用于从文件中读取进程数和num的函数*/
static void getSumInfo(int *threadNum,unsigned long *num)
{
    FILE* file = fopen("input.txt", "r+");
    fscanf(file, "N=%d\nM=%lu",threadNum,num);
    fclose(file);
}

/*用于将结果写进文件中*/
static void writeToFile(unsigned long result)
{
    FILE *file = fopen("output.txt","w");
    fprintf(file,"%lu",result);
    fclose(file);
}
