#pragma once
#include <QDialog>
#include <QWidget>

#include <string>

using namespace std;

namespace Ui {
    class genPathDlg;
}

class GenPathDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit GenPathDialog(QWidget* parent, vector<string>& existPathNames);
    ~GenPathDialog();
    
    void GenerateData();
    
//protected:
    string pathName;
    string extName;
    string newName;
    int    pathSize;
    bool   isAbsolutePath;
    
private:
    Ui::genPathDlg* ui;
};
