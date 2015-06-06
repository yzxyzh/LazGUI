#ifndef DUPPATHDLG_H
#define DUPPATHDLG_H

#include <QDialog>
#include <string>
#include <vector>

using namespace std;

namespace Ui {
class DupPathDlg;
}

class DupPathDlg : public QDialog
{
    Q_OBJECT

public:
    explicit DupPathDlg(QWidget *parent = 0);
    ~DupPathDlg();

    string pathInfo;
    bool   isAbsolutePath;
    
    int  GetDupNums();
    string GetName();
    
protected slots:
    
    void OpenPath();
    void OpenFile();
    
private:
    Ui::DupPathDlg *ui;
};

#endif // DUPPATHDLG_H
