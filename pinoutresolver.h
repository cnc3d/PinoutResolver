#ifndef PINOUTRESOLVER_H
#define PINOUTRESOLVER_H

#include <QMainWindow>

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
};

#endif // PINOUTRESOLVER_H
