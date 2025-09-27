#pragma once

#include <QString>
#include <QSet>
#include <QMap>
#include <vector>
#include "natural_language_processor.h"

class CodeGenerator
{
public:
    CodeGenerator();
    ~CodeGenerator();

    // Genera código C++ a partir de un conjunto de instrucciones
    QString generateCode(const std::vector<Instruction>& instructions);

private:
    // ===== Estado de generación (se reinicia en cada generateCode) =====
    bool needsStringHeader = false;
    bool needsResultado = false;
    bool insideMain = false;

    // Símbolos declarados en el programa (nombre -> tipo C++)
    QMap<QString, QString> symbols;

    // Prototipos de funciones: nombre -> lista tipos de parámetros
    QMap<QString, QStringList> functionParamTypes;
    // Parámetros por nombre de función
    QMap<QString, QStringList> functionParamNames;

    // Memoria del último arreglo para soportar "recorrer la lista ..."
    QString lastArrayName = "lista";
    int     lastArraySize = 0;

    // ===== Utilidades =====
    void resetState();

    // Pre-scan para recopilar variables y decidir includes
    void collectSymbols(const std::vector<Instruction>& instructions);
    void collectSymbolsFromBlock(const std::vector<Instruction>& nested);

    // Generación de bloques/anidados
    QString generateNestedCode(const std::vector<Instruction>& nested, int indentLevel);

    // Métodos auxiliares para cada tipo de instrucción
    QString generateArithmetic(const Instruction& instruction, int indentLevel = 0);
    QString generateVariableDeclaration(const Instruction& instruction, int indentLevel = 0);
    QString generateAssignment(const Instruction& instruction, int indentLevel = 0);
    QString generateArrayCreation(const Instruction& instruction, int indentLevel = 0);
    QString generateControlStructure(const Instruction& instruction, int indentLevel = 0);
    QString buildCondition(const Instruction& instruction);

    QString generateInput(const Instruction& instruction, int indentLevel = 0);
    QString generateOutput(const Instruction& instruction, int indentLevel = 0);

    QString generateFunctionDefinition(const Instruction& instruction);
    QString generateFunctionCall(const Instruction& instruction, int indentLevel = 0);

    // Detección de identificadores válidos (variables) o números
    static bool isIdentifier(const QString& tok);
    static bool isNumber(const QString& tok);

    // Utilidad para concatenar argumentos en una sola cadena
    QString joinArguments(const QStringList& args, const QString& separator);
};
