#ifndef PINOUTRESOLVER_H
#define PINOUTRESOLVER_H

#include <QMainWindow>
#include <QMap>
#include <QMultiMap>


#include <QtXml>


class ResolveData
{
public:
    ResolveData() {_usedPin = 0;_usedPeripherals = 0;}
    ~ResolveData() {}

    void setDescription(QString description) {_description = description;}
    QString getDescription() {return _description;}

    void setUsedPin(quint64 pin) {_usedPin = pin;}
    void addUsedPin(quint64 pin)
    {
        if(IsPinAvailable(pin))
        {
            _usedPin |= pin;
        }
        else
        {
            qDebug() << "pin already used" << (pin & _usedPin);
        }
    }

    quint64 getUsedPin() {return _usedPin;}
    bool IsPinAvailable(quint64 pin) {return ( (_usedPin & pin) == 0);}

    void setUsedPeripheral(quint64 peripheral) {_usedPeripherals = peripheral;}
    void addUsedPeripheral(quint64 peripheral) {_usedPeripherals |= peripheral;}
    quint64 getUsedPeripheral() {return _usedPeripherals;}
    bool IsPeripheralAvailable(quint64 peripheral) {return ( (_usedPeripherals & peripheral) == 0);}

private:
    quint64 _usedPin;
    quint64 _usedPeripherals;

    QString _description;
};

class ResolveTree
{

public:
    ResolveTree(ResolveTree* parent = NULL) {_parent = parent; _child = NULL; _previous = NULL; _next = NULL;
                                             if(parent != NULL) {_data.setUsedPin(parent->_data.getUsedPin());}
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
