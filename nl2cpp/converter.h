#pragma once

#include <QString>
#include "natural_language_processor.h"
#include "code_generator.h"

class Converter
{
public:
    Converter();
    ~Converter();

    // Punto de entrada principal: convierte texto NL -> C++
    QString convert(const QString& inputText);

private:
    NaturalLanguageProcessor processor;
    CodeGenerator generator;
};
