#include "stdafx.h"
#include "natural_language_processor.h"
#include <QStringList>

// ==================== CONSTRUCTOR ====================
NaturalLanguageProcessor::NaturalLanguageProcessor()
{
    initializeDictionaries();
}

NaturalLanguageProcessor::~NaturalLanguageProcessor() {}

// ==================== MÉTODO PRINCIPAL ====================

std::vector<Instruction> NaturalLanguageProcessor::processText(const QString& inputText)
{
    std::vector<Instruction> instructions;

    // Dividir texto por líneas
    QStringList lines = inputText.split("\n", Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        Instruction inst = parseLine(line.trimmed());
        instructions.push_back(inst);
    }

    return instructions;
}

// ==================== PARSE DE UNA LÍNEA ====================

Instruction NaturalLanguageProcessor::parseLine(const QString& line)
{
    Instruction instruction;
    instruction.type = detectInstructionType(line);
    instruction.keyword = line.section(' ', 0, 0);  // primera palabra
    instruction.arguments = line.split(" ", Qt::SkipEmptyParts);

    return instruction;
}

// ==================== DETECCIÓN DE TIPO ====================

InstructionType NaturalLanguageProcessor::detectInstructionType(const QString& line)
{
    QString lowerLine = line.toLower();

    // Aritmética
    for (const auto& pair : arithmeticKeywords) {
        if (lowerLine.contains(pair.first)) {
            return InstructionType::Arithmetic;
        }
    }

    // Variables
    for (const auto& pair : variableKeywords) {
        if (lowerLine.contains(pair.first)) {
            return InstructionType::VariableDeclaration;
        }
    }

    // Control
    for (const auto& pair : controlKeywords) {
        if (lowerLine.contains(pair.first)) {
            return InstructionType::ControlStructure;
        }
    }

    // Entrada / Salida
    for (const auto& pair : ioKeywords) {
        if (lowerLine.contains(pair.first)) {
            return (pair.second == "cin") ? InstructionType::Input : InstructionType::Output;
        }
    }

    return InstructionType::Unknown;
}

// ==================== INICIALIZAR DICCIONARIOS ====================

void NaturalLanguageProcessor::initializeDictionaries()
{
    // Operaciones aritméticas
    arithmeticKeywords = {
        {"sumar", "+"},
        {"restar", "-"},
        {"multiplicar", "*"},
        {"dividir", "/"},
        {"total", "total"},
        {"resultado", "resultado"}
    };

    // Variables y tipos
    variableKeywords = {
        {"crear variable", "var"},
        {"entero", "int"},
        {"número entero", "int"},
        {"número decimal", "float"},
        {"decimal", "float"},
        {"texto", "string"},
        {"palabra", "string"},
        {"cadena", "string"},
        {"carácter", "char"},
        {"booleano", "bool"},
        {"valor inicial", "="},
        {"asignar valor", "="}
    };

    // Estructuras de control
    controlKeywords = {
        {"si", "if"},
        {"sino", "else"},
        {"mientras", "while"},
        {"repetir hasta", "do while"},
        {"para", "for"},
        {"igual a", "=="},
        {"diferente de", "!="},
        {"mayor que", ">"},
        {"menor que", "<"},
        {"y", "&&"},
        {"o", "||"}
    };

    // Entrada / salida
    ioKeywords = {
        {"mostrar", "cout"},
        {"imprimir", "cout"},
        {"mensaje", "cout"},
        {"leer", "cin"},
        {"ingresar valor", "cin"}
    };
}
