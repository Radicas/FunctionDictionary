/**
 * @file FunctionParser.cpp
 * @brief 函数解析器实现
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#include "functionparser.h"
#include "common/logger/logger.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpressionMatchIterator>
#include <QList>
#include <QPair>

FunctionParser& FunctionParser::instance() {
    static FunctionParser instance;
    return instance;
}

FunctionParser::FunctionParser(QObject* parent) : QObject(parent) {
    initializeParsers();
    
    m_languageExtensions["cpp"] = {"cpp", "h", "hpp", "cc", "cxx"};
    m_languageExtensions["python"] = {"py"};
    m_languageExtensions["java"] = {"java"};
    m_languageExtensions["javascript"] = {"js", "ts"};
    
    Logger::instance().info("FunctionParser 初始化完成");
}

FunctionParser::~FunctionParser() {
}

void FunctionParser::initializeParsers() {
    m_parsers["cpp"] = std::bind(&FunctionParser::parseCpp, this, std::placeholders::_1);
    m_parsers["python"] = std::bind(&FunctionParser::parsePython, this, std::placeholders::_1);
    m_parsers["java"] = std::bind(&FunctionParser::parseJava, this, std::placeholders::_1);
    m_parsers["javascript"] = std::bind(&FunctionParser::parseJavaScript, this, std::placeholders::_1);
}

QString FunctionParser::detectLanguage(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    for (auto it = m_languageExtensions.begin(); it != m_languageExtensions.end(); ++it) {
        if (it.value().contains(suffix)) {
            return it.key();
        }
    }
    
    return "generic";
}

bool FunctionParser::isLanguageSupported(const QString& language) const {
    return m_parsers.contains(language);
}

ExtractionResult FunctionParser::extractFunctions(const QString& filePath) {
    ExtractionResult result;
    result.filePath = filePath;
    
    Logger::instance().info("开始解析文件: " + filePath);
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.success = false;
        result.errorMessage = "无法打开文件: " + file.errorString();
        Logger::instance().error(result.errorMessage);
        return result;
    }
    
    QTextStream in(&file);
    QString code = in.readAll();
    file.close();
    
    result.language = detectLanguage(filePath);
    result.totalLines = countLines(code);
    
    if (!isLanguageSupported(result.language)) {
        result = parseGeneric(code, result.language);
    } else {
        auto parser = m_parsers.value(result.language);
        result = parser(code);
    }
    
    result.filePath = filePath;
    result.success = !result.functions.isEmpty();
    
    if (result.success) {
        Logger::instance().info(QString("文件解析成功，提取到 %1 个函数").arg(result.functions.size()));
    } else {
        Logger::instance().warning("文件解析未找到函数");
    }
    
    return result;
}

ExtractionResult FunctionParser::extractFromCode(const QString& code, const QString& language) {
    ExtractionResult result;
    result.language = language;
    result.totalLines = countLines(code);
    
    if (!isLanguageSupported(language)) {
        result = parseGeneric(code, language);
    } else {
        auto parser = m_parsers.value(language);
        result = parser(code);
    }
    
    result.success = !result.functions.isEmpty();
    return result;
}

ExtractionResult FunctionParser::parseCpp(const QString& code) {
    ExtractionResult result;
    result.language = "cpp";
    result.success = false;
    
    Logger::instance().info("使用 C++ 解析器");
    
    QRegularExpression funcPattern(
        R"((?:^|\n)\s*(?:template\s*<[^>]*>\s*)?(?:static\s+|virtual\s+|inline\s+|explicit\s+|friend\s+)*([\w:<>,]+(?:\s*[*&]+)?)\s+(\w+)\s*\(([^)]*)\)\s*(?:const\s*)?(?:override\s*)?(?:final\s*)?(?:noexcept\s*)?(?:->\s*[\w:<>,*&]+\s*)?\{)"
    );
    
    QRegularExpressionMatchIterator it = funcPattern.globalMatch(code);
    int matchCount = 0;
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        ExtractedFunction func;
        
        func.returnType = match.captured(1).trimmed();
        func.name = match.captured(2).trimmed();
        QString paramsStr = match.captured(3);
        
        func.signature = func.returnType + " " + func.name + "(" + paramsStr + ")";
        func.parameters = parseParameters(paramsStr);
        
        int bodyStart = match.capturedEnd();
        func.body = extractFunctionBody(code, bodyStart);
        func.startLine = getLineNumber(code, match.capturedStart()) + 1;
        func.endLine = func.startLine + func.body.count('\n');
        func.language = "cpp";
        
        result.functions.append(func);
        matchCount++;
        
        emit extractionProgress(matchCount, 0, QString("已找到函数: %1").arg(func.name));
    }
    
    result.success = !result.functions.isEmpty();
    return result;
}

ExtractionResult FunctionParser::parsePython(const QString& code) {
    ExtractionResult result;
    result.language = "python";
    result.success = false;
    
    Logger::instance().info("使用 Python 解析器");
    
    QRegularExpression funcPattern(
        R"((?:^|\n)\s*def\s+(\w+)\s*\(([^)]*)\)\s*(?:->\s*([\w\[\],\s]+))?\s*:)"
    );
    
    QRegularExpressionMatchIterator it = funcPattern.globalMatch(code);
    int matchCount = 0;
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        ExtractedFunction func;
        
        func.name = match.captured(1).trimmed();
        QString paramsStr = match.captured(2);
        func.returnType = match.captured(3).trimmed();
        if (func.returnType.isEmpty()) {
            func.returnType = "None";
        }
        
        func.signature = "def " + func.name + "(" + paramsStr + ")";
        func.parameters = parseParameters(paramsStr);
        
        int bodyStart = match.capturedEnd();
        func.body = extractFunctionBody(code, bodyStart);
        func.startLine = getLineNumber(code, match.capturedStart()) + 1;
        func.endLine = func.startLine + func.body.count('\n');
        func.language = "python";
        
        result.functions.append(func);
        matchCount++;
        
        emit extractionProgress(matchCount, 0, QString("已找到函数: %1").arg(func.name));
    }
    
    result.success = !result.functions.isEmpty();
    return result;
}

ExtractionResult FunctionParser::parseJava(const QString& code) {
    ExtractionResult result;
    result.language = "java";
    result.success = false;
    
    Logger::instance().info("使用 Java 解析器");
    
    QRegularExpression funcPattern(
        R"((?:^|\n)\s*(?:public\s+|protected\s+|private\s+|static\s+|final\s+|abstract\s+|synchronized\s+|native\s+|strictfp\s+)*([\w<>,.?\[\]]+(?:\s*\[*\]*))\s+(\w+)\s*\(([^)]*)\)\s*(?:throws\s+[\w\s,<>.]+)?\s*\{)"
    );
    
    QRegularExpressionMatchIterator it = funcPattern.globalMatch(code);
    int matchCount = 0;
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        ExtractedFunction func;
        
        func.returnType = match.captured(1).trimmed();
        func.name = match.captured(2).trimmed();
        QString paramsStr = match.captured(3);
        
        func.signature = func.returnType + " " + func.name + "(" + paramsStr + ")";
        func.parameters = parseParameters(paramsStr);
        
        int bodyStart = match.capturedEnd();
        func.body = extractFunctionBody(code, bodyStart);
        func.startLine = getLineNumber(code, match.capturedStart()) + 1;
        func.endLine = func.startLine + func.body.count('\n');
        func.language = "java";
        
        result.functions.append(func);
        matchCount++;
        
        emit extractionProgress(matchCount, 0, QString("已找到函数: %1").arg(func.name));
    }
    
    result.success = !result.functions.isEmpty();
    return result;
}

ExtractionResult FunctionParser::parseJavaScript(const QString& code) {
    ExtractionResult result;
    result.language = "javascript";
    result.success = false;
    
    Logger::instance().info("使用 JavaScript/TypeScript 解析器");
    
    QList<QPair<QRegularExpression, std::function<ExtractedFunction(const QRegularExpressionMatch&)>>> patterns;
    
    QRegularExpression funcDeclPattern(
        R"((?:^|\n)\s*(?:async\s+)?function\s+(\w+)\s*\(([^)]*)\)\s*\{)"
    );
    patterns.append({funcDeclPattern, [](const QRegularExpressionMatch& match) {
        ExtractedFunction func;
        func.name = match.captured(1).trimmed();
        func.returnType = "any";
        func.signature = "function " + func.name + "(" + match.captured(2) + ")";
        return func;
    }});
    
    QRegularExpression funcExprPattern(
        R"((?:^|\n)\s*(\w+)\s*[:=]\s*(?:async\s+)?function\s*\(([^)]*)\)\s*\{)"
    );
    patterns.append({funcExprPattern, [](const QRegularExpressionMatch& match) {
        ExtractedFunction func;
        func.name = match.captured(1).trimmed();
        func.returnType = "any";
        func.signature = "function " + func.name + "(" + match.captured(2) + ")";
        return func;
    }});
    
    QRegularExpression arrowFuncPattern(
        R"((?:^|\n)\s*(\w+)\s*=\s*(?:async\s+)?\(([^)]*)\)\s*=>\s*\{)"
    );
    patterns.append({arrowFuncPattern, [](const QRegularExpressionMatch& match) {
        ExtractedFunction func;
        func.name = match.captured(1).trimmed();
        func.returnType = "any";
        func.signature = "const " + func.name + " = (" + match.captured(2) + ") => {}";
        return func;
    }});
    
    int matchCount = 0;
    for (const auto& patternPair : patterns) {
        QRegularExpressionMatchIterator it = patternPair.first.globalMatch(code);
        
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            ExtractedFunction func = patternPair.second(match);
            
            QString paramsStr = match.captured(2);
            func.parameters = parseParameters(paramsStr);
            
            int bodyStart = match.capturedEnd();
            func.body = extractFunctionBody(code, bodyStart);
            func.startLine = getLineNumber(code, match.capturedStart()) + 1;
            func.endLine = func.startLine + func.body.count('\n');
            func.language = "javascript";
            
            result.functions.append(func);
            matchCount++;
            
            emit extractionProgress(matchCount, 0, QString("已找到函数: %1").arg(func.name));
        }
    }
    
    result.success = !result.functions.isEmpty();
    return result;
}

ExtractionResult FunctionParser::parseGeneric(const QString& code, const QString& language) {
    ExtractionResult result;
    result.language = language;
    result.success = false;
    result.errorMessage = "不支持的语言类型: " + language;
    Logger::instance().warning(result.errorMessage);
    return result;
}

QString FunctionParser::extractFunctionBody(const QString& code, int startPos) const {
    if (startPos >= code.length()) {
        return "";
    }
    
    int braceBalance = 0;
    int i = startPos;
    bool inFunction = false;
    
    while (i < code.length()) {
        QChar c = code[i];
        
        if (c == '{') {
            braceBalance++;
            inFunction = true;
        } else if (c == '}') {
            braceBalance--;
            if (braceBalance == 0 && inFunction) {
                break;
            }
        }
        i++;
    }
    
    return code.mid(startPos, i - startPos + 1);
}

QVector<ParameterInfo> FunctionParser::parseParameters(const QString& paramsStr) const {
    QVector<ParameterInfo> params;
    
    if (paramsStr.trimmed().isEmpty()) {
        return params;
    }
    
    QStringList paramList;
    int braceBalance = 0;
    int angleBalance = 0;
    int squareBalance = 0;
    int start = 0;
    
    for (int i = 0; i < paramsStr.length(); i++) {
        QChar c = paramsStr[i];
        
        if (c == '(') braceBalance++;
        else if (c == ')') braceBalance--;
        else if (c == '<') angleBalance++;
        else if (c == '>') angleBalance--;
        else if (c == '[') squareBalance++;
        else if (c == ']') squareBalance--;
        else if (c == ',' && braceBalance == 0 && angleBalance == 0 && squareBalance == 0) {
            paramList.append(paramsStr.mid(start, i - start).trimmed());
            start = i + 1;
        }
    }
    
    if (start < paramsStr.length()) {
        paramList.append(paramsStr.mid(start).trimmed());
    }
    
    for (const QString& param : paramList) {
        if (param.isEmpty()) continue;
        
        ParameterInfo info;
        
        QRegularExpression paramPattern(R"((?:[\w:<>,.*&]+\s+)?([\w]+)(?:\s*=\s*[^,]+)?)");
        QRegularExpressionMatch match = paramPattern.match(param);
        
        if (match.hasMatch()) {
            info.name = match.captured(1);
            
            int namePos = param.indexOf(info.name);
            if (namePos > 0) {
                info.type = param.left(namePos).trimmed();
            }
            
            int equalsPos = param.indexOf('=');
            if (equalsPos > 0) {
                info.defaultValue = param.mid(equalsPos + 1).trimmed();
            }
        } else {
            info.name = param;
        }
        
        params.append(info);
    }
    
    return params;
}

int FunctionParser::countLines(const QString& code) const {
    return code.count('\n') + 1;
}

int FunctionParser::getLineNumber(const QString& code, int pos) const {
    if (pos <= 0) return 0;
    return code.left(pos).count('\n');
}

