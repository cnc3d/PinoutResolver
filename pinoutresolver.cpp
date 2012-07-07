#include "pinoutresolver.h"
#include "ui_pinoutresolver.h"

#include <QMessageBox>
#include <QDebug>

PinoutResolver::PinoutResolver(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PinoutResolver)
{
    ui->setupUi(this);

    QDomElement root = LoadXml("pinout.xml","Device");
    QDomElement req = LoadXml("request.xml","Request");


    LoadPeripheralList(root);
    ListPeripheralPinout(root);

    preparePinMap();

    LoadRequest(req);

    qDebug() << _peripheralsMap;
    qDebug() << _pinoutMap;
    qDebug() << _peripheralsRequested;


    resolve(root.namedItem("Pinout"));

}

PinoutResolver::~PinoutResolver()
{
    delete ui;
}

QDomElement PinoutResolver::LoadXml(QString filename, QString rootTag)
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
        exit(1);
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
    if (root.tagName() != rootTag)
    {
        QMessageBox::information(window(), tr("PinoutResolver"),
                                 "The file " + filename + "is not a" + rootTag + " file.");
        //return false;
    }
    else if (root.hasAttribute("version")
               && root.attribute("version") != "1.0")
    {
        QMessageBox::information(window(), tr("PinoutResolver"),
                                 "The file " + filename +" is not a" + rootTag + "version 1.0 file.");
        //return false;
    }

    return root;
}

void PinoutResolver::LoadPeripheralList(QDomElement root)
{
    qDebug() << root.childNodes().count();
    QDomNodeList peripheralNodes = root.namedItem("Peripherals").childNodes();
    qDebug() << "peripheralNodes.count() = " << peripheralNodes.count();
    for (int i=0; i<peripheralNodes.count(); i++)
    {
        if (peripheralNodes.at(i).nodeName() == "Peripheral")
        {
            qDebug() << "Found Peripheral : " << peripheralNodes.at(i).attributes().namedItem("name").nodeValue() << " / Node" << i;
            _peripheralsMap[peripheralNodes.at(i).attributes().namedItem("name").nodeValue()] = (1<<i);
        }
    }
}

void PinoutResolver::ListPeripheralPinout(QDomElement root)
{
    QDomNodeList pinoutNodes = root.namedItem("Pinout").childNodes();
    qDebug() << "pinoutNodes.count() = " << pinoutNodes.count();
    for (int i=0; i<pinoutNodes.count(); i++)
    {
        if (pinoutNodes.at(i).nodeName() == "Peripheral")
        {
            qDebug() << "Found Peripheral : " << pinoutNodes.at(i).attributes().namedItem("type").nodeValue() << " / Node" << i;

            QDomNodeList pinoutOptions = pinoutNodes.at(i).childNodes();
            for (int j=0; j<pinoutOptions.count(); j++)
            {
                qDebug() << "Found Option : " << pinoutOptions.at(j).attributes().namedItem("name").nodeValue();
            }
        }
    }
}

void PinoutResolver::preparePinMap()
{
    for (char port='A'; port <= 'D'; port++)
    {
        for (int pin=0; pin<16; pin++)
        {
            _pinoutMap[QString("P%1%2").arg(port).arg(pin)] =  1 << ((port-'A')*16 + pin);
        }
    }
}

void PinoutResolver::LoadRequest(QDomElement root)
{
    QDomNodeList peripheralNodes = root.childNodes();

    for (int i=0; i<peripheralNodes.count(); i++)
    {
        if (peripheralNodes.at(i).nodeName() == "Peripheral")
        {
            qDebug() << "Requested Peripheral : " << peripheralNodes.at(i).attributes().namedItem("name").nodeValue() << " / Number = " << peripheralNodes.at(i).attributes().namedItem("number").nodeValue();
            _peripheralsRequested[peripheralNodes.at(i).attributes().namedItem("name").nodeValue()] = peripheralNodes.at(i).attributes().namedItem("number").nodeValue().toInt();
        }
    }
}


void PinoutResolver::resolve(QDomNode pinout)
{
    bool finished = false;
    bool solutionFound = false;
    int count = 0;
    const int MAX_LOOP = 5;


    int currentRequest = 0;

    ResolveTree* currentTree = &_treeRoot;

    QStringList peripheralRequestedList = _peripheralsRequested.keys();
    qDebug() << "peripheralRequestedList = " << peripheralRequestedList;

    while ( (!finished) && (count < MAX_LOOP) )
    {
        QString periph = peripheralRequestedList.at(currentRequest);

        QDomNodeList options = pinout.namedItem(periph).childNodes();
        qDebug() << "Resolve : level " << currentRequest << " / " << periph << " => " << options.count() << " options";

        ResolveTree* prev = NULL;

        // Generate leaves of the current node
        for (int i=0; i<options.count(); i++)
        {
            qDebug() << options.at(i).attributes().namedItem("name").nodeValue();

            /// \todo Loop in all pin
            qDebug() << options.at(i).firstChildElement("pin").attributes().namedItem("name").nodeValue();

            quint64 pin = _pinoutMap[options.at(i).firstChildElement("pin").attributes().namedItem("name").nodeValue()];
            qDebug() << QString("%1").arg(pin, 64, 2, QLatin1Char('0'));

            if (currentTree->_data.IsPinAvailable(pin))
            {
                ResolveTree* newTree = new ResolveTree(currentTree);
                newTree->_data.setUsedPin(pin | currentTree->_data.getUsedPin());
                currentTree->_data.setDescription(periph
                                                  + " => " + options.at(i).attributes().namedItem("name").nodeValue()
                                                  + " / " + options.at(i).firstChildElement("pin").attributes().namedItem("name").nodeValue());

                if (prev == NULL)
                {
                    currentTree->_child = newTree;
                }
                else
                {
                    prev->_next = newTree;
                    newTree->_previous = prev;
                }

                prev = newTree;
            }
            else
            {
                qDebug() << "pin " << options.at(i).firstChildElement("pin").attributes().namedItem("name").nodeValue() << "is already used";
            }
        }

        // If there are child, descend inside them
        if (currentTree->_child != NULL)
        {
            qDebug() << "# down in child";
            currentTree = currentTree->_child;
            currentRequest++;
        }
        else
        {
            // Else try the next node
            if (currentTree->_next != NULL)
            {
                qDebug() << "# to next sibling";
                currentTree = currentTree->_next;
            }
            else // Otherwise go up a level, and test the next one
            {
                bool topReached = false;
                bool availableNodeFound = false;

                while ((!topReached) && (!availableNodeFound))
                {

                    if ((currentTree->_parent != NULL))
                    {
                        qDebug() << "# up to parent";
                        currentRequest--;
                        currentTree = currentTree->_parent;
                    }
                    else
                    {
                        topReached = true;
                        finished = true;
                    }

                    if (currentTree->_next != NULL)
                    {
                            currentTree = currentTree->_next;
                            availableNodeFound = true;
                    }
                }
            }
        }


        //
        if (currentRequest == (peripheralRequestedList.size()) )
        {
            qDebug() << "Solution found";
            solutionFound = true;
            break;
        }

        //
        if (currentRequest >= peripheralRequestedList.size())
        {
            finished = true;
        }
        qDebug() << "Next Loop";
    }
    qDebug() << "Loop finished (currentRequest = " << currentRequest << ")";

    if (solutionFound)
    {
        qDebug() << "Solution found :";
        while (currentTree->_parent != NULL)
        {
            qDebug() << currentTree->_data.getDescription();
            currentTree = currentTree->_parent;
        }
        qDebug() << currentTree->_data.getDescription();
    }
}

