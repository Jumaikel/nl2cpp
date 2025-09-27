#include "stdafx.h"
#include "main_view.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QIcon>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 🔹 Cargar estilos desde recursos
    QFile styleFile(":/styles/style.qss");
    if (!styleFile.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "No se pudo cargar style.qss";
    }
    else {
        QTextStream stream(&styleFile);
        QString styleSheet = stream.readAll();
        app.setStyleSheet(styleSheet);
        styleFile.close();
    }

    // 🔹 Crear ventana principal
    MainView window;
	window.resize(1280, 720);
    window.setWindowTitle("Natural Language to C++ Converter");
    window.setWindowIcon(QIcon(":/icons/app_icon.png"));
    window.show();

    return app.exec();
}
