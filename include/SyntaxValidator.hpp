#ifndef YAML_CONFIG_VALIDATOR_SYNTAX_VALIDATOR_HPP
#define YAML_CONFIG_VALIDATOR_SYNTAX_VALIDATOR_HPP

#include <string>
#include <vector>

#include "ValidationReport.hpp"

/**
 * @brief Проверяет базовый синтаксис и структуру YAML-конфигурации.
 */
class SyntaxValidator {
   public:
    /**
     * @brief Создает валидатор синтаксиса.
     * @param spacesPerLevel Количество пробелов для одного уровня вложенности.
     * @throws std::runtime_error если шаг отступа не положительный.
     */
    explicit SyntaxValidator(int spacesPerLevel = 2);

    /**
     * @brief Проверяет расширение файла, доступность и структуру YAML.
     * @param filePath Путь к файлу, полученный от пользователя.
     * @return Отчет со всеми найденными проблемами синтаксиса и структуры.
     */
    ValidationReport validateFile(const std::string& filePath) const;

    /**
     * @brief Проверяет строки YAML. Метод удобен для тестов.
     * @param lines Содержимое YAML, разделенное на строки.
     * @param fileName Имя файла для проверки расширения.
     * @return Отчет со всеми найденными проблемами синтаксиса и структуры.
     */
    ValidationReport validateLines(const std::vector<std::string>& lines,
                                   const std::string& fileName = "config.yaml") const;

   private:
    /**
     * @brief Количество пробелов в одном уровне вложенности YAML.
     */
    int spacesPerLevel;
};

#endif
