#include "stdafx.h"
#include "converter.h"

// ==================== CONSTRUCTOR ====================
Converter::Converter() {}

Converter::~Converter() {}

// ==================== M�TODO PRINCIPAL ====================

QString Converter::convert(const QString& inputText)
{
    // 1. Procesar el texto natural en instrucciones
    std::vector<Instruction> instructions = processor.processText(inputText);

    // 2. Generar el c�digo C++ a partir de esas instrucciones
    QString generatedCode = generator.generateCode(instructions);

    return generatedCode;
}
