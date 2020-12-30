#ifndef JACKPARAMETERFORM_H
#define JACKPARAMETERFORM_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>


#include "lrnetjackservertest.h"

#ifdef LRNETJACKSERVERTEST_LIBRARY

#include "ui_jackparameterform.h"
#include "jackinterface.h"
#include "qjackctlInterfaceComboBox.h"

#endif



namespace Ui {
class JackParameterForm;
}

class qjackctlInterfaceComboBox;
class JackInterface;

class LRNETJACKSERVERTEST_EXPORT JackParameterForm : public QWidget
{
    Q_OBJECT
    JackInterface * jackServer;
     Ui::JackParameterForm *ui;
public:
    explicit JackParameterForm(QWidget *parent = nullptr);
    ~JackParameterForm();

     QComboBox * driverBox;
     QComboBox * m_fppChoice;
     QComboBox * m_srChoice;
     qjackctlInterfaceComboBox * iFaceBox;
     qjackctlInterfaceComboBox * oFaceBox;
     QLabel * iFaceLabel;
     QLabel * oFaceLabel;
     void sendAllParameters();

signals:
     //void setParameter(QString & name, QVariant & value);
    void jackStarted();
    void jackStopped();

private:
     void sendNewRate(const QString & rate);
     void sendNewPeriod(const QString & period);


};

#endif // JACKPARAMETERFORM_H
