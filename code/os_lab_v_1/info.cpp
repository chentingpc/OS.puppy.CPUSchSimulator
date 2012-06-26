#include "info.h"
#include "ui_info.h"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
using namespace std;
#include "lab.h"

vector<vector<int> > proc_state_table;
int cur_proc_table_idx = -1;
int memo_state_table[MEMO_MAT_WIDTH][MEMO_MAT_HEIGHT];

vector<code_t> instr_vec;

extern int continuation;
extern PCB pcb_list[PCB_LIST_SIZE];
extern int pcb_pos;
extern PCB *waiting_list,*running_list,*ready_list;
extern vector <int> interuption;
extern int resource[RESOURCE_SIZE][2];//read lock [0] and wirte lock [1]
extern vector <int> exu_list;

int new_proc_tag = 1;

const QColor COLOR_READY(0, 128, 24);
const QColor COLOR_DEAD(255, 255, 255);
const QColor COLOR_WAITING(0, 128, 255);
const QColor COLOR_RUNNING(255, 128, 192);
const QColor COLOR_INTR(255, 128, 0);

const QColor COLOR_NOT_USED_MEMO(255, 255, 255);

info::info(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::info)
{
    ui->setupUi(this);

    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(execute()));
    timer->start(1000);

    for (int i = 0; i < MEMO_MAT_WIDTH; ++i)
    {
        ui->tw_memo->insertRow(i);
        ui->tw_memo->insertColumn(i);
        ui->tw_memo->setColumnWidth(i, 60);
        ui->tw_memo->setRowHeight(i, 20);
        for (int j = 0; j < MEMO_MAT_HEIGHT; ++j)
        {
            memo_state_table[i][j] = -1;
        }
    }

    for (int i = 0; i < PROC_MAT_WIDTH; ++i)
    {
        ui->tw_proc->insertRow(i);
        ui->tw_proc->insertColumn(i);
        ui->tw_proc->setColumnWidth(i, 60);
        ui->tw_proc->setRowHeight(i, 20);
    }

    init();
}

info::~info()
{
    delete ui;
}

void info::set_proc_table_item(int x, int y, QColor color)
{
    if (x < ui->tw_proc->rowCount() && y < ui->tw_proc->columnCount() && x >= 0 && y >= 0)
    {
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setBackgroundColor(color);

        ui->tw_proc->setItem(x, y, item);
    }
}

void info::set_memo_table_item(int x, int y, QColor color)
{
    if (x < ui->tw_memo->rowCount() && y < ui->tw_memo->columnCount() && x >= 0 && y >= 0)
    {
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setBackgroundColor(color);

        ui->tw_memo->setItem(x, y, item);
    }
}

void info::execute()
{
    proc_manage();
    running();
    show_proc_table();
    show_memo_table();
}

void info::show_proc_table()
{
    if (cur_proc_table_idx >= PROC_MAT_WIDTH)
    {
        ui->tw_proc->insertColumn(PROC_MAT_WIDTH);
        ui->tw_proc->setColumnWidth(PROC_MAT_WIDTH, 60);
        ui->tw_proc->removeColumn(0);
        for (int idx = 0; idx < PROC_MAT_WIDTH; ++idx)
        {
            QTableWidgetItem *item = new QTableWidgetItem;
            if (IS_PROC_READY(proc_state_table[cur_proc_table_idx][idx]))
            {

                item->setBackgroundColor(COLOR_READY);
                ui->tw_proc->setItem(idx, PROC_MAT_WIDTH - 1, item);
            }
            else if (IS_PROC_DEAD(proc_state_table[cur_proc_table_idx][idx]))
            {
                item->setBackgroundColor(COLOR_DEAD);
                ui->tw_proc->setItem(idx, PROC_MAT_WIDTH - 1, item);
            }
            else if (IS_PROC_WAITING(proc_state_table[cur_proc_table_idx][idx]))
            {
                item->setBackgroundColor(COLOR_WAITING);
                ui->tw_proc->setItem(idx, PROC_MAT_WIDTH - 1, item);
            }
            else if (IS_PROC_RUNNING(proc_state_table[cur_proc_table_idx][idx]))
            {
                item->setBackgroundColor(COLOR_RUNNING);
                ui->tw_proc->setItem(idx, PROC_MAT_WIDTH - 1, item);
            }
            else if (IS_PROC_INTR(proc_state_table[cur_proc_table_idx][idx]))
            {
                item->setBackgroundColor(COLOR_INTR);
                ui->tw_proc->setItem(idx, PROC_MAT_WIDTH - 1, item);
            }
        }
    }
    else
    {
        for (int idx = 0; idx < PROC_MAT_HEIGHT; ++idx)
        {
            QTableWidgetItem *item = new QTableWidgetItem;
            if (IS_PROC_READY(proc_state_table[cur_proc_table_idx][idx]))
            {

                item->setBackgroundColor(COLOR_READY);
                ui->tw_proc->setItem(idx, cur_proc_table_idx, item);
            }
            else if (IS_PROC_DEAD(proc_state_table[cur_proc_table_idx][idx]))
            {
                item->setBackgroundColor(COLOR_DEAD);
                ui->tw_proc->setItem(idx, cur_proc_table_idx, item);
            }
            else if (IS_PROC_WAITING(proc_state_table[cur_proc_table_idx][idx]))
            {
                item->setBackgroundColor(COLOR_WAITING);
                ui->tw_proc->setItem(idx, cur_proc_table_idx, item);
            }
            else if (IS_PROC_RUNNING(proc_state_table[cur_proc_table_idx][idx]))
            {
                item->setBackgroundColor(COLOR_RUNNING);
                ui->tw_proc->setItem(idx, cur_proc_table_idx, item);
            }
            else if (IS_PROC_INTR(proc_state_table[cur_proc_table_idx][idx]))
            {
                item->setBackgroundColor(COLOR_INTR);
                ui->tw_proc->setItem(idx, cur_proc_table_idx, item);
            }
        }
    }
}

void info::show_memo_table()
{
    for (int i = 0; i < MEMO_MAT_WIDTH; ++i)
    {
        for (int j = 0; j < MEMO_MAT_HEIGHT; ++j)
        {
            QTableWidgetItem *item = new QTableWidgetItem;
            if (MEMO_FREE == memo_state_table[i][j])
            {
                item->setBackgroundColor(COLOR_NOT_USED_MEMO);
                ui->tw_memo->setItem(i, j, item);
            }
            else
            {
                int r((memo_state_table[i][j] * 25) % 255);
                int g((memo_state_table[i][j] * 50) % 255);
                int b((memo_state_table[i][j] * 75) % 255);
                QColor proc_color(r, g, b);
                item->setBackgroundColor(proc_color);
                ui->tw_memo->setItem(i, j, item);
            }
        }
    }
}

void info::on_pb_exec_clicked()
{
    QStringList code_list(ui->te_code->toPlainText().split("\n"));
    vector<code_t> code;

    for (int i = 0; i < code_list.size(); ++i)
    {
        QStringList code_unit(code_list[i].split(' '));
        if ("CALC" == code_unit[0])
        {
            code_t t;
            t.addition = 0;
            t.op = CALC;
            int rep_times(code_unit[1].toInt());
            for (int j = 0; j < rep_times; ++j)
            {
                code.push_back(t);
            }
        }
        else if ("RDLK" == code_unit[0])
        {
            int res_no(code_unit[1].toInt());
            code_t t;
            t.op = RDLK;
            t.addition = res_no;
            code.push_back(t);
        }
        else if ("WTLK" == code_unit[0])
        {
            int res_no(code_unit[1].toInt());
            code_t t;
            t.op = WTLK;
            t.addition = res_no;
            code.push_back(t);
        }
        else if ("RDFL" == code_unit[0])
        {
            code_t t;
            t.addition = 0;
            t.op = RDFL;
            int rep_times(code_unit[1].toInt());
            for (int j = 0; j < rep_times; ++j)
            {
                code.push_back(t);
            }
        }
        else if ("WTFL" == code_unit[0])
        {
            code_t t;
            t.addition = 0;
            t.op = WTFL;
            int rep_times(code_unit[1].toInt());
            for (int j = 0; j < rep_times; ++j)
            {
                code.push_back(t);
            }
        }
        else if ("URLK" == code_unit[0])
        {
            int res_no(code_unit[1].toInt());
            code_t t;
            t.op = URLK;
            t.addition = res_no;
            code.push_back(t);
        }
        else if ("UWLK" == code_unit[0])
        {
            int res_no(code_unit[1].toInt());
            code_t t;
            t.op = UWLK;
            t.addition = res_no;
            code.push_back(t);
        }
    }

    //input
    int pid=pcb_pos;
    for (int i = 0; i < pcb_pos; ++i)
    {
        if (PROC_DEAD == pcb_list[i].status)
        {
            pid = pcb_list[i].pid;
            break;
        }
    }


    pcb_list[pid].pid=pid;
    pcb_list[pid].next=NULL;
    pcb_list[pid].mem_size=ui->le_memo->text().toInt();
    pcb_list[pid].code = code;
    pcb_list[pid].status=PROC_READY;
    pcb_list[pid].pc=0;
    pcb_list[pid].priority=(ui->cb_pri->currentIndex() == 0?PRI_LOW:PRI_HIGH);
    if (pid == pcb_pos)
        pcb_pos++;
    if (!ready_list)
        ready_list=&pcb_list[pid];
    else{
        PCB *temp=ready_list;
        while(temp->next)
            temp=temp->next;
        temp->next=&pcb_list[pid];
    }
}

void info::on_pb_new_intr_clicked()
{
    interuption.push_back(1);
}
