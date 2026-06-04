#include "YamlTextUtils.hpp"

#include <cctype>
#include <stdexcept>

std::string trimText(const std::string& text) {
    std::size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }

    std::size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }

    return text.substr(start, end - start);
}

std::string removeYamlComment(const std::string& line) {
    bool insideSingleQuotes = false;
    bool insideDoubleQuotes = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char current = line[i];
        if (current == '\'' && !insideDoubleQuotes) {
            insideSingleQuotes = !insideSingleQuotes;
        } else if (current == '"' && !insideSingleQuotes) {
            insideDoubleQuotes = !insideDoubleQuotes;
        } else if (current == '#' && !insideSingleQuotes && !insideDoubleQuotes) {
            return line.substr(0, i);
        }
    }

    return line;
}

int countYamlIndent(const std::string& line, int lineNumber) {
    int indent = 0;
    for (const char current : line) {
        if (current == ' ') {
            ++indent;
        } else if (current == '\t') {
            throw std::runtime_error("Строка " + std::to_string(lineNumber) +
                                     ": табуляция запрещена в отступах");
        } else {
            break;
        }
    }

    return indent;
}

std::string fileNameFromPath(const std::string& filePath) {
    const std::size_t slash = filePath.find_last_of('/');
    if (slash == std::string::npos) {
        return filePath;
    }

    return filePath.substr(slash + 1);
}
