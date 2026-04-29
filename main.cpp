/**
 * @file main.cpp
 * @brief Entry point of the VR CAD Viewer application.
 *
 * Initializes the Qt application and launches the main window.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
