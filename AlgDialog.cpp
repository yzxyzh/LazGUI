#include "algdialog.h"
#include <QWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <LazarusWatchDog.h>
#include <LazarusDataWarpper.h>
#include <LazarusStringOpration.h>
#include "GUITypes.h"
#include <LazarusProcedureFactory.h>
#include <iostream>

using namespace std;
using namespace Lazarus;


void AlgDialog::PhaseComboBox(const string &Type, QComboBox *inComboBox)
{
    //this function phase what shall be in a combobox by searching all instance
    //in watchdog, and find type equals to type of this combobox;
    inComboBox->addItem("<NEW>");
    vector<Instance>* insList=WatchDog::ReturnInstanceList();
    int insSize=insList->size();
    for (int i=0; i<insSize; i++) {
        if(Type == insList->at(i).type)
            inComboBox->addItem(QString::fromStdString(insList->at(i).name));
    }
}

AlgDialog::~AlgDialog()
{
    //cout<<"Calling destructor"<<endl;
    
    for (int i=0; i<inSize; i++) {
        if(inLabels[i]) delete inLabels[i];
        if(inNames[i]) delete inNames[i];
        if(inDefValues[i]) delete inDefValues[i];
    }
    if(inLabels) delete inLabels;
    if(inNames) delete inNames;
    if(inDefValues) delete inDefValues;
    
    for (int i=0; i<outSize; i++) {
        if(outLabels[i]) delete outLabels[i];
        if(outNames[i]) delete outNames[i];
    }
    if(outLabels) delete outLabels;
    if(outNames) delete outNames;
    
    
    if(inBigLabel) delete inBigLabel;
    if(outBigLabel) delete outBigLabel;
    
}

AlgDialog::AlgDialog(QWidget *parent,const string& _algName) :
algName(_algName),
inBigLabel(NULL),
outBigLabel(NULL),
inLabels(NULL),
outLabels(NULL),
inNames(NULL),
outNames(NULL),
inDefValues(NULL),
QDialog(parent)
{
    
    /**************SET GEOMETRY*******************/
    //setGeometry(0, 0, 1000, 1000);
    setWindowTitle(QString::fromStdString(algName));
    //btn.show();
    /**************SET GEOMETRY*******************/
    
    if (WatchDog::ProcedureExist(algName))
    {
        algIOTypes curProcIO=WatchDog::GetIoType(algName);
        inSize=curProcIO.inTypes.size();
        outSize=curProcIO.outTypes.size();
        inLabels=new QLabel*[inSize];
        outLabels=new QLabel*[outSize];
        inNames= new QComboBox*[inSize];
        outNames=new QComboBox*[outSize];
        inDefValues = new QTextEdit*[inSize];
        
        //New inVars;
        inBigLabel=new QLabel(this);
        inBigLabel->setText("InTypes:                                             name:                                             value:");
        inBigLabel->setStyleSheet("QLabel {color : red; }");
        //inBigLabel->setAutoFillBackground(true);
        inBigLabel->setGeometry(10, 30, 500, 20);
        for (int i=0; i<inSize; i++) {
            inLabels[i] = new QLabel(this);
            inLabels[i]->setGeometry(30, 60+30*i, 200, 20);
            inLabels[i]->setText(QString::fromStdString("TYPE:"+curProcIO.inTypes[i]));
            
            inNames[i] = new QComboBox(this);
            inNames[i]->setGeometry(260, 60+30*i, 200, 20);
            PhaseComboBox(curProcIO.inTypes[i], inNames[i]);
            
            inDefValues[i]=new QTextEdit(this);
            inDefValues[i]->setGeometry(490, 60+30*i, 200, 20);
            inDefValues[i]->setText(QString::fromStdString(curProcIO.InDefalutValues[i]));
        }
        
        
        //New outVars
        outBigLabel=new QLabel(this);
        outBigLabel->setText("outTypes:                                           name:");
        outBigLabel->setStyleSheet("QLabel {color : blue; }");
        //inBigLabel->setAutoFillBackground(true);
        outBigLabel->setGeometry(10, 60+inSize*30, 500, 20);
        for (int i=0; i<outSize; i++) {
            outLabels[i] = new QLabel(this);
            outLabels[i]->setGeometry(30, 90+30*(i+inSize), 200, 20);
            outLabels[i]->setText(QString::fromStdString("TYPE:"+curProcIO.outTypes[i]));
            
            outNames[i] = new QComboBox(this);
            outNames[i]->setGeometry(260, 90+30*(i+inSize), 200, 20);
            //PhaseComboBox(curProcIO.outTypes[i], outNames[i]);
            outNames[i]->addItem("<DEFAULT>");
        }
        
        
        //根据这个来计算主窗口的宽度
        int main_height=60*(1+inSize+outSize);
        this->setGeometry(0, 0, 800, main_height);
        //runBtn.setWindowTitle("Run Script");
        runBtn.setText("Run Script");
        runBtn.setParent(this);
        runBtn.setGeometry(650, main_height-50, 120,40 );
    }
    
    
    
    QObject::connect(&runBtn,SIGNAL(clicked()),this,SLOT(RunAlg()));
}

void AlgDialog::RunAlg()
{
    //first I need to give all varialbes a spec name;
    //in a form of PROC_X_IN_Y;, means X Proc, Y IN/OUT Variables;
    cout<<"inSize = "<<inSize<<" outSize = "<<outSize<<endl;
    string header="PROC_"+StringOperator::ToStr(LazGUI::Pool::alg_counter);
    string in_header=header+"IN_";
    string out_header = header+"OUT_";
    
    scriptUnit newUnit;
    newUnit.inputVarsFull="";
    newUnit.outputVarsFull="";
    newUnit.procName=algName;
//    newUnit.inVars.clear();
//    newUnit.outVars.clear();
//    newUnit.inDefVals.clear();
//    newUnit.outDefVals.clear();
    for (int i=0; i<inSize; i++) {
        //Set inNames;
        if("<NEW>" == inNames[i]->currentText())
            newUnit.inVars.push_back(in_header+StringOperator::ToStr(i));
        else
            newUnit.inVars.push_back(inNames[i]->currentText().toStdString());
        //Set inDefVals;
        newUnit.inDefVals.push_back(inDefValues[i]->toPlainText().toStdString());
    }
    
    for (int i=0; i<outSize; i++) {
        //Set outNames;
        newUnit.outVars.push_back(out_header+StringOperator::ToStr(i));
        //Set outDefVals;
        newUnit.outDefVals.push_back("");
    }
    
    //set full input&output vars
    for (int i=0; i<inSize-1; i++) {
        newUnit.inputVarsFull=newUnit.inputVarsFull + newUnit.inVars[i]+",";
    }
    if(newUnit.inVars.size()>0)
    newUnit.inputVarsFull+=newUnit.inVars.back();

    for (int i=0; i<outSize-1; i++) {
        newUnit.outputVarsFull=newUnit.outputVarsFull + newUnit.outVars[i]+",";
    }
    if(newUnit.outVars.size()>0)
    newUnit.outputVarsFull+=newUnit.outVars.back();

    cout<<"newUnit: in = "<<newUnit.inVars.size()<<" out: "<<newUnit.outVars.size()<<endl;
    //PhaseOneScript(newUnit);
    
    LazGUI::Pool::alg_counter++;
    LazGUI::Pool::Script.push_back(newUnit);
    
    
    this->close();
}
