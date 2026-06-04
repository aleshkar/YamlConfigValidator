#ifndef YAML_CONFIG_VALIDATOR_SECRET_VALIDATOR_HPP
#define YAML_CONFIG_VALIDATOR_SECRET_VALIDATOR_HPP

#include "ValidationReport.hpp"
#include "YamlNode.hpp"

/**
 * @brief Проверяет пароли, токены и ключи в разобранном YAML-дереве.
 */
class SecretValidator {
   public:
    /**
     * @brief Проверяет все ключи, похожие на секретные данные.
     * @param root Корневой узел, полученный от YamlParser.
     * @return Отчет с проблемами безопасности и предупреждениями.
     */
    ValidationReport validate(const YamlNode& root) const;

   private:
    /**
     * @brief Рекурсивно проверяет один YAML-узел и его дочерние узлы.
     * @param node Текущий YAML-узел.
     * @param report Отчет, в который добавляются найденные проблемы.
     */
    void validateNode(const YamlNode& node, ValidationReport& report) const;
};

#endif
