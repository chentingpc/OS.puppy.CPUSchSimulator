#include <iostream>
#include <string>
#include <vector>
#include <cassert>
using namespace std;
#include "lab.h"

#include <QMessageBox>

extern vector<vector<int> > proc_state_table;
extern int cur_proc_table_idx;
extern int memo_state_table[MEMO_MAT_WIDTH][MEMO_MAT_HEIGHT];
extern vector<code_t> instr_vec;

int continuation;
//vector <PCB> pcb_list;
PCB pcb_list[PCB_LIST_SIZE];
int pcb_pos;
PCB *waiting_list,*running_list,*ready_list;
vector <int> interuption;
int resource[RESOURCE_SIZE][2];//read lock [0] and wirte lock [1]
vector <int> exu_list;

int proc_manage(){

    PCB *TT1=running_list;
    if (running_list)
    {
        if (running_list->pc == running_list->code.size()){
            int pid=running_list->pid;
            running_list->status=PROC_DEAD;
            running_list=NULL;

            //retract memory resourece
            for (int i=0;i<MEMO_MAT_WIDTH;i++){
                for (int j=0;j<MEMO_MAT_HEIGHT;j++){
                    if (memo_state_table[i][j] == pid){
                        memo_state_table[i][j]=MEMO_FREE;
                    }
                }
            }
        }
    }

    PCB *temp,*temp_pre,*temp2;

    //check interuption, dispatch when no interuption
    if (interuption.size()==0){

        //check running list
        if (running_list != NULL ){
            if ( ( running_list->priority == PRI_HIGH && continuation == PRI_HIGH_TIME )
                 || ( running_list->priority == PRI_LOW && continuation == PRI_LOW_TIME ) ){
                continuation=0;
                if (ready_list){
                    temp=ready_list;
                    while (temp->next)
                        temp=temp->next;
                    temp->next=running_list;
                    temp->next->status=PROC_READY;
                    temp->next->next=NULL;
                } else{
                    ready_list=running_list;
                    ready_list->status=PROC_READY;
                    ready_list->next=NULL;
                }
                running_list=NULL;
            } else {
                continuation++;
            }
        }

        //check waiting list
        temp_pre=NULL;
        temp=waiting_list;
        code_t op_temp;
        while (temp){
            //check the resource, if free, put the proc into the end of ready list
            bool temp_pass=false;
            assert(temp->pc>0);
            op_temp=temp->code[temp->pc-1];
            if (op_temp.op == RDLK && resource[op_temp.addition][1] == 0){
                resource[op_temp.addition][0]++;
                temp_pass=true;
            } else if (op_temp.op == WTLK && resource[op_temp.addition][0] == 0 && resource[op_temp.addition][1] == 0){
                resource[op_temp.addition][1]++;
                temp_pass=true;
            }

            if (temp_pass == true){
                //move waiting list ele to ready list
                if (ready_list){
                    temp2=ready_list;
                    while (temp2->next)
                        temp2=temp2->next;
                    temp2->next=temp;
                } else
                    ready_list=temp;
                if (temp_pre==NULL)
                    waiting_list=temp->next;
                else
                    temp_pre->next=temp->next;
                temp->status=PROC_READY;
                temp->next=NULL;
            }

            temp_pre=temp;
            temp=temp->next;
        }

        //select proc
        if ( running_list == NULL && ready_list != NULL){
            //get the first of ready list to running list
            running_list=ready_list;
            ready_list=ready_list->next;
            running_list->next=NULL;
            running_list->status = PROC_RUNNING;
        }
    }
    return 0;
}
int res_manage(){
    assert(running_list != NULL && running_list->mem_size < MEMO_MAT_WIDTH*MEMO_MAT_HEIGHT);
    int size=running_list->mem_size;
    int allocated=0;
    int pid=running_list->pid;
    int free=MEMO_FREE;

    //search allocated
    for (int i=0;i<MEMO_MAT_WIDTH;i++){
        for (int j=0;j<MEMO_MAT_HEIGHT;j++){
            if (memo_state_table[i][j] == pid){
                allocated++;
            }
        }
    }

    //allocate free ones
    if (allocated < size)
        for (int i=0; i<MEMO_MAT_WIDTH;i++){
            for (int j=0; j<MEMO_MAT_HEIGHT;j++){
                if (memo_state_table[i][j] == free){
                    memo_state_table[i][j]=pid;
                    allocated++;
                    if (allocated == size){//allocation complete
                        exu_list.push_back(pid);
                        return 0;
                    }
                }
            }
        }
    else{
        exu_list.push_back(pid);
        return 0;
    }

    //make the sorted sequence of the proc excution
    vector<int> sorted_seq;
    bool exist;
    for (int i=exu_list.size()-1;i>=0;i--){
        exist=false;
        for (int j=0;j<sorted_seq.size();j++){
            if (exu_list[i] == sorted_seq[j]){
                exist=true;
                break;
            }
        }
        if (!exist){
            sorted_seq.push_back(exu_list[i]);
        }
    }

    while (allocated != size){
        //get the longest not used proc
        if (sorted_seq.size() > 0)
        {
            free=sorted_seq[sorted_seq.size()-1];
            sorted_seq.pop_back();
        }

        //allocate the longest not used ones
        for (int i=0; i<MEMO_MAT_WIDTH;i++){
            for (int j=0; j<MEMO_MAT_HEIGHT;j++){
                if (memo_state_table[i][j] == free){
                    if (allocated != size){
                        memo_state_table[i][j]=pid;
                        allocated++;
                    } else {
                        memo_state_table[i][j]=MEMO_FREE;
                    }
                }
            }
        }
    }

    exu_list.push_back(pid);
    return 0;
}

int excution(){
    assert(running_list->pc < running_list->code.size());
    code_t cur_code;
    bool go_waiting=false;
    cur_code=running_list->code[running_list->pc];
    switch (cur_code.op){
    case CALC: case RDFL: case WTFL:
        //just excute it!
        break;
    case RDLK:
        if (resource[cur_code.addition][1] == 0)
            resource[cur_code.addition][0]++;
        else{
            go_waiting=true;
        }
        break;
    case WTLK:
        if (resource[cur_code.addition][0] == 0 && resource[cur_code.addition][1] == 0)
            resource[cur_code.addition][1]++;
        else{
            go_waiting=true;
        }
        break;
    case URLK:
        resource[cur_code.addition][0]--;
        break;
    case UWLK:
        resource[cur_code.addition][1]--;
        break;
    }

    running_list->pc++;
    if (go_waiting){
        running_list->status=PROC_WAITING;
        running_list->next=NULL;
        if (waiting_list){
            PCB *tempw=waiting_list;
            while (tempw->next)
                tempw=tempw->next;
            tempw->next=running_list;
        }else {
            waiting_list=running_list;
        }
        running_list=NULL;
    }

    return 0;
}

int running(){
    if (interuption.size()!=0){
        //interupted
        //update display
        vector <int> temp;
        for (int i=0; i<PCB_LIST_SIZE;i++)
            if (pcb_list[i].status == PROC_RUNNING)
                temp.push_back((-1)*pcb_list[i].status);
            else
                temp.push_back(pcb_list[i].status);
        if (temp.size() < 10)
        {
            for (; temp.size() < 10; temp.push_back(0));
        }
        proc_state_table.push_back(temp);

        //update interupiton
        interuption[interuption.size()-1]--;
        if (interuption[interuption.size()-1]==0)
            interuption.pop_back();

    } else {
        if (running_list != NULL){
            res_manage();
            excution();
        }

        //update display
        vector <int> temp;
        for (int i=0; i<PCB_LIST_SIZE ;i++)
            temp.push_back(pcb_list[i].status);
        if (temp.size() < 10)
        {
            for (; temp.size() < 10; temp.push_back(0));
        }
        proc_state_table.push_back(temp);
    }
    ++cur_proc_table_idx;
    return 0;
}



int init(){
    //resourse init
    memset(resource,0,sizeof(int)*RESOURCE_SIZE*2);
    for (int i=0;i<MEMO_MAT_WIDTH;i++)
        for (int j=0;j<MEMO_MAT_HEIGHT;j++)
            memo_state_table[i][j]=MEMO_FREE;

    //list init
    running_list=waiting_list=ready_list=NULL;
    //parameter init
    continuation=0;
    pcb_pos=0;

    return 0;
}
