#ifndef LAB_H
#define LAB_H

#define PROC_MAT_WIDTH 10
#define PROC_MAT_HEIGHT 10

#define MEMO_MAT_WIDTH 8
#define MEMO_MAT_HEIGHT 8

#define MEMO_FREE -1


/*
 * CALC ����
 * RDLK ����
 * WTLK д��
 * RDFL ���ļ�
 * WTFL д�ļ�
 * URLK ȥ����
 * UWLK ȥд��
 */

#define CALC 1
#define RDLK 2
#define WTLK 3
#define RDFL 4
#define WTFL 5
#define URLK 6
#define UWLK 7

typedef struct _code_t
{
    int op;
    int addition;
}code_t;

//proc
#define PROC_READY 1
#define PROC_WAITING 2
#define PROC_RUNNING 3
#define PROC_DEAD 4

//priority
#define PRI_HIGH 1
#define PRI_LOW 2
#define PRI_HIGH_TIME 5
#define PRI_LOW_TIME 2

#define RESOURCE_SIZE 65536

#define BUF_LEN 10
typedef struct _PCB{
    int pid;
    struct _PCB *next;
    int status;
    int priority;
    int mem_size;
    vector<int> memory;//δʹ��
    vector<int> resource;//δʹ��
    int pc;
    vector<code_t>code;
}PCB;
#define PCB_LIST_SIZE 1000

#define IS_PROC_READY(X) (PROC_READY==X?1:0)
#define IS_PROC_WAITING(X) (PROC_WAITING==X?1:0)
#define IS_PROC_RUNNING(X) (PROC_RUNNING==X?1:0)
#define IS_PROC_DEAD(X) (PROC_DEAD==X?1:0)
#define IS_PROC_INTR(X) (X<0?1:0)


//function
int proc_manage();
int res_manage();
int running();
int excution();
int init();


#endif // LAB_H
