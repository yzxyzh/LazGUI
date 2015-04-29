#pragma once
#include <QDialog>
//#include <QWidget>
#include <QString>
#include <LazarusDataWarpper.h>
#include "ParallelTest.h"

namespace Ui {
    class bindDlg;
}

class bindDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit bindDlg(QWidget* parent,Lazarus::LazScript& _script);
    ~bindDlg();
    
    void ReturnBindResult(int& line,bool& is_in,int& index);
    
protected:
    
    void                UpdateIOSelection();
    void                UpdateIndexSelection();
    
    Lazarus::LazScript script;
    ParallelTest*       test;

private:
    Ui::bindDlg* ui;
    
public slots:
    void ChangeIndex(int index);
};