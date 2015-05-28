
#include "ui_GenPathDialog.h"
#include "GenPathDialog.h"

GenPathDialog::GenPathDialog(QWidget* p,vector<string>& existPathNames)
:QDialog(p)
{
    ui = new Ui::genPathDlg;
    ui->setupUi(this);
    
    int extPathSize = existPathNames.size();
    
    for (int i=0; i<extPathSize; i++) {
        this->ui->pathSelector->addItem(QString::fromStdString(existPathNames[i]));
    }
}

void GenPathDialog::GenerateData()
{
    this->pathName = this->ui->pathSelector->currentText().toStdString();
    this->extName = this->ui->extNameEditor->toPlainText().toStdString();
    this->newName = this->ui->pathNameEditor->toPlainText().toStdString();
    QString TFAbsolute = this->ui->isPathAbsolute->currentText();
    
    if(0 ==  TFAbsolute.compare("TRUE"))
    {
        this->isAbsolutePath = true;
    }else{
        this->isAbsolutePath = false;
    }
    
}

GenPathDialog::~GenPathDialog()
{
    delete ui;
}