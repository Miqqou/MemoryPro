/* Memory game with graphical user interface
*
* Description:
* The program takes inputs from the user about the number of players and cards. In the program
* there are also push buttons that the user can press to give commands to the program.
* The program includes a memory game, more about the rules and instructions
* in the instructions.txt file.
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
