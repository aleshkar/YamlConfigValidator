#include "YamlParser.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "YamlTextUtils.hpp"

namespace {

YamlNode& getNodeByPath(YamlNode& root, const std::vector<std::size_t>& path) {
    YamlNode* current = &root;
    for (const std::size_t index : path) {
        current = &current->children.at(index);
    }

    return *current;
}

YamlNode makeNode(const std::string& content, int lineNumber, int indent) {
    bool isListItem = false;
    std::string lineBody = trimText(content);

    if (lineBody.rfind("- ", 0) == 0) {
        isListItem = true;
        lineBody = trimText(lineBody.substr(2));
    } else if (lineBody == "-") {
        throw std::runtime_error("Строка " + std::to_string(lineNumber) +
                                 ": пустой элемент списка");
    }

    const std::size_t separator = lineBody.find(':');
    if (separator == std::string::npos) {
        if (isListItem) {
            return YamlNode("-", lineBody, lineNumber, indent, true);
        }

        throw std::runtime_error("Строка " + std::to_string(lineNumber) +
                                 ": ожидалась пара ключ-значение");
    }

    std::string key = trimText(lineBody.substr(0, separator));
    std::string value = trimText(lineBody.substr(separator + 1));

    if (key.empty()) {
        throw std::runtime_error("Строка " + std::to_string(lineNumber) +
                                 ": ключ не может быть пустым");
    }

    return YamlNode(key, value, lineNumber, indent, isListItem);
}

}  // namespace

YamlParser::YamlParser(int spacesPerLevel) : spacesPerLevel(spacesPerLevel) {
    if (spacesPerLevel <= 0) {
        throw std::runtime_error("Размер отступа должен быть положительным");
    }
}

YamlNode YamlParser::parseFile(const std::string& filePath) const {
    std::ifstream input(filePath);
    if (!input.is_open()) {
        throw std::runtime_error("Не удалось открыть файл: " + filePath);
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        lines.push_back(line);
    }

    return parseLines(lines, fileNameFromPath(filePath));
}

YamlNode YamlParser::parseLines(const std::vector<std::string>& lines,
                                const std::string& rootName) const {
    YamlNode root(rootName, "", 0, -1, false);
    std::vector<std::vector<std::size_t>> nodePathByLevel(1);

    for (std::size_t i = 0; i < lines.size(); ++i) {
        const int lineNumber = static_cast<int>(i + 1);
        std::string line = lines[i];

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        const int indent = countYamlIndent(line, lineNumber);
        const std::string lineWithoutComment = removeYamlComment(line);
        const std::string lineBody = trimText(lineWithoutComment);

        if (lineBody.empty()) {
            continue;
        }

        if (indent % spacesPerLevel != 0) {
            throw std::runtime_error("Строка " + std::to_string(lineNumber) +
                                     ": отступ должен быть кратен " +
                                     std::to_string(spacesPerLevel));
        }

        const int level = indent / spacesPerLevel;
        if (level >= static_cast<int>(nodePathByLevel.size())) {
            throw std::runtime_error("Строка " + std::to_string(lineNumber) +
                                     ": слишком резкий скачок вложенности");
        }

        YamlNode node = makeNode(lineBody, lineNumber, indent);
        std::vector<std::size_t> parentPath = nodePathByLevel[level];
        YamlNode& parent = getNodeByPath(root, parentPath);
        const std::size_t newIndex = parent.addChild(node);

        std::vector<std::size_t> currentPath = std::move(parentPath);
        currentPath.push_back(newIndex);

        if (nodePathByLevel.size() > static_cast<std::size_t>(level + 1)) {
            nodePathByLevel.resize(static_cast<std::size_t>(level + 1));
        }

        nodePathByLevel.push_back(currentPath);
    }

    return root;
}
