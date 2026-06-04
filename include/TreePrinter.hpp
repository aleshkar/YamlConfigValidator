#ifndef YAML_CONFIG_VALIDATOR_TREE_PRINTER_HPP
#define YAML_CONFIG_VALIDATOR_TREE_PRINTER_HPP

#include <string>

#include "YamlNode.hpp"

/**
 * @brief Преобразует разобранное YAML-дерево в удобный текстовый вид для консоли.
 */
class TreePrinter {
   public:
    /**
     * @brief Создает текстовое представление YAML-дерева.
     * @param root Корневой узел, полученный от YamlParser.
     * @return Готовый текст дерева.
     */
    std::string print(const YamlNode& root) const;

   private:
    /**
     * @brief Форматирует один узел без дочерних веток.
     * @param node Узел для форматирования.
     * @return Текстовая подпись узла.
     */
    std::string formatNode(const YamlNode& node) const;

    /**
     * @brief Рекурсивно добавляет дочерние узлы в вывод.
     * @param node Текущий узел.
     * @param prefix Уже подготовленный префикс дерева.
     * @param isLast true, если текущий узел является последним дочерним узлом.
     * @param result Строка вывода, в которую добавляются готовые строки.
     */
    void printChildren(const YamlNode& node, const std::string& prefix, bool isLast,
                       std::string& result) const;
};

#endif
