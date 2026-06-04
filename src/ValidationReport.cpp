#include "ValidationReport.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

class ReportErrorException : public std::runtime_error {
   public:
    ReportErrorException(int sourceLine, std::string sourceCheck,
                         std::string sourceMessage)
        : std::runtime_error(sourceMessage),
          lineNumber(sourceLine),
          checkName(std::move(sourceCheck)) {}

    int line() const {
        return lineNumber;
    }

    const std::string& check() const {
        return checkName;
    }

   private:
    int lineNumber;
    std::string checkName;
};

}  // namespace

void ValidationReport::addIssue(IssueLevel level, int lineNumber,
                                const std::string& checkName,
                                const std::string& message) {
    items.push_back({level, lineNumber, checkName, message});
}

void ValidationReport::addError(int lineNumber, const std::string& checkName,
                                const std::string& message) {
    try {
        throw ReportErrorException(lineNumber, checkName, message);
    } catch (const ReportErrorException& error) {
        addIssue(IssueLevel::Error, error.line(), error.check(), error.what());
    }
}

void ValidationReport::addWarning(int lineNumber, const std::string& checkName,
                                  const std::string& message) {
    addIssue(IssueLevel::Warning, lineNumber, checkName, message);
}

void ValidationReport::addInfo(int lineNumber, const std::string& checkName,
                               const std::string& message) {
    addIssue(IssueLevel::Info, lineNumber, checkName, message);
}

void ValidationReport::merge(const ValidationReport& other) {
    for (const ValidationIssue& issue : other.issues()) {
        addIssue(issue.level, issue.lineNumber, issue.checkName, issue.message);
    }
}

bool ValidationReport::hasErrors() const {
    for (const ValidationIssue& issue : items) {
        if (issue.level == IssueLevel::Error) {
            return true;
        }
    }

    return false;
}

bool ValidationReport::isValid() const {
    return !hasErrors();
}

std::size_t ValidationReport::errorCount() const {
    std::size_t count = 0;
    for (const ValidationIssue& issue : items) {
        if (issue.level == IssueLevel::Error) {
            ++count;
        }
    }

    return count;
}

std::size_t ValidationReport::warningCount() const {
    std::size_t count = 0;
    for (const ValidationIssue& issue : items) {
        if (issue.level == IssueLevel::Warning) {
            ++count;
        }
    }

    return count;
}

std::size_t ValidationReport::issueCount() const {
    return items.size();
}

const std::vector<ValidationIssue>& ValidationReport::issues() const {
    return items;
}

std::string ValidationReport::toString(const std::string& title, bool showResult) const {
    std::ostringstream output;
    output << title << "\n";

    if (items.empty()) {
        output << "Проблем не найдено\n";
    }

    for (const ValidationIssue& issue : items) {
        output << "[" << issueLevelToString(issue.level) << "]";
        if (issue.lineNumber > 0) {
            output << " строка " << issue.lineNumber;
        } else {
            output << " файл";
        }

        output << " | " << issue.checkName << " | " << issue.message << "\n";
    }

    output << "Ошибки: " << errorCount() << "\n";
    output << "Предупреждения: " << warningCount() << "\n";
    if (showResult) {
        output << "Результат: " << (isValid() ? "валиден" : "невалиден") << "\n";
    }

    return output.str();
}
