#pragma once

#include <QString>
#include <vector>
#include "natural_language_processor.h"

class CodeGenerator
{
public:
    CodeGenerator();
    ~CodeGenerator();

    // Genera c�digo C++ a partir de un conjunto de instrucciones
    QString generateCode(const std::vector<Instruction>& instructions);

private:
    // M�todos auxiliares para cada tipo de instrucci�n
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
