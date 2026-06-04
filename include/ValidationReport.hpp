#ifndef YAML_CONFIG_VALIDATOR_VALIDATION_REPORT_HPP
#define YAML_CONFIG_VALIDATOR_VALIDATION_REPORT_HPP

#include <string>
#include <vector>

#include "ValidationIssue.hpp"

/**
 * @brief Хранит все сообщения, которые были созданы валидаторами.
 */
class ValidationReport {
   public:
    /**
     * @brief Добавляет произвольное сообщение в отчет.
     * @param level Уровень серьезности сообщения.
     * @param lineNumber Номер строки. Значение 0 используется для ошибок всего файла.
     * @param checkName Название правила проверки.
     * @param message Понятное пользователю сообщение.
     */
    void addIssue(IssueLevel level, int lineNumber, const std::string& checkName,
                  const std::string& message);

    /**
     * @brief Добавляет сообщение об ошибке.
     * @param lineNumber Номер строки или 0 для ошибок всего файла.
     * @param checkName Название правила проверки.
     * @param message Понятное пользователю сообщение.
     */
    void addError(int lineNumber, const std::string& checkName,
                  const std::string& message);

    /**
     * @brief Добавляет предупреждение.
     * @param lineNumber Номер строки или 0 для предупреждений всего файла.
     * @param checkName Название правила проверки.
     * @param message Понятное пользователю сообщение.
     */
    void addWarning(int lineNumber, const std::string& checkName,
                    const std::string& message);

    /**
     * @brief Добавляет информационное сообщение.
     * @param lineNumber Номер строки или 0 для сообщений всего файла.
     * @param checkName Название правила проверки.
     * @param message Понятное пользователю сообщение.
     */
    void addInfo(int lineNumber, const std::string& checkName,
                 const std::string& message);

    /**
     * @brief Добавляет все сообщения из другого отчета.
     * @param other Отчет, сообщения которого нужно скопировать.
     */
    void merge(const ValidationReport& other);

    /**
     * @brief Проверяет, есть ли в отчете хотя бы одна ошибка.
     * @return true, если в отчете есть ошибка.
     */
    bool hasErrors() const;

    /**
     * @brief Проверяет, завершилась ли проверка без ошибок.
     * @return true, если ошибок нет.
     */
    bool isValid() const;

    /**
     * @brief Считает количество ошибок.
     * @return Количество ошибок в отчете.
     */
    std::size_t errorCount() const;

    /**
     * @brief Считает количество предупреждений.
     * @return Количество предупреждений в отчете.
     */
    std::size_t warningCount() const;

    /**
     * @brief Считает все сообщения.
     * @return Количество сохраненных сообщений.
     */
    std::size_t issueCount() const;

    /**
     * @brief Возвращает все собранные сообщения.
     * @return Вектор с сообщениями проверки.
     */
    const std::vector<ValidationIssue>& issues() const;

    /**
     * @brief Форматирует отчет для вывода в консоль.
     * @param title Заголовок, который выводится перед сообщениями.
     * @param showResult Если true, печатает итоговый статус проверки.
     * @return Готовый текст отчета.
     */
    std::string toString(const std::string& title = "Отчет проверки",
                         bool showResult = true) const;

   private:
    /**
     * @brief Собранные сообщения проверки.
     */
    std::vector<ValidationIssue> items;
};

#endif
