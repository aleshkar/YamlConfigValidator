#include "YamlNode.hpp"

#include <utility>

YamlNode::YamlNode(std::string nodeKey, std::string nodeValue, int sourceLine,
                   int sourceIndent, bool isListItem)
    : key(std::move(nodeKey)),
      value(std::move(nodeValue)),
      lineNumber(sourceLine),
      indent(sourceIndent),
      listItem(isListItem) {}

std::size_t YamlNode::addChild(const YamlNode& child) {
    children.push_back(child);
    return children.size() - 1;
}

bool YamlNode::hasValue() const {
    return !value.empty();
}
