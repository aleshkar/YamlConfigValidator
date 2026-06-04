#include "ValidationIssue.hpp"

std::string issueLevelToString(IssueLevel level) {
    switch (level) {
        case IssueLevel::Info:
            return "ИНФО";
        case IssueLevel::Warning:
            return "ПРЕДУПРЕЖДЕНИЕ";
        case IssueLevel::Error:
            return "ОШИБКА";
    }

    return "НЕИЗВЕСТНО";
}
