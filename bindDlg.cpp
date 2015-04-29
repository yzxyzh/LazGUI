#include "ui_bindDlg.h"
#include "bindDlg.h"
#include <QTextBlock>

#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace std;

bindDlg::bindDlg(QWidget* parent,Lazarus::LazScript& _inScript):
QDialog(parent),
ui(new Ui::bindDlg)
{
    ui->setupUi(this);
    this->script = _inScript;
    
    //ParallelTest test(script);
    test = new ParallelTest(script);
    
    int size = test->gui_need_replace.size();
    
    for (int i=0; i<size; i++) {
        int lineNum = test->gui_need_replace[i];
        QString discription = "line : "+QString::fromStdString(boost::lexical_cast<string>(lineNum))+" procname = "+QString::fromStdString(script[lineNum].procName);
        //ui->indexSelector->addItem(discription);
        ui->procSelector->addItem(discription);
        ui->procSelector->setCurrentIndex(0);
    }
    
    connect(ui->procSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeIndex(int)));
    
    UpdateIOSelection();
    
    
    
}

void bindDlg::ReturnBindResult(int &line, bool &is_in, int &index)
{
    int selectedIndex = ui->procSelector->currentIndex();
    line = test->gui_need_replace[selectedIndex];
    
    QString value = ui->ioSelector->itemText(ui->ioSelector->currentIndex());
    is_in = (value == "IN");
    
    index = ui->indexSelector->currentIndex();
}

void bindDlg::UpdateIOSelection()
{
    ui->indexSelector->clear();
    ui->ioSelector->clear();
    
    int index = ui->procSelector->currentIndex();
    if(index<0 || index>=test->gui_need_replace.size()) return;
    
    string procName = script[test->gui_need_replace[index]].procName;
    
    //cout<<"procedure name = "<<procName<<endl;
    //auto iter =
    int which = std::distance(test->replace_gui_procedures.begin(),
                              std::find(test->replace_gui_procedures.begin(),test->replace_gui_procedures.end(),procName));
    
    //cout<<"which = "<<which<<endl;
    if(which>=test->replace_gui_procedures.size()) return;
    
    int in_size = test->gui_input_var_num[which];
    int out_size = test->gui_output_var_num[which];
    
    //cout<<"inSize = "<<in_size<<" outSize = "<<out_size<<endl;
    
    if(in_size>0)
    {
        ui->ioSelector->addItem("IN");
        for (int i=0; i<in_size; i++) {
            ui->indexSelector->addItem(QString::fromStdString(boost::lexical_cast<string>(i)));
        }
    }
    
    if(out_size>0)
    {
        ui->ioSelector->addItem("OUT");
        for (int i=0; i<out_size; i++) {
            ui->indexSelector->addItem(QString::fromStdString(boost::lexical_cast<string>(i)));
        }
    }

    
}

void bindDlg::ChangeIndex(int index)
{
    UpdateIOSelection();
}


bindDlg::~bindDlg()
{
    delete ui;
    if(test) delete test;
    
}

