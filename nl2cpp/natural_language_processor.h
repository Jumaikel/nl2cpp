#pragma once

#include <QString>
#include <QStringList>
#include <vector>
#include <map>

// Enum que representa tipos de instrucciones reconocidas
enum class InstructionType {
    Arithmetic,
    VariableDeclaration,
    Assignment,
    ArrayCreation,
    ControlStructure,
    Input,
    Output,
    FunctionDefinition,
    FunctionCall,
    Unknown
};

// Estructura para representar una instrucción procesada
struct Instruction {
    InstructionType type;
    QString keyword;
    QStringList arguments;
};

class NaturalLanguageProcessor
{
public:
    NaturalLanguageProcessor();
    ~NaturalLanguageProcessor();

    // Procesa texto de entrada y devuelve lista de instrucciones
    std::vector<Instruction> processText(const QString& inputText);

private:
    // Métodos auxiliares
    Instruction parseLine(const QString& line);
    InstructionType detectInstructionType(const QString& line);

    // Diccionarios de palabras clave
    std::map<QString, QString> arithmeticKeywords;
    std::map<QString, QString> variableKeywords;
    std::map<QString, QString> controlKeywords;
    std::map<QString, QString> ioKeywords;

    void initializeDictionaries();
};
