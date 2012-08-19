#ifndef PINOUTRESOLVER_H
#define PINOUTRESOLVER_H

#include <QMainWindow>
#include <QMap>
#include <QMultiMap>


#include <QtXml>


const unsigned int pin_T_array_size = 2; // How many times 64 pins can be handled by the resolver (1=64, 2=128, ...)

class pin_T
{
public:
    pin_T()
    {
        for (int i=0; i<pin_T_array_size; i++) { value[i] = 0; }
    }

    pin_T(const pin_T& source)
    {
        for (int i=0; i<pin_T_array_size; i++) { value[i] = source.value[i]; }
    }

    bool operator == (const pin_T other)
    {
        bool res = true;
        for (int i=0; i<pin_T_array_size; i++)
        {
            res = res && (value[i] == other.value[i]);
        }
        return res;
    }

    bool operator = (const pin_T other)
    {
        for (int i=0; i<pin_T_array_size; i++)
        {
            value[i] = other.value[i];
        }
    }

    bool isAvailable(const unsigned int pinNb)
    {
        unsigned int nbArray = pinNb % 64;
        unsigned int nbOffset = pinNb - 64*nbArray;

        return (value[nbArray] & (1 << nbOffset) == 0);
    }

    void setUsed(const unsigned int pinNb)
    {
        unsigned int nbArray = pinNb % 64;
        unsigned int nbOffset = pinNb - 64*nbArray;

        value[nbArray] |= (1 << nbOffset);
    }

    void addUsed(const unsigned int pinNb)
    {

    }

private:
    quint64 value[pin_T_array_size];
};

class ResolveData
{
public:
    ResolveData() {_usedPeripherals = 0;}
    ~ResolveData() {}

    void setDescription(QString description) {_description = description;}
    QString getDescription() {return _description;}

    void setUsedPin(const unsigned int pinNb) {_usedPin.setUsed(pinNb);}
    void setAllUsedPin(pin_T pin) {_usedPin = pin;}
    void addUsedPin(const unsigned int pinNb)
    {
        if(IsPinAvailable(pinNb))
        {
            _usedPin.setUsed(pinNb);
        }
        else
        {
            qDebug() << "pin already used" << pinNb;
        }
    }

    pin_T getAllUsedPin() {return _usedPin;}
    bool IsPinAvailable(const unsigned int pinNb) {return _usedPin.isAvailable(pinNb);}

    void setUsedPeripheral(quint64 peripheral) {_usedPeripherals = peripheral;}
    void addUsedPeripheral(quint64 peripheral) {_usedPeripherals |= peripheral;}
    quint64 getUsedPeripheral() {return _usedPeripherals;}
    bool IsPeripheralAvailable(quint64 peripheral) {return ( (_usedPeripherals & peripheral) == 0);}

private:
    pin_T _usedPin;
    quint64 _usedPeripherals;

    QString _description;
};

class ResolveTree
{

public:
    ResolveTree(ResolveTree* parent = NULL) {_parent = parent; _child = NULL; _previous = NULL; _next = NULL;
                                             if(parent != NULL) {_data.setAllUsedPin(parent->_data.getAllUsedPin());}
                                             if(parent != NULL) {_data.setUsedPeripheral(parent->_data.getUsedPeripheral());}
                                            }
    ~ResolveTree() {}

    ResolveData _data;

    ResolveTree* _parent;
    ResolveTree* _child;
    ResolveTree* _previous;
    ResolveTree* _next;

private:

};

namespace Ui {
class PinoutResolver;
}

class PinoutResolver : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit PinoutResolver(QWidget *parent = 0);
    ~PinoutResolver();
    
    void PrintTree();
    QString pinoutToText(quint64 pinout);

private:
    Ui::PinoutResolver *ui;

    QDomElement LoadXml(QString filename, QString rootTag);
    void LoadPeripheralList(QDomElement root);
    void LoadDevicePinout(QDomElement root);
    void ListFunctions(QDomElement root);
    void LoadRequest(QDomElement root);
    void preparePinMap();
    void resolve(QDomNode pinout);
    QList<quint64> GetPinoutCartesianProduct(QDomNodeList function);

    QMap<QString, quint64> _peripheralsMap;
    QMap<QString, quint64> _pinoutMap;
    QStringList _peripheralsRequested;
    QMultiMap<QString, QString> _alternatePinoutMap;

    ResolveTree _treeRoot;
};

#endif // PINOUTRESOLVER_H
