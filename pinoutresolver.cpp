#include "pinoutresolver.h"
#include "ui_pinoutresolver.h"

#include <QtXml>
#include <QMessageBox>
#include <QDebug>

PinoutResolver::PinoutResolver(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PinoutResolver)
{
    ui->setupUi(this);

    LoadXmlPinout("pinout.xml");
}

PinoutResolver::~PinoutResolver()
{
    delete ui;
}

void PinoutResolver::LoadXmlPinout(QString filename)
{
#ifdef ASK_FILE
    QString fileName =
            QFileDialog::getOpenFileName(this, tr("Open File"),
                                         QDir::currentPath(),
                                         tr("XML Files (*.xml)"));
    if (fileName.isEmpty())
        return;
#else
    QString fileName(filename);
#endif

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("PinoutResolver"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QDomDocument domDocument;

    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!domDocument.setContent(&file, &errorStr, &errorLine,
                                &errorColumn)) {
        QMessageBox::information(window(), tr("PinoutResolver"),
                                 tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        //return false;
    }

    QDomElement root = domDocument.documentElement();
    if (root.tagName() != "Pinout")
    {
        QMessageBox::information(window(), tr("CraneSim"),
                                 tr("The file is not an Assembly file."));
        //return false;
    }
    else if (root.hasAttribute("version")
               && root.attribute("version") != "1.0")
    {
        QMessageBox::information(window(), tr("CraneSim"),
                                 tr("The file is not an Assembly version 1.0 "
                                    "file."));
        //return false;
    }

    QDomNodeList nodes = root.childNodes();
    qDebug() << "nodes.count() = " << nodes.count();
    for (int i=0; i<nodes.count(); i++)
    {
        if (nodes.at(i).nodeName() == "Peripheral")
        {
            qDebug() << "Found Peripheral : " << nodes.at(i).attributes().namedItem("name").nodeValue() << " / Node" << i;
        }
    }

}
