#pragma once

#include <QString>
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
    // Métodos auxiliares para cada tipo de instrucción
    QString generateArithmetic(const Instruction& instruction);
    QString generateVariableDeclaration(const Instruction& instruction);
    QString generateAssignment(const Instruction& instruction);
    QString generateArrayCreation(const Instruction& instruction);
    QString generateControlStructure(const Instruction& instruction);
    QString generateInput(const Instruction& instruction);
    QString generateOutput(const Instruction& instruction);
    QString generateFunctionDefinition(const Instruction& instruction);
    QString generateFunctionCall(const Instruction& instruction);

    // Utilidad para concatenar argumentos en una sola cadena
    QString joinArguments(const QStringList& args, const QString& separator);
};
