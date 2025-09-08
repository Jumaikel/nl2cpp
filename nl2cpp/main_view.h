#pragma once

#include <QtWidgets/QMainWindow>
#include <QString>
#include "ui_main_view.h"
#include "converter.h"

class MainView : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainView(QWidget* parent = nullptr);
    ~MainView();

private slots:
    void onBtnLoadClicked();
    void onBtnCleanClicked();
    void onBtnConvertClicked();
    void onBtnSaveClicked();

private:
    Ui::MainViewClass ui;
    Converter converter;

    // Métodos auxiliares
    void loadFromFile(const QString& filePath);
    void saveToFile(const QString& filePath);
    QString convertText(const QString& input);
};
