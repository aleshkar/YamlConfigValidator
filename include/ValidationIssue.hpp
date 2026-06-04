#ifndef YAML_CONFIG_VALIDATOR_VALIDATION_ISSUE_HPP
#define YAML_CONFIG_VALIDATOR_VALIDATION_ISSUE_HPP

#include <string>

/**
 * @brief Уровень серьезности сообщения проверки.
 */
enum class IssueLevel {
    /**
     * @brief Информационное сообщение.
     */
    Info,

    /**
     * @brief Предупреждение, которое не делает файл невалидным.
     */
    Warning,

    /**
     * @brief Ошибка, из-за которой файл считается невалидным.
     */
    Error
};

/**
 * @brief Одно сообщение, найденное во время проверки.
 */
struct ValidationIssue {
    /**
     * @brief Уровень серьезности сообщения.
     */
    IssueLevel level = IssueLevel::Info;

    /**
     * @brief Номер строки в исходном файле. Значение 0 означает проблему всего файла.
     */
    int lineNumber = 0;

    /**
     * @brief Название проверки, которая создала сообщение.
     */
    std::string checkName;

    /**
     * @brief Понятное пользователю объяснение.
     */
    std::string message;
};

/**
 * @brief Преобразует уровень сообщения в читаемый текст.
 * @param level Уровень серьезности сообщения.
 * @return Текстовое название уровня.
 */
std::string issueLevelToString(IssueLevel level);

#endif
