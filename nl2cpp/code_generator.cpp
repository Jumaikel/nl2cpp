#include "stdafx.h"
#include "code_generator.h"
#include <QTextStream>

// ==================== CONSTRUCTOR ====================
CodeGenerator::CodeGenerator() {}
CodeGenerator::~CodeGenerator() {}

// ==================== MÉTODO PRINCIPAL ====================
QString CodeGenerator::generateCode(const std::vector<Instruction>& instructions)
{
    QString code;
    QTextStream out(&code);

    // Encabezado estándar
    out << "#include <iostream>\n";
    out << "using namespace std;\n\n";
    out << "int main() {\n";

    // Generar cada instrucción
    for (const auto& inst : instructions) {
        switch (inst.type) {
        case InstructionType::Arithmetic:
            out << "    " << generateArithmetic(inst) << "\n";
            break;
        case InstructionType::VariableDeclaration:
            out << "    " << generateVariableDeclaration(inst) << "\n";
            break;
        case InstructionType::Assignment:
            out << "    " << generateAssignment(inst) << "\n";
            break;
        case InstructionType::ArrayCreation:
            out << "    " << generateArrayCreation(inst) << "\n";
            break;
        case InstructionType::ControlStructure:
            out << "    " << generateControlStructure(inst) << "\n";
            break;
        case InstructionType::Input:
            out << "    " << generateInput(inst) << "\n";
            break;
        case InstructionType::Output:
            out << "    " << generateOutput(inst) << "\n";
            break;
        case InstructionType::FunctionDefinition:
            out << "    " << generateFunctionDefinition(inst) << "\n";
            break;
        case InstructionType::FunctionCall:
            out << "    " << generateFunctionCall(inst) << "\n";
            break;
        default:
            out << "    // [WARN] Instrucción desconocida: " << inst.keyword << "\n";
            break;
        }
    }

    // Finalizar main
    out << "    return 0;\n";
    out << "}\n";

    return code;
}

// ==================== AUXILIARES ====================

// Aritmética: "sumar 5 y 3" -> int resultado = 5 + 3;
QString CodeGenerator::generateArithmetic(const Instruction& instruction)
{
    if (instruction.arguments.size() >= 3) {
        QString op = "+";
        if (instruction.keyword.contains("restar")) op = "-";
        else if (instruction.keyword.contains("multiplicar")) op = "*";
        else if (instruction.keyword.contains("dividir")) op = "/";

        QString expr = joinArguments(instruction.arguments.mid(1), op);
        return "int resultado = " + expr + ";";
    }
    return "// Error: instrucción aritmética inválida";
}

// Declaración de variable: "crear variable entero x"
QString CodeGenerator::generateVariableDeclaration(const Instruction& instruction)
{
    if (instruction.arguments.size() >= 3) {
        QString type = "int";
        QString varName = instruction.arguments.last();

        if (instruction.arguments.contains("entero")) type = "int";
        else if (instruction.arguments.contains("decimal")) type = "float";
        else if (instruction.arguments.contains("texto")) type = "string";
        else if (instruction.arguments.contains("carácter")) type = "char";
        else if (instruction.arguments.contains("booleano")) type = "bool";

        return type + " " + varName + ";";
    }
    return "// Error: declaración inválida";
}

// Asignación: "asignar valor x = 10"
QString CodeGenerator::generateAssignment(const Instruction& instruction)
{
    if (instruction.arguments.size() >= 3) {
        QString varName = instruction.arguments[2];
        QString value = instruction.arguments.last();
        return varName + " = " + value + ";";
    }
    return "// Error: asignación inválida";
}

// Creación de arreglo: "crear lista de enteros con 5 elementos"
QString CodeGenerator::generateArrayCreation(const Instruction& instruction)
{
    if (instruction.arguments.contains("lista") || instruction.arguments.contains("arreglo")) {
        QString type = "int";
        int size = 0;

        if (instruction.arguments.contains("decimal")) type = "float";
        else if (instruction.arguments.contains("texto")) type = "string";
        else if (instruction.arguments.contains("carácter")) type = "char";

        // buscar número
        for (const auto& arg : instruction.arguments) {
            bool ok = false;
            int n = arg.toInt(&ok);
            if (ok) {
                size = n;
                break;
            }
        }

        return type + " lista[" + QString::number(size) + "];";
    }
    return "// Error: creación de lista inválida";
}

// Control: "si x igual a 5"
QString CodeGenerator::generateControlStructure(const Instruction& instruction)
{
    if (instruction.arguments.contains("si")) {
        return "if (/* condición */) { \n        // TODO \n    }";
    }
    else if (instruction.arguments.contains("sino")) {
        return "else { \n        // TODO \n    }";
    }
    else if (instruction.arguments.contains("mientras")) {
        return "while (/* condición */) { \n        // TODO \n    }";
    }
    else if (instruction.arguments.contains("para")) {
        return "for (int i = 0; i < /* N */; i++) { \n        // TODO \n    }";
    }
    return "// Error: estructura de control inválida";
}

// Entrada: "leer x"
QString CodeGenerator::generateInput(const Instruction& instruction)
{
    if (instruction.arguments.size() >= 2) {
        QString varName = instruction.arguments.last();
        return "cin >> " + varName + ";";
    }
    return "// Error: entrada inválida";
}

// Salida: "mostrar resultado"
QString CodeGenerator::generateOutput(const Instruction& instruction)
{
    if (instruction.arguments.size() >= 2) {
        QString msg = instruction.arguments.mid(1).join(" ");
        return "cout << \"" + msg + "\" << endl;";
    }
    return "// Error: salida inválida";
}

// Función: "definir función"
QString CodeGenerator::generateFunctionDefinition(const Instruction& instruction)
{
    return "// TODO: funcion";
}

// Llamado a función: "llamar función"
QString CodeGenerator::generateFunctionCall(const Instruction& instruction)
{
    return "// TODO: llamada a funcion";
}

// Utilidad para unir argumentos en expresión
QString CodeGenerator::joinArguments(const QStringList& args, const QString& separator)
{
    return args.join(" " + separator + " ");
}
