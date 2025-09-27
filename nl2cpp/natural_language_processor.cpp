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
    QStringList lines = inputText.split("\n", Qt::SkipEmptyParts);

    for (QString& line : lines) {
        line = line.trimmed();

        // 1. Preservar literales entre comillas
        QString preserved;
        bool insideQuotes = false;
        for (int i = 0; i < line.size(); ++i) {
            QChar c = line[i];
            if (c == '\"') {
                insideQuotes = !insideQuotes;
                preserved.append(c);
            }
            else {
                if (insideQuotes) {
                    // dentro de comillas: no cambiar mayúsculas
                    preserved.append(c);
                }
                else {
                    // fuera de comillas: pasar a minúscula
                    preserved.append(c.toLower());
                }
            }
        }
        line = preserved;

        // 2. Limpieza de conectores fuera de comillas
        if (!insideQuotes) {
            line.replace(" y ", " ");
            line.replace(" con ", " ");
            line.replace(" elementos", "");
            // Ojo: ya no borramos " a " ni " que " aquí,
            // porque forman parte de condiciones y mensajes.
        }

        // 3. Normalizaciones mínimas de acentos
        line.replace("número", "numero");
        line.replace("carácter", "caracter");
    }

    int index = 0;
    return parseBlock(lines, index);
}


// ==================== PARSER DE BLOQUES ====================
// Parsea hasta encontrar alguno de los stopTokens SIN consumirlo.
std::vector<Instruction> NaturalLanguageProcessor::parseUntil(const QStringList& lines, int& index, const QStringList& stopTokens)
{
    std::vector<Instruction> block;

    while (index < lines.size()) {
        QString line = lines[index];
        if (line.isEmpty()) { index++; continue; }

        // ¿Debemos detenernos aquí?
        bool shouldStop = false;
        for (const QString& s : stopTokens) {
            if (line.startsWith(s)) { shouldStop = true; break; }
        }
        if (shouldStop) break;

        // Si aquí empieza otro control, delega a parseBlock para manejar su propio fin
        if (line.startsWith("si") || line.startsWith("sino") ||
            line.startsWith("mientras") || line.startsWith("para") ||
            line.startsWith("repetir hasta")) {
            // parseBlock devolverá uno o varios instructions (ej. if y luego else)
            auto sub = parseBlock(lines, index);
            for (auto& ins : sub) block.push_back(ins);
            continue;
        }

        // Instrucción simple
        block.push_back(parseLine(line));
        index++;
    }
    return block;
}

std::vector<Instruction> NaturalLanguageProcessor::parseBlock(const QStringList& lines, int& index)
{
    std::vector<Instruction> block;

    while (index < lines.size()) {
        QString line = lines[index];

        if (line.isEmpty()) { index++; continue; }

        // Estos cierran niveles superiores
        if (line.startsWith("fin si") || line.startsWith("fin mientras") || line.startsWith("fin para")) {
            index++; // consumir el fin
            break;
        }

        // ---- IF / ELSE ----
        if (line.startsWith("si")) {
            Instruction ifInst = parseLine(line);
            index++; // avanzar tras 'si ...'

            // Cuerpo del IF: hasta 'sino' o 'fin si'
            ifInst.nested = parseUntil(lines, index, { "sino", "fin si" });
            block.push_back(ifInst);

            // ¿Hay 'sino'?
            if (index < lines.size() && lines[index].startsWith("sino")) {
                Instruction elseInst = parseLine(lines[index]);
                index++; // avanzar tras 'sino'
                // Cuerpo del ELSE: hasta 'fin si'
                elseInst.nested = parseUntil(lines, index, { "fin si" });
                block.push_back(elseInst);
            }

            // Consumir 'fin si' si está presente
            if (index < lines.size() && lines[index].startsWith("fin si")) {
                index++;
            }
            continue;
        }

        // ---- WHILE ----
        if (line.startsWith("mientras")) {
            Instruction wInst = parseLine(line);
            index++; // avanzar tras 'mientras ...'
            wInst.nested = parseUntil(lines, index, { "fin mientras" });
            block.push_back(wInst);

            if (index < lines.size() && lines[index].startsWith("fin mientras")) {
                index++; // cerrar while
            }
            continue;
        }

        // ---- FOR ----
        if (line.startsWith("para")) {
            Instruction fInst = parseLine(line);
            index++; // avanzar tras 'para ...'
            fInst.nested = parseUntil(lines, index, { "fin para" });
            block.push_back(fInst);

            if (index < lines.size() && lines[index].startsWith("fin para")) {
                index++; // cerrar for
            }
            continue;
        }

        // ---- DO-WHILE ----
        if (line.startsWith("repetir")) {
            Instruction dInst = parseLine(line);
            index++; // avanzar tras 'repetir'

            // cuerpo hasta 'hasta que'
            dInst.nested = parseUntil(lines, index, { "hasta", "hasta que" });
            block.push_back(dInst);

            if (index < lines.size() && (lines[index].startsWith("hasta") || lines[index].startsWith("hasta que"))) {
                Instruction condInst = parseLine(lines[index]);
                block.push_back(condInst);
                index++;
            }
            continue;
        }

        // ---- FUNCTION DECLARATION ----
        if (line.startsWith("definir funcion")) {
            Instruction funInst = parseLine(line);
            index++;
            funInst.nested = parseUntil(lines, index, { "fin funcion" });
            block.push_back(funInst);

            if (index < lines.size() && lines[index].startsWith("fin funcion")) {
                index++;
            }
            continue;
        }

        // Instrucción simple
        block.push_back(parseLine(line));
        index++;
    }

    return block;
}

// ==================== PARSE DE UNA LÍNEA ====================

Instruction NaturalLanguageProcessor::parseLine(const QString& line)
{
    Instruction instruction;
    instruction.type = detectInstructionType(line);

    // Determinar keyword: usar frases clave si aplican
    if (line.startsWith("definir funcion"))
        instruction.keyword = "definir funcion";
    else if (line.startsWith("llamar funcion"))
        instruction.keyword = "llamar funcion";
    else if (line.startsWith("comenzar programa"))
        instruction.keyword = "comenzar programa";
    else if (line.startsWith("terminar programa"))
        instruction.keyword = "terminar programa";
    else if (line.startsWith("fin funcion"))
        instruction.keyword = "fin funcion";
    else if (line.startsWith("hasta que"))
        instruction.keyword = "hasta que";
    else
        instruction.keyword = line.section(' ', 0, 0);  // primera palabra

    instruction.arguments = line.split(" ", Qt::SkipEmptyParts);
    return instruction;
}

// ==================== DETECCIÓN DE TIPO ====================

InstructionType NaturalLanguageProcessor::detectInstructionType(const QString& line)
{
    const QString lowerLine = line.toLower();

    // Inicio / fin de programa
    if (lowerLine.startsWith("comenzar programa")) return InstructionType::ProgramStart;
    if (lowerLine.startsWith("terminar programa")) return InstructionType::ProgramEnd;

    // Funciones
    if (lowerLine.startsWith("definir funcion")) return InstructionType::FunctionDefinition;
    if (lowerLine.startsWith("llamar funcion")) return InstructionType::FunctionCall;

    // Control
    if (lowerLine.startsWith("si") || lowerLine.startsWith("sino") ||
        lowerLine.startsWith("mientras") || lowerLine.startsWith("para") ||
        lowerLine.startsWith("repetir") || lowerLine.startsWith("hasta")) {
        return InstructionType::ControlStructure;
    }

    // Asignación
    if (lowerLine.startsWith("asignar")) return InstructionType::Assignment;

    // Variables
    if (lowerLine.startsWith("crear variable")) return InstructionType::VariableDeclaration;

    // IO
    for (const auto& pair : ioKeywords) {
        if (lowerLine.startsWith(pair.first)) {
            return (pair.second == "cin") ? InstructionType::Input : InstructionType::Output;
        }
    }

    // Listas / arreglos
    if (lowerLine.contains("lista") || lowerLine.contains("arreglo")) return InstructionType::ArrayCreation;

    // Aritmética
    for (const auto& pair : arithmeticKeywords) {
        if (lowerLine.startsWith(pair.first)) return InstructionType::Arithmetic;
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
        {"numero entero", "int"},
        {"numero decimal", "float"},
        {"decimal", "float"},
        {"texto", "string"},
        {"palabra", "string"},
        {"cadena", "string"},
        {"caracter", "char"},
        {"booleano", "bool"},
        {"valor inicial", "="},
        {"asignar valor", "="}
    };

    // Estructuras de control (sin 'y' ni 'o' para evitar falsos positivos)
    controlKeywords = {
        {"si", "if"},
        {"sino", "else"},
        {"mientras", "while"},
        {"repetir hasta", "do while"},
        {"para", "for"},
        {"igual a", "=="},
        {"diferente de", "!="},
        {"mayor que", ">"},
        {"menor que", "<"}
        // 🔸 Omitimos 'y' y 'o' aquí a propósito
    };

    // Entrada / salida
    ioKeywords = {
        {"mostrar", "cout"},
        {"imprimir", "cout"},
        {"mensaje", "cout"},
        {"leer", "cin"},
        {"ingresar valor", "cin"}
    };

    // Funciones y programa
    controlKeywords.insert({ "definir funcion", "void" });
    controlKeywords.insert({ "llamar funcion", "call" });
    controlKeywords.insert({ "fin funcion", "endfunc" });
    controlKeywords.insert({ "comenzar programa", "main" });
    controlKeywords.insert({ "terminar programa", "endmain" });
    controlKeywords.insert({ "fin", "end" });
    controlKeywords.insert({ "hasta que", "while" });

}
