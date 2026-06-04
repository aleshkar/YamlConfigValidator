#ifndef YAML_CONFIG_VALIDATOR_YAML_TEXT_UTILS_HPP
#define YAML_CONFIG_VALIDATOR_YAML_TEXT_UTILS_HPP

#include <string>

/**
 * @brief Удаляет пробелы в начале и конце строки.
 * @param text Исходный текст.
 * @return Текст без пробелов по краям.
 */
std::string trimText(const std::string& text);

/**
 * @brief Удаляет YAML-комментарий из строки.
 * @param line Исходная строка.
 * @return Строка без комментария. Символ # внутри кавычек сохраняется.
 */
std::string removeYamlComment(const std::string& line);

/**
 * @brief Считает пробелы перед первым непробельным символом.
 * @param line Исходная YAML-строка.
 * @param lineNumber Номер строки, который используется в сообщениях об ошибках.
 * @return Количество начальных пробелов.
 * @throws std::runtime_error если в отступе используется табуляция.
 */
int countYamlIndent(const std::string& line, int lineNumber);

/**
 * @brief Получает имя файла из полного или относительного пути.
 * @param filePath Путь к файлу.
 * @return Имя файла без родительских папок.
 */
std::string fileNameFromPath(const std::string& filePath);

#endif
