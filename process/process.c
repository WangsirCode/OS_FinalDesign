#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/ipc.h>
#include <errno.h>
#include <stdlib.h>
#define SEM_KEY 1230
#define MAX_PROCESS_NUM 100
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};
/*信号量操作*/
static int sem_id = 0;
static int set_semvalue();
static void del_semvalue();
static int semaphore_p();
static int semaphore_v();

/*此例中用到的共享内存的数据结构*/
struct shared_data
{
    unsigned long result[MAX_PROCESS_NUM];
    unsigned long finishedProcess;     /*累加计数*/
};
int processNum;

typedef struct SumInfo
{
    unsigned long start;
    unsigned long end;
}SumInfo;

/*用于从文件中读取进程数和num的函数*/
static void getSumInfo(int *processNum,unsigned long *num);

/*用于将结果写进文件中*/
static void writeToFile(unsigned long result);

int main()
{
    /*共享内存相关变量*/
    void *shm = NULL;
    struct shared_data *shared;
    int shmid;
    
    
    unsigned long finalNum;
    int i = 0;
    unsigned long length = 0;
    SumInfo info;
    
    pid_t fpid;
    
    /*初始化共享内存*/
    shmid = shmget(IPC_PRIVATE,sizeof(struct shared_data),0666|IPC_CREAT);
    if(shmid == -1)
    {
        printf("shmget failed\n");
        
    }
    shm = shmat(shmid,NULL,0);
    if(shm == (void*)-1)
    {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    
    shared = (struct shared_data*)shm;
    shared->finishedProcess = 0;
    
    

    /*初始化信号量*/
    sem_id = semget((key_t)SEM_KEY, 1, 0666 | IPC_CREAT);
    if(!set_semvalue())
    {
        fprintf(stderr, "Failed to initialize semaphore\n");
        exit(EXIT_FAILURE);
    }
  
    /*从文件中读取进程数和累加数*/
    getSumInfo(&processNum, &finalNum);
    
    length = finalNum / processNum;
    info.start = 1;
    info.end = info.start + length;
    
    
    /*新建进程*/
    for(i= 0;i < (processNum - 1);i++)
    {
        fpid = fork();
        if(fpid == 0)
        {
            break;
        }
        info.start = info.end + 1;
        if (i == processNum - 2) {
            info.end = finalNum;
        }
        else
        {
           info.end += length;
        }

    }
    
    
    /*开始执行累加*/
    unsigned long index = info.start;
    unsigned long sumResult = 0;
    
    while(index <= info.end)
    {
        sumResult += index;
        index ++;
    }
    
    
    /*P操作*/
    if(!semaphore_p())
        exit(EXIT_FAILURE);
    shared -> result[shared->finishedProcess] = sumResult;
    shared -> finishedProcess += 1;

    
    /*判断所有进程是否完成累加*/
    if (shared->finishedProcess == processNum)
    {
        /*获取最终结果*/
        unsigned long finalResult = 0;
        for (index = 0; index < processNum; index ++)
        {
            finalResult += shared->result[index];
        }
        writeToFile(finalResult);
        printf("final reslt is %lu \n",finalResult);
        
        
        /*删除共享内存*/
        if(shmdt(shm) == -1)
        {
            fprintf(stderr, "shmdt failed\n");
            exit(EXIT_FAILURE);
        }

        if(shmctl(shmid, IPC_RMID, 0) == -1)
        {
            fprintf(stderr, "shmctl(IPC_RMID) failed\n");
            exit(EXIT_FAILURE);
        }
        
        del_semvalue();
        exit(EXIT_SUCCESS);
        
    }
    /*V操作*/
    if(!semaphore_v())
        exit(EXIT_FAILURE);
    
    
    return 0;
}

/*P操作*/
static int semaphore_p()
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;//P()
    sem_b.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sem_b, 1) == -1)
    {
        return 0;
    }
    return 1;
}

/*V操作*/
static int semaphore_v()
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;//V()
    sem_b.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sem_b, 1) == -1)
    {
        return 0;
    }
    return 1;
}

/*删除信号量*/
static void del_semvalue()
{
    union semun sem_union;  
    if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1)  
    {
        fprintf(stderr, "Failed to delete semaphore\n"); 
    }
}

/*初始化信号量*/
static int set_semvalue()  
{
    union semun sem_union;  
    sem_union.val = 1; 
    if(semctl(sem_id, 0, SETVAL, sem_union) == -1)  
        return 0;
    return 1;
}

/*用于从文件中读取进程数和num的函数*/
static void getSumInfo(int *processNum,unsigned long *num)
{
    FILE* file = fopen("input.txt", "r+");
    fscanf(file, "N=%d\nM=%lu",processNum,num);
    fclose(file);
}

/*用于将结果写进文件中*/
static void writeToFile(unsigned long result)
{
    FILE *file = fopen("output.txt","w");
    fprintf(file,"%lu",result);
    fclose(file);
}

