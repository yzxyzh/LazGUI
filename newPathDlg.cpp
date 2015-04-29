#include "ui_newPathDlg.h"
#include "newPathDlg.h"
#include <QTextBlock>


newPathDlg::newPathDlg(QWidget* parent):
QDialog(parent),
ui(new Ui::newPathDlg)
{
    ui->setupUi(this);
}

newPathDlg::~newPathDlg()
{
    delete ui;
}

void newPathDlg::returnName(QString &name, bool& is_path)
{
    name = ui->plainTextEdit->document()->firstBlock().text();
    if(ui->comboBox->currentText() == ui->comboBox->itemText(0))
    {
        is_path = true;
    }else{
        is_path = false;
    }
    
}