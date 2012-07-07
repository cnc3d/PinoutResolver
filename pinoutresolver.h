#ifndef PINOUTRESOLVER_H
#define PINOUTRESOLVER_H

#include <QMainWindow>
#include <QMap>

#include <QtXml>


class ResolveData
{
public:
    ResolveData() {_usedPin = 0;_usedPeripherals = 0;}
    ~ResolveData() {}

    void setDescription(QString description) {_description = description;}
    QString getDescription() {return _description;}

    void setUsedPin(quint64 pin) {_usedPin = pin;}
    quint64 getUsedPin() {return _usedPin;}
    bool IsPinAvailable(quint64 pin) {return ( (_usedPin & pin) == 0);}

private:
    quint64 _usedPin;
    quint64 _usedPeripherals;

    QString _description;
};

class ResolveTree
{

public:
    ResolveTree(ResolveTree* parent = NULL) {_parent = parent; _child = NULL; _previous = NULL; _next = NULL;}
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
    
private:
    Ui::PinoutResolver *ui;

    QDomElement LoadXml(QString filename, QString rootTag);
    void LoadPeripheralList(QDomElement root);
    void ListPeripheralPinout(QDomElement root);
    void LoadRequest(QDomElement root);
    void preparePinMap();
    void resolve(QDomNode pinout);

    QMap<QString, quint64> _peripheralsMap;
    QMap<QString, quint64> _pinoutMap;
    QMap<QString, int> _peripheralsRequested;

    ResolveTree _treeRoot;
};

#endif // PINOUTRESOLVER_H
