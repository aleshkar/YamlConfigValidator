#ifndef YAML_CONFIG_VALIDATOR_YAML_PARSER_HPP
#define YAML_CONFIG_VALIDATOR_YAML_PARSER_HPP

#include <string>
#include <vector>

#include "YamlNode.hpp"

/**
 * @brief Строит дерево из упрощенного YAML-файла.
 *
 * Парсер специально сделан небольшим и предсказуемым. Он не пытается покрыть
 * весь стандарт YAML, но подходит для конфигов приложений и DevOps-конфигов,
 * которые используются в этом учебном проекте.
 */
class YamlParser {
   public:
    /**
     * @brief Создает парсер с фиксированным шагом отступа.
     * @param spacesPerLevel Количество пробелов для одного уровня вложенности.
     * @throws std::runtime_error если шаг отступа не положительный.
     */
    explicit YamlParser(int spacesPerLevel = 2);

    /**
     * @brief Разбирает YAML-файл с диска.
     * @param filePath Путь к файлу, полученный от пользователя.
     * @return Корневой узел с разобранными дочерними узлами.
     * @throws std::runtime_error если файл не удалось прочитать или YAML некорректен.
     */
    YamlNode parseFile(const std::string& filePath) const;

    /**
     * @brief Разбирает YAML-текст, заранее разделенный на строки.
     * @param lines Строки YAML-файла.
     * @param rootName Имя виртуального корневого узла.
     * @return Корневой узел с разобранными дочерними узлами.
     * @throws std::runtime_error если отступы или структура строки некорректны.
     */
    YamlNode parseLines(const std::vector<std::string>& lines,
                        const std::string& rootName = "yaml") const;

   private:
    /**
     * @brief Количество пробелов в одном уровне вложенности YAML.
     */
    int spacesPerLevel;
};

#endif
