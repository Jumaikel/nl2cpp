#include "stdafx.h"
#include "main_view.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

// Constructor
MainView::MainView(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // Conectar botones a sus slots
    connect(ui.btnLoad, &QPushButton::clicked, this, &MainView::onBtnLoadClicked);
    connect(ui.btnClean, &QPushButton::clicked, this, &MainView::onBtnCleanClicked);
    connect(ui.btnConvert, &QPushButton::clicked, this, &MainView::onBtnConvertClicked);
    connect(ui.btnSave, &QPushButton::clicked, this, &MainView::onBtnSaveClicked);
}

// Destructor
MainView::~MainView()
{
}

// ==================== SLOTS ====================

void MainView::onBtnLoadClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Abrir archivo de entrada"),
        "",
        tr("Archivos de texto (*.txt);;Todos los archivos (*.*)")
    );

    if (!filePath.isEmpty()) {
        loadFromFile(filePath);
    }
}

void MainView::onBtnCleanClicked()
{
    ui.txtEdtLoaded->clear();
    ui.txtEdtConverted->clear();
}

void MainView::onBtnConvertClicked()
{
    QString input = ui.txtEdtLoaded->toPlainText();

    if (input.trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Advertencia"), tr("El texto de entrada está vacío."));
        return;
    }

    QString output = convertText(input);
    ui.txtEdtConverted->setPlainText(output);
}

void MainView::onBtnSaveClicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Guardar archivo convertido"),
        "",
        tr("Archivos de texto (*.txt);;Todos los archivos (*.*)")
    );

    if (!filePath.isEmpty()) {
        saveToFile(filePath);
    }
}

// ==================== MÉTODOS AUXILIARES ====================

void MainView::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("No se pudo abrir el archivo."));
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    ui.txtEdtLoaded->setPlainText(content);
    file.close();
}

void MainView::saveToFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("No se pudo guardar el archivo."));
        return;
    }

    QTextStream out(&file);
    out << ui.txtEdtConverted->toPlainText();
    file.close();
}

QString MainView::convertText(const QString& input)
{
    // 🔹 Ahora usamos la clase Converter en lugar del mock
    return converter.convert(input);
}
