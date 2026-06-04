#include "SecretValidator.hpp"

#include <algorithm>
#include <cctype>
#include <set>
#include <string>
#include <vector>

#include "YamlTextUtils.hpp"

namespace {

std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

    return text;
}

std::string stripQuotes(const std::string& value) {
    if (value.size() < 2) {
        return value;
    }

    const char first = value.front();
    const char last = value.back();
    if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
        return value.substr(1, value.size() - 2);
    }

    return value;
}

bool containsAny(const std::string& text, const std::vector<std::string>& words) {
    for (const std::string& word : words) {
        if (text.find(word) != std::string::npos) {
            return true;
        }
    }

    return false;
}

bool isSecretKey(const std::string& key) {
    const std::string lowerKey = toLower(key);
    const std::vector<std::string> secretWords = {"password",   "pass",       "token",
                                                  "secret",     "api_key",    "apikey",
                                                  "access_key", "private_key"};

    return containsAny(lowerKey, secretWords);
}

bool isPasswordKey(const std::string& key) {
    const std::string lowerKey = toLower(key);
    return lowerKey.find("password") != std::string::npos ||
           lowerKey.find("pass") != std::string::npos;
}

bool isTokenOrKey(const std::string& key) {
    const std::string lowerKey = toLower(key);
    const std::vector<std::string> tokenWords = {"token",      "api_key",     "apikey",
                                                 "access_key", "private_key", "secret"};

    return containsAny(lowerKey, tokenWords);
}

bool looksLikeEnvironmentReference(const std::string& value) {
    return value.size() >= 4 && value.rfind("${", 0) == 0 && value.back() == '}';
}

bool isEnvironmentReference(const std::string& value) {
    if (!looksLikeEnvironmentReference(value)) {
        return false;
    }

    const std::string variableName = value.substr(2, value.size() - 3);
    if (variableName.empty()) {
        return false;
    }

    const char first = variableName.front();
    if (!std::isalpha(static_cast<unsigned char>(first)) && first != '_') {
        return false;
    }

    for (const char ch : variableName) {
        const bool allowed = std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
        if (!allowed) {
            return false;
        }
    }

    return true;
}

bool isWeakSecretValue(const std::string& value) {
    const std::set<std::string> weakValues = {"admin", "password", "123456",  "qwerty",
                                              "test",  "root",     "changeme"};

    return weakValues.find(toLower(value)) != weakValues.end();
}

bool isBadLiteral(const std::string& value) {
    const std::set<std::string> badValues = {"true", "false", "none", "null"};
    return badValues.find(toLower(value)) != badValues.end();
}

bool containsOnlyDigits(const std::string& value) {
    if (value.empty()) {
        return false;
    }

    for (const char ch : value) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }

    return true;
}

bool hasEnglishLetter(const std::string& value) {
    for (const char ch : value) {
        const bool isSmallLetter = ch >= 'a' && ch <= 'z';
        const bool isBigLetter = ch >= 'A' && ch <= 'Z';
        if (isSmallLetter || isBigLetter) {
            return true;
        }
    }

    return false;
}

bool hasSpecialCharacter(const std::string& value) {
    for (const char ch : value) {
        if (!std::isalnum(static_cast<unsigned char>(ch))) {
            return true;
        }
    }

    return false;
}

void checkSecretValue(const YamlNode& node, ValidationReport& report) {
    const std::string secretValue = stripQuotes(trimText(node.value));

    if (secretValue.empty()) {
        report.addError(node.lineNumber, "проверка-секретов",
                        "секретный ключ '" + node.key + "' имеет пустое значение");
        return;
    }

    if (isEnvironmentReference(secretValue)) {
        return;
    }

    if (looksLikeEnvironmentReference(secretValue)) {
        report.addError(
            node.lineNumber, "проверка-секретов",
            "некорректная ссылка на переменную окружения в ключе '" + node.key + "'");
        return;
    }

    if (isWeakSecretValue(secretValue)) {
        report.addError(node.lineNumber, "проверка-секретов",
                        "секретный ключ '" + node.key + "' использует слабое значение");
    }

    if (isBadLiteral(secretValue)) {
        report.addError(
            node.lineNumber, "проверка-секретов",
            "секретный ключ '" + node.key + "' не может быть true, false, none или null");
    }

    if (isPasswordKey(node.key)) {
        if (secretValue.size() < 8) {
            report.addError(node.lineNumber, "проверка-секретов",
                            "значение пароля короче 8 символов");
        }

        if (containsOnlyDigits(secretValue)) {
            report.addError(node.lineNumber, "проверка-секретов",
                            "пароль не должен состоять только из цифр");
        }

        if (!hasEnglishLetter(secretValue)) {
            report.addError(node.lineNumber, "проверка-секретов",
                            "пароль должен содержать английские буквы");
        }

        if (!hasSpecialCharacter(secretValue)) {
            report.addError(node.lineNumber, "проверка-секретов",
                            "пароль должен содержать спецсимвол");
        }
    }

    if (isTokenOrKey(node.key) && secretValue.size() < 16) {
        report.addError(node.lineNumber, "проверка-секретов",
                        "значение токена или ключа короче 16 символов");
    }

    report.addWarning(node.lineNumber, "проверка-секретов",
                      "секретный ключ '" + node.key +
                          "' хранится прямо в YAML; лучше использовать ${ENV_VAR}");
}

}  // namespace

ValidationReport SecretValidator::validate(const YamlNode& root) const {
    ValidationReport report;
    validateNode(root, report);
    return report;
}

void SecretValidator::validateNode(const YamlNode& node, ValidationReport& report) const {
    if (isSecretKey(node.key)) {
        checkSecretValue(node, report);
    }

    for (const YamlNode& child : node.children) {
        validateNode(child, report);
    }
}
