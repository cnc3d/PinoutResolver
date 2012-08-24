#include "pinoutresolver.h"
#include "ui_pinoutresolver.h"

#include <QMessageBox>
#include <QDebug>

PinoutResolver::PinoutResolver(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PinoutResolver)
{
    ui->setupUi(this);

    QDomElement root = LoadXml("devices/STM32F407.xml","Device");
    QDomElement req = LoadXml("request.xml","Request");

    LoadPeripheralList(root);
    LoadDevicePinout(root);
    ListFunctions(root);

    preparePinMap();

    LoadRequest(req);

    qDebug() << _peripheralsMap;
    qDebug() << _alternatePinoutMap;
    qDebug() << _pinoutMap;
    qDebug() << _peripheralsRequested;


    resolve(root.namedItem("Functions"));

    exit(0);
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

void PinoutResolver::LoadDevicePinout(QDomElement root)
{
    qDebug() << root.childNodes().count();
    QDomNodeList pinoutNodes = root.namedItem("Pinout").childNodes();
    qDebug() << "pinoutNodes.count() = " << pinoutNodes.count();
    for (int i=0; i<pinoutNodes.count(); i++)
    {
        if (pinoutNodes.at(i).nodeName() == "Pin")
        {
            QDomNodeList alternatePinoutNodes = pinoutNodes.at(i).childNodes();

            qDebug() << "Found Pin : " << pinoutNodes.at(i).attributes().namedItem("port").nodeValue() << " with " << alternatePinoutNodes.count() << " alternate functions";

            for (int j=0; j<alternatePinoutNodes.count(); j++)
            {
                if (alternatePinoutNodes.at(j).nodeName() == "Alternate")
                {
                    _alternatePinoutMap.insert(alternatePinoutNodes.at(j).attributes().namedItem("name").nodeValue(), pinoutNodes.at(i).attributes().namedItem("port").nodeValue());
                }
                else
                {
                    qDebug() << "Uknown Pin tag : " << alternatePinoutNodes.at(j).nodeName();
                }
            }
        }
        else
        {
            qDebug() << "Uknown Pinout tag : " << pinoutNodes.at(i).nodeName();
        }
    }
}

void PinoutResolver::ListFunctions(QDomElement root)
{
    QDomNodeList functionsNodes = root.namedItem("Functions").childNodes();
    qDebug() << "functionsNodes.count() = " << functionsNodes.count();
    for (int i=0; i<functionsNodes.count(); i++)
    {
        if (functionsNodes.at(i).nodeName() == "Peripheral")
        {
            qDebug() << "Found Function : " << functionsNodes.at(i).attributes().namedItem("type").nodeValue() << " / Node" << i;

            QDomNodeList functionsOptions = functionsNodes.at(i).childNodes();
            for (int j=0; j<functionsOptions.count(); j++)
            {
                qDebug() << "Found Option : " << functionsOptions.at(j).attributes().namedItem("name").nodeValue();
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
            quint64 decal = (quint64)((port-'A')*16 + pin);
            quint64 bitPos = 1ULL << decal; // 1ULL => to define '1' as a 64 bits constant, otherwise, it defaults to 32 bits, and the shift fails
            //qDebug() << "pinoutMap : " << decal << QString("%1").arg(bitPos,64,2,QLatin1Char('0'));
            _pinoutMap[QString("P%1%2").arg(port).arg(pin)] =  bitPos;//1 << ((port-'A')*16 + pin);
        }
    }
}

QString PinoutResolver::pinoutToText(quint64 pinout)
{
    QList<QString> keys = _pinoutMap.keys();

    QString result;

    for (int i=0; i<keys.size(); i++)
    {
        if (_pinoutMap[keys.at(i)] & pinout)
        {
            result += keys.at(i) + " ";
        }
    }

    return result;
}

void PinoutResolver::LoadRequest(QDomElement root)
{
    QDomNodeList peripheralNodes = root.childNodes();

    for (int i=0; i<peripheralNodes.count(); i++)
    {
        if (peripheralNodes.at(i).nodeName() == "Peripheral")
        {
            qDebug() << "Requested Peripheral : " << peripheralNodes.at(i).attributes().namedItem("name").nodeValue() << " / Number = " << peripheralNodes.at(i).attributes().namedItem("number").nodeValue();
            for (int num = 0; num <  peripheralNodes.at(i).attributes().namedItem("number").nodeValue().toInt(); num++)
            {
                _peripheralsRequested << peripheralNodes.at(i).attributes().namedItem("name").nodeValue();
            }
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

    QStringList peripheralRequestedList = _peripheralsRequested;
    qDebug() << "peripheralRequestedList = " << peripheralRequestedList;

    while ( (!finished) && (count < MAX_LOOP) )
    {
        qDebug() << "new Loop with"
                 << currentTree->_data.getUsedPeripheral()
                 << currentTree->_data.getUsedPin()
                 << currentTree->_data.getDescription();

        QString reqPeriph = peripheralRequestedList.at(currentRequest);

        QDomNodeList options = pinout.namedItem(reqPeriph).childNodes();
        qDebug() << "Resolve : level " << currentRequest << " / " << reqPeriph << " => " << options.count() << " options";

        ResolveTree* prev = NULL;

        // Generate leaves of the current node
        for (int i=0; i<options.count(); i++)
        {
            qDebug() << options.at(i).attributes().namedItem("name").nodeValue();

            QList<quint64> pinoutCartesianProduct = GetPinoutCartesianProduct(options.at(i).childNodes());

            foreach(quint64 pin, pinoutCartesianProduct)
            {
                qDebug() << QString("Trying : %1").arg(pin, 64, 2, QLatin1Char('0'));

                quint64 periph = _peripheralsMap[options.at(i).attributes().namedItem("name").nodeValue()];
                if (currentTree->_data.IsPinAvailable(pin) && currentTree->_data.IsPeripheralAvailable(periph))
                {
                    ResolveTree* newTree = new ResolveTree(currentTree);
                    newTree->_data.addUsedPin(pin);// | currentTree->_data.getUsedPin());
                    newTree->_data.addUsedPeripheral(periph);

                    QString desc = reqPeriph
                                   + " => " + options.at(i).attributes().namedItem("name").nodeValue() + " with pin : ";
                    for(int nodeId = 0; nodeId < options.at(i).childNodes().count(); nodeId++)
                    {
                        desc += options.at(i).childNodes().at(nodeId).attributes().namedItem("name").nodeValue() + " / ";
                    }
                    //desc += QString(" %1").arg(pin,64,2,QLatin1Char('0'));
                    desc += pinoutToText(pin);
                    newTree->_data.setDescription(desc);

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
                    /// \todo Print real pin in problem, not the first one
                    qDebug() << "pin " << options.at(i).firstChildElement("pin").attributes().namedItem("name").nodeValue() << "is already used";
                }
            }
        }

        // If there are child, descend inside them
        if (currentTree->_child != NULL)
        {
            qDebug() << "# down in child" << currentTree->_child->_data.getDescription();
            currentTree = currentTree->_child;
            currentRequest++;
        }
        else
        {
            // Else try the next node
            if (currentTree->_next != NULL)
            {
                qDebug() << "# to next sibling" << currentTree->_next->_data.getDescription();
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

    PrintTree();
}

QList<quint64> PinoutResolver::GetPinoutCartesianProduct(QDomNodeList function)
{
    QList<quint64> cartesianProduct;

    QVector< QList<QString> > pinConfigs;
    QVector<int> currentPos;

    for (int i=0; i < function.count() ; i++)
    {
        QList<QString> alternate = _alternatePinoutMap.values(function.at(i).attributes().namedItem("name").nodeValue());
        pinConfigs.append(alternate);
        currentPos.append(0);
    }

    qDebug() << "pinConfigs =" << pinConfigs;
    bool finished = false;
    while(!finished)
    {
        int currentPosInVector = 0;
        while ( currentPos[currentPosInVector] == pinConfigs.at(currentPosInVector).size() )
        {
            currentPos[currentPosInVector] = 0;
            currentPosInVector++;

            if(currentPosInVector == pinConfigs.size())
            {
                finished = true;
                break;
            }
            currentPos[currentPosInVector]++;
        }

        if(!finished)
        {
            //qDebug() << "new config = " << currentPos;

            quint64 pin = 0;
            for(int i=0; i<pinConfigs.size(); i++)
            {
                //qDebug() << "Pin : " << pinConfigs[i].at(currentPos[i]);
                pin |= _pinoutMap[pinConfigs[i].at(currentPos[i])];
            }
            cartesianProduct.append(pin);

            currentPos[0] ++;
        }
    }

    return cartesianProduct;
}


void PinoutResolver::PrintTree()
{
    ResolveTree* currentTree = &_treeRoot;

    bool finished = false;

    QFile file("tree.dot");
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);

    out << "digraph tree {" << endl;
    out << "node [shape=rect];" << endl;

    while (!finished && (currentTree != NULL))
    {
        //qDebug() << currentTree->_data.getDescription() << "[" << currentTree->_child << currentTree->_previous << currentTree->_parent << "]";

        out << (quint64) currentTree << "[label=\"" << currentTree->_data.getDescription() << "\"]";
            //<< QString(" %1").arg(currentTree->_data.getUsedPin(),64,2,QLatin1Char('0')) << "\"]";

        if (currentTree->_child != NULL)
        {
            out << (quint64) currentTree << "-> " << (quint64) currentTree->_child << " [label=\"child\", color=green]" << endl;
        }
        if (currentTree->_previous != NULL)
        {
            out << (quint64) currentTree << "-> " << (quint64) currentTree->_previous << " [label=\"prev\", color=blue]" << endl;
        }
        if (currentTree->_next != NULL)
        {
            out << (quint64) currentTree << "-> " << (quint64) currentTree->_next << " [label=\"next\", color=red]" << endl;
        }

        if (currentTree->_child != NULL)
        {
            currentTree = currentTree->_child;
        }
        else
        {
            if (currentTree->_next != NULL)
            {
                currentTree = currentTree->_next;
            }
            else
            {
                while (currentTree->_parent != NULL)
                {
                    currentTree = currentTree->_parent;

                    if (currentTree->_next != NULL)
                    {
                        currentTree = currentTree->_next;
                        break;
                    }
                }

                if(currentTree->_parent == NULL)
                {
                    finished = true;
                }

            }

        }
    }

    out << "}" << endl;

    //qDebug() << "system : " << system("dot -Tpng -O tree.dot");
}
