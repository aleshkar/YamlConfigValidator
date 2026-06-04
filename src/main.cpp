#include <exception>
#include <iostream>
#include <stdexcept>

#include "ResourceValidator.hpp"
#include "SecretValidator.hpp"
#include "SyntaxValidator.hpp"
#include "TreePrinter.hpp"
#include "YamlParser.hpp"

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            throw std::runtime_error(
                "Использование: YamlConfigValidator <путь-к-yaml-файлу>");
        }

        const std::string filePath = argv[1];
        const SyntaxValidator syntaxValidator;
        ValidationReport finalReport = syntaxValidator.validateFile(filePath);

        if (finalReport.hasErrors()) {
            std::cout << finalReport.toString("Отчет проверки") << "\n";
            return 1;
        }

        const YamlParser parser;
        const YamlNode root = parser.parseFile(filePath);

        const SecretValidator secretValidator;
        const ValidationReport secretReport = secretValidator.validate(root);
        finalReport.merge(secretReport);

        const ResourceValidator resourceValidator;
        const ValidationReport resourceReport = resourceValidator.validate(root);
        finalReport.merge(resourceReport);

        std::cout << finalReport.toString("Отчет проверки") << "\n";

        const TreePrinter printer;
        if (finalReport.isValid()) {
            std::cout << printer.print(root);
        }

        return finalReport.isValid() ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << "\n";
        return 1;
    }
}
