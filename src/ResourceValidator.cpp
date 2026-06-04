#include "ResourceValidator.hpp"

#include <cctype>
#include <cmath>
#include <exception>
#include <string>
#include <vector>

#include "YamlTextUtils.hpp"

namespace {

const YamlNode* findChild(const YamlNode& node, const std::string& key) {
    for (const YamlNode& child : node.children) {
        if (child.key == key) {
            return &child;
        }
    }

    return nullptr;
}

const YamlNode* findFirstChild(const YamlNode& node,
                               const std::vector<std::string>& keys) {
    for (const std::string& key : keys) {
        const YamlNode* child = findChild(node, key);
        if (child != nullptr) {
            return child;
        }
    }

    return nullptr;
}

std::string toLowerText(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (const char ch : text) {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }

    return result;
}

std::string stripQuotes(const std::string& value) {
    const std::string text = trimText(value);
    if (text.size() < 2) {
        return text;
    }

    const char first = text.front();
    const char last = text.back();
    if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
        return text.substr(1, text.size() - 2);
    }

    return text;
}

bool isPositiveInteger(const std::string& text) {
    if (text.empty()) {
        return false;
    }

    for (const char ch : text) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }

    return std::stoll(text) > 0;
}

bool parseCpuToMilliCores(const std::string& value, int& result) {
    const std::string cpuText = stripQuotes(value);
    if (cpuText.empty()) {
        return false;
    }

    if (cpuText.back() == 'm') {
        const std::string milliCores = cpuText.substr(0, cpuText.size() - 1);
        if (!isPositiveInteger(milliCores)) {
            return false;
        }

        result = static_cast<int>(std::stoll(milliCores));
        return true;
    }

    std::size_t parsedChars = 0;
    double cores = 0.0;
    try {
        cores = std::stod(cpuText, &parsedChars);
    } catch (const std::exception&) {
        return false;
    }

    if (parsedChars != cpuText.size() || cores <= 0.0) {
        return false;
    }

    result = static_cast<int>(std::round(cores * 1000.0));
    if (result <= 0) {
        return false;
    }

    return true;
}

bool parseMemoryToMi(const std::string& value, int& result) {
    const std::string memoryText = stripQuotes(value);
    if (memoryText.size() < 2) {
        return false;
    }

    std::size_t suffixStart = memoryText.size();
    while (suffixStart > 0 &&
           std::isalpha(static_cast<unsigned char>(memoryText[suffixStart - 1]))) {
        --suffixStart;
    }

    const std::string amountText = memoryText.substr(0, suffixStart);
    const std::string suffix = toLowerText(memoryText.substr(suffixStart));
    if (!isPositiveInteger(amountText)) {
        return false;
    }

    const int amount = static_cast<int>(std::stoll(amountText));
    if (suffix == "mi" || suffix == "m" || suffix == "mb") {
        result = amount;
        return true;
    }

    if (suffix == "gi" || suffix == "g" || suffix == "gb") {
        result = amount * 1024;
        return true;
    }

    return false;
}

const YamlNode* findCpuNode(const YamlNode& block) {
    return findFirstChild(block, {"cpu", "cpus"});
}

const YamlNode* findMemoryNode(const YamlNode& block) {
    return findFirstChild(block, {"memory", "mem_limit"});
}

bool validateCpuValue(const YamlNode& block, ValidationReport& report) {
    const YamlNode* valueNode = findCpuNode(block);
    if (valueNode == nullptr) {
        report.addError(block.lineNumber, "проверка-ресурсов",
                        "блок '" + block.key + "' должен содержать cpu или cpus");
        return false;
    }

    int parsedValue = 0;
    const bool valueIsValid = parseCpuToMilliCores(valueNode->value, parsedValue);

    if (!valueIsValid) {
        report.addError(
            valueNode->lineNumber, "проверка-ресурсов",
            "некорректное значение " + valueNode->key + " в блоке '" + block.key + "'");
    }

    return valueIsValid;
}

bool validateMemoryValue(const YamlNode& block, ValidationReport& report) {
    const YamlNode* valueNode = findMemoryNode(block);
    if (valueNode == nullptr) {
        report.addError(block.lineNumber, "проверка-ресурсов",
                        "блок '" + block.key + "' должен содержать memory или mem_limit");
        return false;
    }

    int parsedValue = 0;
    const bool valueIsValid = parseMemoryToMi(valueNode->value, parsedValue);

    if (!valueIsValid) {
        report.addError(
            valueNode->lineNumber, "проверка-ресурсов",
            "некорректное значение " + valueNode->key + " в блоке '" + block.key + "'");
    }

    return valueIsValid;
}

void compareCpu(const YamlNode& requests, const YamlNode& limits,
                ValidationReport& report) {
    const YamlNode* requestCpu = findCpuNode(requests);
    const YamlNode* limitCpu = findCpuNode(limits);
    if (requestCpu == nullptr || limitCpu == nullptr) {
        return;
    }

    int requestValue = 0;
    int limitValue = 0;
    if (!parseCpuToMilliCores(requestCpu->value, requestValue) ||
        !parseCpuToMilliCores(limitCpu->value, limitValue)) {
        return;
    }

    if (limitValue < requestValue) {
        report.addError(limitCpu->lineNumber, "проверка-ресурсов",
                        "лимит CPU не может быть меньше запрошенного CPU");
    }
}

void compareMemory(const YamlNode& requests, const YamlNode& limits,
                   ValidationReport& report) {
    const YamlNode* requestMemory = findMemoryNode(requests);
    const YamlNode* limitMemory = findMemoryNode(limits);
    if (requestMemory == nullptr || limitMemory == nullptr) {
        return;
    }

    int requestValue = 0;
    int limitValue = 0;
    if (!parseMemoryToMi(requestMemory->value, requestValue) ||
        !parseMemoryToMi(limitMemory->value, limitValue)) {
        return;
    }

    if (limitValue < requestValue) {
        report.addError(limitMemory->lineNumber, "проверка-ресурсов",
                        "лимит памяти не может быть меньше запрошенной памяти");
    }
}

void validateResourceBlock(const YamlNode& resources, ValidationReport& report) {
    const YamlNode* requests = findFirstChild(resources, {"requests", "reservations"});
    const YamlNode* limits = findChild(resources, "limits");

    if (requests == nullptr) {
        report.addError(resources.lineNumber, "проверка-ресурсов",
                        "resources должен содержать блок requests или reservations");
    }

    if (limits == nullptr) {
        report.addError(resources.lineNumber, "проверка-ресурсов",
                        "resources должен содержать блок limits");
    }

    if (requests == nullptr || limits == nullptr) {
        return;
    }

    validateCpuValue(*requests, report);
    validateMemoryValue(*requests, report);
    validateCpuValue(*limits, report);
    validateMemoryValue(*limits, report);

    compareCpu(*requests, *limits, report);
    compareMemory(*requests, *limits, report);
}

}  // namespace

ValidationReport ResourceValidator::validate(const YamlNode& root) const {
    ValidationReport report;
    validateNode(root, report);
    return report;
}

void ResourceValidator::validateNode(const YamlNode& node,
                                     ValidationReport& report) const {
    if (node.key == "resources") {
        validateResourceBlock(node, report);
    }

    for (const YamlNode& child : node.children) {
        validateNode(child, report);
    }
}
