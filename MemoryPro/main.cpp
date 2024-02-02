/* Memory game with graphical user interface
*
* Creator:
* miqqou @ GitHub
*
* */

#include "mainwindow.hh"
#include <QApplication>
#include <QtWidgets>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setFixedSize(1400, 1000);
    w.show();

    // Setting style for all elements.
    QFile styleFile(":/style.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&styleFile);
        w.setStyleSheet(stream.readAll());
        styleFile.close();
    }

    return a.exec();
}
