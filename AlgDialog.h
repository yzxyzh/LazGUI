#ifndef ALGDIALOG_H
#define ALGDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QPushButton>
#include <string>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
using namespace std;
class AlgDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AlgDialog(QWidget *parent = 0,const string& _algName = "");
    ~AlgDialog();
    
protected://protected functions
    void PhaseComboBox(const string& Type, QComboBox* inComboBox);
    
protected://protected variables
    QPushButton        runBtn;
    string             algName;
    QLabel*            inBigLabel;
    QLabel*            outBigLabel;
    QLabel**           inLabels;
    QLabel**           outLabels;
    QComboBox**        inNames;
    QComboBox**        outNames;
    QTextEdit**        inDefValues;
    
    int                inSize;
    int                outSize;
    
signals:
    
public slots:
    void RunAlg();
};

#endif // ALGDIALOG_H
