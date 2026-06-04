#include "TreePrinter.hpp"

std::string TreePrinter::print(const YamlNode& root) const {
    std::string result = formatNode(root) + "\n";

    for (std::size_t i = 0; i < root.children.size(); ++i) {
        printChildren(root.children[i], "", i + 1 == root.children.size(), result);
    }

    return result;
}

std::string TreePrinter::formatNode(const YamlNode& node) const {
    std::string label;

    if (node.listItem) {
        if (node.key == "-") {
            return "- " + node.value;
        }

        label += "- ";
    }

    label += node.key;

    if (node.hasValue()) {
        label += ": ";
        label += node.value;
    }

    return label;
}

void TreePrinter::printChildren(const YamlNode& node, const std::string& prefix,
                                bool isLast, std::string& result) const {
    result += prefix;
    result += isLast ? "`-- " : "|-- ";
    result += formatNode(node);
    result += "\n";

    const std::string childPrefix = prefix + (isLast ? "    " : "|   ");
    for (std::size_t i = 0; i < node.children.size(); ++i) {
        printChildren(node.children[i], childPrefix, i + 1 == node.children.size(),
                      result);
    }
}
