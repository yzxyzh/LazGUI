#pragma once
#include <QDialog>
//#include <QWidget>
#include <QString>
namespace Ui {
    class newPathDlg;
}

class newPathDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit newPathDlg(QWidget* parent = 0);
    ~newPathDlg();
    
    void returnName(QString& name,bool& is_path);
protected:
    

private:
    Ui::newPathDlg* ui;
};