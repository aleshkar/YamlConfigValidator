#ifndef YAML_CONFIG_VALIDATOR_YAML_NODE_HPP
#define YAML_CONFIG_VALIDATOR_YAML_NODE_HPP

#include <cstddef>
#include <string>
#include <vector>

/**
 * @brief Один узел разобранного YAML-дерева.
 *
 * Проект поддерживает простой набор возможностей YAML: ключи, значения,
 * вложенные блоки и элементы списков. Структура хранит результат разбора в
 * виде дерева, которое потом можно вывести в консоль и проверить валидаторами.
 */
struct YamlNode {
    /**
     * @brief Имя ключа из строки YAML.
     */
    std::string key;

    /**
     * @brief Значение после ':' или обычное значение элемента списка.
     */
    std::string value;

    /**
     * @brief Номер строки в исходном файле. Для корневого узла используется 0.
     */
    int lineNumber = 0;

    /**
     * @brief Количество пробелов в начале исходной строки.
     */
    int indent = 0;

    /**
     * @brief Показывает, был ли узел создан из элемента YAML-списка.
     */
    bool listItem = false;

    /**
     * @brief Вложенные YAML-узлы.
     */
    std::vector<YamlNode> children;

    /**
     * @brief Создает пустой YAML-узел.
     */
    YamlNode() = default;

    /**
     * @brief Создает узел YAML-дерева.
     * @param nodeKey Имя ключа или "-" для обычного элемента списка.
     * @param nodeValue Скалярное значение узла.
     * @param sourceLine Номер строки в исходном файле.
     * @param sourceIndent Отступ строки в пробелах.
     * @param isListItem true, если узел является элементом YAML-списка.
     */
    YamlNode(std::string nodeKey, std::string nodeValue, int sourceLine, int sourceIndent,
             bool isListItem);

    /**
     * @brief Добавляет дочерний узел и возвращает его индекс.
     * @param child Узел, который нужно добавить внутрь текущего узла.
     * @return Индекс добавленного узла внутри вектора children.
     */
    std::size_t addChild(const YamlNode& child);

    /**
     * @brief Проверяет, есть ли у узла скалярное значение.
     * @return true, если поле value не пустое.
     */
    bool hasValue() const;
};

#endif
