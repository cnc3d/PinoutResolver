#include "pinoutresolver.h"
#include "ui_pinoutresolver.h"

PinoutResolver::PinoutResolver(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PinoutResolver)
{
    ui->setupUi(this);
}

PinoutResolver::~PinoutResolver()
{
    delete ui;
}
