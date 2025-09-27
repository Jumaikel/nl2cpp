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
    Unknown,
    FunctionDeclaration,
    ProgramStart,
    ProgramEnd
};

// Estructura para representar una instrucción procesada
struct Instruction {
    InstructionType type;
    QString keyword;
    QStringList arguments;
    std::vector<Instruction> nested;
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

    std::vector<Instruction> parseUntil(const QStringList& lines, int& index, const QStringList& stopTokens);

    std::vector<Instruction> parseBlock(const QStringList& lines, int& index);

    // Diccionarios de palabras clave
    std::map<QString, QString> arithmeticKeywords;
    std::map<QString, QString> variableKeywords;
    std::map<QString, QString> controlKeywords;
    std::map<QString, QString> ioKeywords;

    void initializeDictionaries();
};
