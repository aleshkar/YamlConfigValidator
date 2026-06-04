#ifndef YAML_CONFIG_VALIDATOR_RESOURCE_VALIDATOR_HPP
#define YAML_CONFIG_VALIDATOR_RESOURCE_VALIDATOR_HPP

#include "ValidationReport.hpp"
#include "YamlNode.hpp"

/**
 * @brief Проверяет блоки ресурсов в Kubernetes-like и Docker-like конфигах.
 */
class ResourceValidator {
   public:
    /**
     * @brief Проверяет все блоки resources в YAML-дереве.
     * @param root Корневой узел, полученный от YamlParser.
     * @return Отчет с проблемами настройки ресурсов.
     */
    ValidationReport validate(const YamlNode& root) const;

   private:
    /**
     * @brief Рекурсивно ищет блоки resources.
     * @param node Текущий YAML-узел.
     * @param report Отчет, в который добавляются найденные проблемы.
     */
    void validateNode(const YamlNode& node, ValidationReport& report) const;
};

#endif
