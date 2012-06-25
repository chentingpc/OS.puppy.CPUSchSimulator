#ifndef INFO_H
#define INFO_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class info;
}

class info : public QWidget
{
    Q_OBJECT
    
public:
    explicit info(QWidget *parent = 0);
    ~info();

    void set_proc_table_item(int x, int y, QColor color = QColor(255, 255, 255));
    void set_memo_table_item(int x, int y, QColor color = QColor(255, 255, 255));
    void show_proc_table();
    void show_memo_table();
    
private:
    Ui::info *ui;
    QTimer *timer;

private slots:
    void execute();
    void on_pb_exec_clicked();
    void on_pb_new_intr_clicked();
};

#endif // INFO_H
