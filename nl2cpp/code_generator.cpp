#include "stdafx.h"
#include "code_generator.h"
#include <QTextStream>
#include <QRegularExpression>

// ==================== CONSTRUCTOR ====================
CodeGenerator::CodeGenerator() {}
CodeGenerator::~CodeGenerator() {}

void CodeGenerator::resetState() {
    needsStringHeader = false;
    needsResultado = false;
    insideMain = false;
    symbols.clear();
    functionParamTypes.clear();
    functionParamNames.clear();
    lastArrayName = "lista";
    lastArraySize = 0;
}

// ==================== MÉTODO PRINCIPAL ====================
QString CodeGenerator::generateCode(const std::vector<Instruction>& instructions)
{
    resetState();

    // 1) Colectar símbolos declarados (para firmas de funciones y llamadas)
    collectSymbols(instructions);

    QString code;
    QTextStream out(&code);

    // 2) Includes básicos
    out << "#include <iostream>\n";
    if (needsStringHeader) out << "#include <string>\n";
    out << "using namespace std;\n\n";

    // 3) Definiciones de funciones ANTES de main
    for (const auto& inst : instructions) {
        if (inst.type == InstructionType::FunctionDefinition) {
            out << generateFunctionDefinition(inst) << "\n";
        }
    }

    // 4) main()
    out << "int main() {\n";
    insideMain = true;

    // 5) 'resultado' si habrá aritmética
    for (const auto& inst : instructions) {
        if (inst.type == InstructionType::Arithmetic) { needsResultado = true; break; }
        for (const auto& nin : inst.nested)
            if (nin.type == InstructionType::Arithmetic) { needsResultado = true; break; }
        if (needsResultado) break;
    }
    if (needsResultado) {
        out << "    int resultado;\n";
    }

    // 6) Cuerpo (ignorando ProgramStart/End y FunctionDefinition)
    for (const auto& inst : instructions) {
        if (inst.type == InstructionType::FunctionDefinition ||
            inst.type == InstructionType::ProgramStart ||
            inst.type == InstructionType::ProgramEnd) {
            continue;
        }

        QString line;
        switch (inst.type) {
        case InstructionType::Arithmetic:
            line = generateArithmetic(inst, 1);
            break;
        case InstructionType::VariableDeclaration:
            line = generateVariableDeclaration(inst, 1);
            break;
        case InstructionType::Assignment:
            line = generateAssignment(inst, 1);
            break;
        case InstructionType::ArrayCreation:
            line = generateArrayCreation(inst, 1);
            break;
        case InstructionType::ControlStructure:
            line = generateControlStructure(inst, 1);
            break;
        case InstructionType::Input:
            line = generateInput(inst, 1);
            break;
        case InstructionType::Output:
            line = generateOutput(inst, 1);
            break;
        case InstructionType::FunctionCall:
            line = generateFunctionCall(inst, 1);
            break;
        default:
            line = "    // [WARN] Unknown instruction: " + inst.keyword + "\n";
            break;
        }

        out << line;
        if (!line.endsWith('\n')) out << "\n";
    }

    // 7) Cerrar main
    out << "    return 0;\n";
    out << "}\n";

    return code;
}

// ==================== PRE-SCAN SÍMBOLOS ====================
void CodeGenerator::collectSymbols(const std::vector<Instruction>& instructions)
{
    auto collectOne = [&](const Instruction& ins) {
        if (ins.type == InstructionType::VariableDeclaration) {
            QString type = "int";
            if (ins.arguments.contains("decimal")) { type = "float"; }
            else if (ins.arguments.contains("texto") || ins.arguments.contains("string")
                || ins.arguments.contains("palabra") || ins.arguments.contains("cadena")) {
                type = "string"; needsStringHeader = true;
            }
            else if (ins.arguments.contains("caracter")) { type = "char"; }
            else if (ins.arguments.contains("booleano")) { type = "bool"; }

            if (!ins.arguments.isEmpty()) {
                QString name = ins.arguments.last();
                symbols[name] = type;
            }
        }
        if (!ins.nested.empty()) {
            collectSymbolsFromBlock(ins.nested);
        }
        };

    for (const auto& ins : instructions) collectOne(ins);
}

void CodeGenerator::collectSymbolsFromBlock(const std::vector<Instruction>& nested)
{
    for (const auto& ins : nested) {
        if (ins.type == InstructionType::VariableDeclaration) {
            QString type = "int";
            if (ins.arguments.contains("decimal")) { type = "float"; }
            else if (ins.arguments.contains("texto") || ins.arguments.contains("string")
                || ins.arguments.contains("palabra") || ins.arguments.contains("cadena")) {
                type = "string"; needsStringHeader = true;
            }
            else if (ins.arguments.contains("caracter")) { type = "char"; }
            else if (ins.arguments.contains("booleano")) { type = "bool"; }

            if (!ins.arguments.isEmpty()) {
                QString name = ins.arguments.last();
                symbols[name] = type;
            }
        }
        if (!ins.nested.empty()) collectSymbolsFromBlock(ins.nested);
    }
}

// ==================== AUXILIARES ====================

// Aritmética: admite variables y números. Reutiliza 'resultado'.
QString CodeGenerator::generateArithmetic(const Instruction& instruction, int indentLevel)
{
    QString op = "+";
    if (instruction.keyword.contains("restar")) op = "-";
    else if (instruction.keyword.contains("multiplicar")) op = "*";
    else if (instruction.keyword.contains("dividir")) op = "/";

    QStringList tokens;
    for (int i = 1; i < instruction.arguments.size(); ++i) {
        const QString tok = instruction.arguments[i];
        if (tok == "y" || tok == "e" || tok == "con") continue;
        tokens << tok;
    }

    QStringList terms;
    for (const auto& t : tokens) {
        if (isIdentifier(t) || isNumber(t)) terms << t;
    }

    QString indent(indentLevel * 4, ' ');
    if (terms.size() >= 2) {
        QString expr;
        for (int i = 0; i < terms.size(); ++i) {
            if (i) expr += " " + op + " ";
            expr += terms[i];
        }
        return indent + "resultado = " + expr + ";";
    }
    return indent + "// Error: invalid arithmetic instruction";
}

// Declaración de variable: "crear variable entero x"
QString CodeGenerator::generateVariableDeclaration(const Instruction& instruction, int indentLevel)
{
    QString type = "int";
    QString varName = "var";

    if (instruction.arguments.contains("entero") || instruction.arguments.contains("enteros")) type = "int";
    else if (instruction.arguments.contains("decimal")) type = "float";
    else if (instruction.arguments.contains("texto") || instruction.arguments.contains("string")
        || instruction.arguments.contains("palabra") || instruction.arguments.contains("cadena")) {
        type = "string"; needsStringHeader = true;
    }
    else if (instruction.arguments.contains("caracter")) type = "char";
    else if (instruction.arguments.contains("booleano")) type = "bool";

    if (!instruction.arguments.isEmpty()) varName = instruction.arguments.last();

    symbols[varName] = type;
    QString indent(indentLevel * 4, ' ');
    return indent + type + " " + varName + ";";
}

// Asignación robusta (soporta "asignar valor x = 10" y "asignar x 10")
QString CodeGenerator::generateAssignment(const Instruction& instruction, int indentLevel)
{
    QString indent(indentLevel * 4, ' ');

    // Encuentra primer identificador tras "asignar" (saltando "valor")
    int firstId = -1;
    for (int i = 0; i < instruction.arguments.size(); ++i) {
        const QString& t = instruction.arguments[i];
        if (t == "asignar" || t == "valor") continue;
        if (isIdentifier(t)) { firstId = i; break; }
    }
    // Busca '='
    int eqIndex = instruction.arguments.indexOf("=");

    if (firstId != -1 && eqIndex != -1 && eqIndex + 1 < instruction.arguments.size()) {
        QString varName = instruction.arguments[firstId];
        QString value = instruction.arguments.mid(eqIndex + 1).join(" ");
        return indent + varName + " = " + value + ";";
    }

    // Sin '=': tomar todo lo que sigue al identificador como valor
    if (firstId != -1 && firstId + 1 < instruction.arguments.size()) {
        QString varName = instruction.arguments[firstId];
        QString value = instruction.arguments.mid(firstId + 1).join(" ");
        return indent + varName + " = " + value + ";";
    }

    return indent + "// Error: invalid assignment";
}

// Creación de arreglo o "recorrer la lista ..."
QString CodeGenerator::generateArrayCreation(const Instruction& instruction, int indentLevel)
{
    QString indent(indentLevel * 4, ' ');

    // Si viene mal tipado desde NLP para "recorrer la lista ..."
    if (instruction.arguments.contains("recorrer")) {
        int n = (lastArraySize > 0 ? lastArraySize : 5);
        QString arr = lastArrayName;

        // Buscar literal entre comillas para el mensaje
        QString msg;
        bool inQuotes = false;
        for (const auto& tok : instruction.arguments) {
            if (tok.startsWith("\"")) { inQuotes = true; msg += tok; }
            else if (inQuotes) { msg += " " + tok; }
            if (tok.endsWith("\"")) { inQuotes = false; break; }
        }
        if (msg.isEmpty()) msg = "\"Elemento:\"";

        QString code;
        code += indent + "for (int i = 0; i < " + QString::number(n) + "; i++) {\n";
        code += indent + "    cout << " + msg + " << \" \" << " + arr + "[i] << endl;\n";
        code += indent + "}";
        return code;
    }

    // Caso normal: "crear lista de enteros con 5 elementos"
    QString type = "int";
    int size = 0;

    if (instruction.arguments.contains("decimal") || instruction.arguments.contains("decimales")) type = "float";
    else if (instruction.arguments.contains("texto") || instruction.arguments.contains("string")
        || instruction.arguments.contains("palabra") || instruction.arguments.contains("cadena")) {
        type = "string"; needsStringHeader = true;
    }
    else if (instruction.arguments.contains("caracter") || instruction.arguments.contains("caracteres")) type = "char";

    for (const auto& arg : instruction.arguments) {
        bool ok = false;
        int n = arg.toInt(&ok);
        if (ok) { size = n; break; }
    }

    // Nombre por defecto 'lista'; no usar palabras de tipo como nombre
    QString name = "lista";
    const QSet<QString> banned = { "crear","lista","arreglo","de","con","elementos",
                                  "entero","enteros","decimal","decimales","texto",
                                  "string","palabra","cadena","caracter","caracteres","booleano","bool" };
    for (const auto& a : instruction.arguments) {
        if (isIdentifier(a) && !banned.contains(a)) { name = a; break; }
    }

    if (size > 0) {
        lastArrayName = name;
        lastArraySize = size;
        return indent + type + " " + name + "[" + QString::number(size) + "];";
    }

    return indent + "// Error: invalid array creation";
}

// Control: if, else, while, for, repetir/hasta que
QString CodeGenerator::generateControlStructure(const Instruction& instruction, int indentLevel)
{
    QString indent(indentLevel * 4, ' ');
    QString code;

    // ---- IF ----
    if (instruction.arguments.contains("si")) {
        QString cond = buildCondition(instruction);
        code += indent + "if (" + cond + ") {\n";
        code += generateNestedCode(instruction.nested, indentLevel + 1);
        code += indent + "}";
        return code;
    }

    // ---- ELSE ----
    if (instruction.arguments.contains("sino")) {
        code += indent + "else {\n";
        code += generateNestedCode(instruction.nested, indentLevel + 1);
        code += indent + "}";
        return code;
    }

    // ---- WHILE ----
    if (instruction.arguments.contains("mientras")) {
        QString cond = buildCondition(instruction);
        code += indent + "while (" + cond + ") {\n";
        code += generateNestedCode(instruction.nested, indentLevel + 1);
        code += indent + "}";
        return code;
    }

    // ---- FOR ----  "para i desde 0 hasta 4"  -> i <= 4
    if (instruction.arguments.contains("para")) {
        QString var = "i";
        QString start = "0";
        QString end = "0";

        for (int i = 0; i < instruction.arguments.size(); ++i) {
            if (instruction.arguments[i] == "para" && i + 1 < instruction.arguments.size())
                var = instruction.arguments[i + 1];
            if (instruction.arguments[i] == "desde" && i + 1 < instruction.arguments.size())
                start = instruction.arguments[i + 1];
            if (instruction.arguments[i] == "hasta" && i + 1 < instruction.arguments.size())
                end = instruction.arguments[i + 1];
        }

        code += indent + "for (int " + var + " = " + start + "; " + var + " <= " + end + "; " + var + "++) {\n";
        code += generateNestedCode(instruction.nested, indentLevel + 1);
        code += indent + "}";
        return code;
    }

    // ---- DO (repetir) ----
    if (instruction.keyword.startsWith("repetir") || instruction.arguments.contains("repetir")) {
        code += indent + "do {\n";
        code += generateNestedCode(instruction.nested, indentLevel + 1);
        code += indent + "}";
        return code; // el 'while (...)' lo imprime 'hasta que'
    }

    // ---- HASTA QUE ----  -> cierra el do while:    } while (cond);
    if (instruction.keyword.startsWith("hasta") || instruction.arguments.contains("hasta")) {
        QString cond = buildCondition(instruction);
        code = " " + indent + "while (" + cond + ");";
        return code;
    }

    return indent + "// Error: invalid control structure";
}

QString CodeGenerator::buildCondition(const Instruction& instruction)
{
    // Copia de los argumentos
    QStringList args = instruction.arguments;

    // Guardar keyword original (si, mientras, hasta…)
    QString keyword = args.isEmpty() ? "" : args.first();

    // Quita solo la palabra clave de control
    if (!args.isEmpty()) args.removeFirst();
    if (!args.isEmpty() && args.front() == "que") args.removeFirst();

    QString cond;
    for (int i = 0; i < args.size(); ++i) {
        QString token = args[i];

        if (token == "igual" && i + 1 < args.size() && args[i + 1] == "a") {
            cond += "=="; i++;
        }
        else if (token == "diferente" && i + 1 < args.size() && args[i + 1] == "de") {
            cond += "!="; i++;
        }
        else if (token == "mayor" && i + 1 < args.size() && args[i + 1] == "que") {
            cond += ">"; i++;
        }
        else if (token == "menor" && i + 1 < args.size() && args[i + 1] == "que") {
            cond += "<"; i++;
        }
        else if (token == "y") cond += "&&";
        else if (token == "o") cond += "||";
        else cond += token;

        cond += " ";
    }

    cond = cond.trimmed();

    // Caso especial: "hasta que" → negamos la condición
    if (keyword == "hasta") {
        return "!(" + cond + ")";
    }

    return cond;
}



// Entrada: "leer x"
QString CodeGenerator::generateInput(const Instruction& instruction, int indentLevel)
{
    QString indent(indentLevel * 4, ' ');
    if (instruction.arguments.size() >= 2) {
        QString varName = instruction.arguments.last();
        return indent + "cin >> " + varName + ";";
    }
    return indent + "// Error: invalid input";
}

// Salida: "mostrar resultado" o "mostrar \"El resultado es\""
QString CodeGenerator::generateOutput(const Instruction& instruction, int indentLevel)
{
    QString indent(indentLevel * 4, ' ');
    if (instruction.arguments.size() >= 2) {
        // Unimos todo lo que venga después de "mostrar"
        QString arg = instruction.arguments.mid(1).join(" ");

        // Si viene entre comillas, respetarlo
        if (arg.startsWith("\"") && arg.endsWith("\"")) {
            return indent + "cout << " + arg + " << endl;";
        }

        // Si no, lo tratamos como variable/expresión
        return indent + "cout << " + arg + " << endl;";
    }
    return indent + "// Error: invalid output";
}


// ===== Funciones =====
QString CodeGenerator::generateFunctionDefinition(const Instruction& instruction)
{
    // Nombre de la función: segunda palabra tras "definir"
    QString funcName = "funcion";
    for (int i = 0; i < instruction.arguments.size(); ++i) {
        if (instruction.arguments[i] == "funcion" && i + 1 < instruction.arguments.size()) {
            funcName = instruction.arguments[i + 1];
            break;
        }
    }

    // Deducción de parámetros: si el cuerpo usa variables conocidas, las pasamos por valor.
    QSet<QString> used;
    struct ScanUseHelper {
        static void scan(const std::vector<Instruction>& nested, const QMap<QString, QString>& symbols, QSet<QString>& used) {
            for (const auto& ins : nested) {
                for (const auto& t : ins.arguments) {
                    if (symbols.contains(t)) used.insert(t);
                }
                if (!ins.nested.empty()) scan(ins.nested, symbols, used);
            }
        }
    };
    ScanUseHelper::scan(instruction.nested, symbols, used);

    QStringList paramDecls, paramNames;
    for (const auto& name : used) {
        QString type = symbols.value(name, "int");
        if (type == "string") needsStringHeader = true;
        paramDecls << (type + " " + name);
        paramNames << name;
    }
    functionParamTypes[funcName] = [&] {
        QStringList ts;
        for (const auto& n : paramNames) ts << symbols.value(n, "int");
        return ts;
        }();
    functionParamNames[funcName] = paramNames;

    QString sig = "void " + funcName + "(" + paramDecls.join(", ") + ")";
    QString body = "{\n" + generateNestedCode(instruction.nested, 1) + "}\n";
    return sig + " " + body;
}

// Llamado: "llamar funcion nombre"
QString CodeGenerator::generateFunctionCall(const Instruction& instruction, int indentLevel)
{
    QString indent(indentLevel * 4, ' ');
    QString funcName = "funcion";
    for (int i = 0; i < instruction.arguments.size(); ++i) {
        if (instruction.arguments[i] == "funcion" && i + 1 < instruction.arguments.size()) {
            funcName = instruction.arguments[i + 1];
            break;
        }
    }

    QStringList args;
    if (functionParamNames.contains(funcName)) {
        for (const auto& pn : functionParamNames[funcName]) args << pn;
    }
    return indent + funcName + "(" + args.join(", ") + ");";
}

// ===== Utilidades generales =====
bool CodeGenerator::isIdentifier(const QString& tok) {
    if (tok.isEmpty()) return false;
    static QRegularExpression rx("^[A-Za-z_][A-Za-z0-9_]*$");
    return rx.match(tok).hasMatch();
}
bool CodeGenerator::isNumber(const QString& tok) {
    bool ok = false; tok.toDouble(&ok); return ok;
}

QString CodeGenerator::joinArguments(const QStringList& args, const QString& separator)
{
    return args.join(" " + separator + " ");
}

QString CodeGenerator::generateNestedCode(const std::vector<Instruction>& nested, int indentLevel)
{
    QString code;

    for (const auto& inst : nested) {
        switch (inst.type) {
        case InstructionType::Arithmetic:
            code += generateArithmetic(inst, indentLevel) + "\n";
            break;
        case InstructionType::VariableDeclaration:
            code += generateVariableDeclaration(inst, indentLevel) + "\n";
            break;
        case InstructionType::Assignment:
            code += generateAssignment(inst, indentLevel) + "\n";
            break;
        case InstructionType::ArrayCreation:
            code += generateArrayCreation(inst, indentLevel) + "\n";
            break;
        case InstructionType::ControlStructure:
            code += generateControlStructure(inst, indentLevel) + "\n";
            break;
        case InstructionType::Input:
            code += generateInput(inst, indentLevel) + "\n";
            break;
        case InstructionType::Output:
            code += generateOutput(inst, indentLevel) + "\n";
            break;
        case InstructionType::FunctionDefinition:
            // definiciones anidadas: evitamos imprimir aquí
            break;
        case InstructionType::FunctionCall:
            code += generateFunctionCall(inst, indentLevel) + "\n";
            break;
        default:
            code += QString(indentLevel * 4, ' ') + "// [WARN] Unknown nested instruction: " + inst.keyword + "\n";
            break;
        }
    }
    return code;
}
