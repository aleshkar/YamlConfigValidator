#include "SyntaxValidator.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <stdexcept>

#include "YamlParser.hpp"
#include "YamlTextUtils.hpp"

namespace {

bool hasYamlExtension(const std::string& fileName) {
    std::string lowerName = fileName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

    const std::size_t dot = lowerName.find_last_of('.');
    if (dot == std::string::npos) {
        return false;
    }

    const std::string extension = lowerName.substr(dot);
    return extension == ".yaml" || extension == ".yml";
}

bool lineStartsWithListMarker(const std::string& prepared) {
    return prepared == "-" || prepared.rfind("- ", 0) == 0;
}

std::string contentAfterListMarker(const std::string& prepared) {
    if (prepared.rfind("- ", 0) == 0) {
        return trimText(prepared.substr(2));
    }

    return prepared;
}

void validateLineShape(const std::string& prepared, int lineNumber,
                       ValidationReport& report) {
    if (prepared == "-") {
        report.addError(lineNumber, "проверка-синтаксиса",
                        "элемент списка не может быть пустым");
        return;
    }

    std::string content = prepared;
    if (lineStartsWithListMarker(prepared)) {
        content = contentAfterListMarker(prepared);
        if (content.find(':') == std::string::npos) {
            return;
        }
    }

    const std::size_t separator = content.find(':');
    if (separator == std::string::npos) {
        report.addError(lineNumber, "проверка-синтаксиса", "строка должна содержать ':'");
        return;
    }

    const std::string key = trimText(content.substr(0, separator));
    if (key.empty()) {
        report.addError(lineNumber, "проверка-синтаксиса", "ключ не может быть пустым");
    }
}

void validateEmptyBlocks(const YamlNode& node, ValidationReport& report) {
    for (const YamlNode& child : node.children) {
        if (child.value.empty() && child.children.empty()) {
            report.addError(
                child.lineNumber, "проверка-структуры",
                "ключ '" + child.key + "' не имеет значения и вложенных элементов");
        }

        validateEmptyBlocks(child, report);
    }
}

void validateDuplicateKeys(const YamlNode& node, ValidationReport& report) {
    std::set<std::string> keys;

    for (const YamlNode& child : node.children) {
        if (!child.listItem && !keys.insert(child.key).second) {
            report.addError(child.lineNumber, "проверка-структуры",
                            "дублирующийся ключ '" + child.key + "' в одном блоке");
        }
    }

    for (const YamlNode& child : node.children) {
        validateDuplicateKeys(child, report);
    }
}

}  // namespace

SyntaxValidator::SyntaxValidator(int spacesPerLevel) : spacesPerLevel(spacesPerLevel) {
    if (spacesPerLevel <= 0) {
        throw std::runtime_error("Размер отступа должен быть положительным");
    }
}

ValidationReport SyntaxValidator::validateFile(const std::string& filePath) const {
    ValidationReport report;
    const std::string fileName = fileNameFromPath(filePath);

    if (!hasYamlExtension(fileName)) {
        report.addError(0, "проверка-файла",
                        "расширение файла должно быть .yaml или .yml");
    }

    std::ifstream input(filePath);
    if (!input.is_open()) {
        report.addError(0, "проверка-файла", "файл не удалось открыть");
        return report;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        lines.push_back(line);
    }

    ValidationReport contentReport = validateLines(lines, fileName);
    for (const ValidationIssue& issue : contentReport.issues()) {
        report.addIssue(issue.level, issue.lineNumber, issue.checkName, issue.message);
    }

    return report;
}

ValidationReport SyntaxValidator::validateLines(const std::vector<std::string>& lines,
                                                const std::string& fileName) const {
    ValidationReport report;

    if (!hasYamlExtension(fileName)) {
        report.addError(0, "проверка-файла",
                        "расширение файла должно быть .yaml или .yml");
    }

    int previousLevel = 0;
    bool hasPreviousContent = false;

    for (std::size_t i = 0; i < lines.size(); ++i) {
        const int lineNumber = static_cast<int>(i + 1);
        std::string line = lines[i];

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        int indent = 0;
        try {
            indent = countYamlIndent(line, lineNumber);
        } catch (const std::exception& error) {
            report.addError(lineNumber, "проверка-синтаксиса", error.what());
            continue;
        }

        const std::string withoutComment = removeYamlComment(line);
        const std::string prepared = trimText(withoutComment);

        if (prepared.empty()) {
            continue;
        }

        if (indent % spacesPerLevel != 0) {
            report.addError(
                lineNumber, "проверка-синтаксиса",
                "отступ должен быть кратен " + std::to_string(spacesPerLevel));
        }

        const int level = indent / spacesPerLevel;
        if (hasPreviousContent && level > previousLevel + 1) {
            report.addError(lineNumber, "проверка-структуры",
                            "слишком резкий скачок вложенности");
        }

        validateLineShape(prepared, lineNumber, report);

        previousLevel = level;
        hasPreviousContent = true;
    }

    if (!hasPreviousContent) {
        report.addError(0, "проверка-структуры",
                        "YAML-файл не содержит данных конфигурации");
    }

    if (report.hasErrors()) {
        return report;
    }

    try {
        const YamlParser parser(spacesPerLevel);
        const YamlNode root = parser.parseLines(lines, fileName);
        validateEmptyBlocks(root, report);
        validateDuplicateKeys(root, report);
    } catch (const std::exception& error) {
        report.addError(0, "проверка-структуры", error.what());
    }

    return report;
}
