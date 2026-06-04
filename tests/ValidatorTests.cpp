#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <stdexcept>
#include <string>
#include <vector>

#include "ResourceValidator.hpp"
#include "SecretValidator.hpp"
#include "ValidationReport.hpp"
#include "YamlParser.hpp"

// Сторонняя библиотека doctest взята из официального репозитория:
// https://github.com/doctest/doctest/blob/master/doctest/doctest.h
#include "doctest/doctest.h"

namespace {

YamlNode parseYaml(const std::vector<std::string>& lines) {
    const YamlParser parser;
    return parser.parseLines(lines);
}

ValidationReport validateResources(const std::vector<std::string>& lines) {
    const ResourceValidator validator;
    return validator.validate(parseYaml(lines));
}

ValidationReport validateSecrets(const std::vector<std::string>& lines) {
    const SecretValidator validator;
    return validator.validate(parseYaml(lines));
}

bool hasIssueAtLine(const ValidationReport& report, IssueLevel level, int lineNumber,
                    const std::string& messagePart) {
    for (const ValidationIssue& issue : report.issues()) {
        if (issue.level == level && issue.lineNumber == lineNumber &&
            issue.message.find(messagePart) != std::string::npos) {
            return true;
        }
    }

    return false;
}

bool hasErrorAtLine(const ValidationReport& report, int lineNumber,
                    const std::string& messagePart) {
    return hasIssueAtLine(report, IssueLevel::Error, lineNumber, messagePart);
}

bool hasWarningAtLine(const ValidationReport& report, int lineNumber,
                      const std::string& messagePart) {
    return hasIssueAtLine(report, IssueLevel::Warning, lineNumber, messagePart);
}

std::string getParserError(const std::vector<std::string>& lines) {
    try {
        parseYaml(lines);
    } catch (const std::runtime_error& error) {
        return error.what();
    }

    return "";
}

bool containsText(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

}  // namespace

TEST_CASE("YamlParser разбирает обычный серверный конфиг") {
    const YamlNode root = parseYaml({
        "server:",
        "  host: 0.0.0.0",
        "  port: 8080",
        "database:",
        "  user: payment_user",
        "  password: ${DB_PASSWORD}",
    });

    REQUIRE(root.children.size() == 2);

    const YamlNode& server = root.children[0];
    CHECK(server.key == "server");
    REQUIRE(server.children.size() == 2);
    CHECK(server.children[0].key == "host");
    CHECK(server.children[0].value == "0.0.0.0");
    CHECK(server.children[1].key == "port");
    CHECK(server.children[1].value == "8080");

    const YamlNode& database = root.children[1];
    CHECK(database.key == "database");
    REQUIRE(database.children.size() == 2);
    CHECK(database.children[0].key == "user");
    CHECK(database.children[0].value == "payment_user");
    CHECK(database.children[1].key == "password");
    CHECK(database.children[1].value == "${DB_PASSWORD}");
}

TEST_CASE("YamlParser пропускает пустые строки и комментарии") {
    const YamlNode root = parseYaml({
        "",
        "# main server block",
        "server:",
        "  # host is local in tests",
        "  host: localhost",
        "",
        "  port: 8080 # default port",
        "title: \"payment # main\"",
        "# end",
    });

    REQUIRE(root.children.size() == 2);

    const YamlNode& server = root.children[0];
    CHECK(server.key == "server");
    REQUIRE(server.children.size() == 2);
    CHECK(server.children[0].key == "host");
    CHECK(server.children[0].value == "localhost");
    CHECK(server.children[0].lineNumber == 5);
    CHECK(server.children[1].key == "port");
    CHECK(server.children[1].value == "8080");
    CHECK(server.children[1].lineNumber == 7);

    CHECK(root.children[1].key == "title");
    CHECK(root.children[1].value == "\"payment # main\"");
}

TEST_CASE("YamlParser разбирает вложенный список сервисов") {
    const YamlNode root = parseYaml({
        "services:",
        "  - name: api",
        "    image: registry.example.com/payment-api:1.4.2",
        "    ports:",
        "      - 8080",
        "  - name: worker",
        "    queue: payments",
    });

    REQUIRE(root.children.size() == 1);

    const YamlNode& services = root.children[0];
    CHECK(services.key == "services");
    REQUIRE(services.children.size() == 2);

    const YamlNode& api = services.children[0];
    CHECK(api.listItem);
    CHECK(api.key == "name");
    CHECK(api.value == "api");
    REQUIRE(api.children.size() == 2);
    CHECK(api.children[0].key == "image");
    CHECK(api.children[0].value == "registry.example.com/payment-api:1.4.2");

    const YamlNode& ports = api.children[1];
    CHECK(ports.key == "ports");
    REQUIRE(ports.children.size() == 1);
    CHECK(ports.children[0].listItem);
    CHECK(ports.children[0].value == "8080");

    const YamlNode& worker = services.children[1];
    CHECK(worker.listItem);
    CHECK(worker.key == "name");
    CHECK(worker.value == "worker");
    REQUIRE(worker.children.size() == 1);
    CHECK(worker.children[0].key == "queue");
    CHECK(worker.children[0].value == "payments");
}

TEST_CASE("YamlParser отклоняет некорректный YAML") {
    SUBCASE("отступ не кратен шагу парсера") {
        const std::string error = getParserError({"server:", " host: localhost"});

        CHECK(containsText(error, "Строка 2"));
        CHECK(containsText(error, "отступ должен быть кратен"));
    }

    SUBCASE("в строке нет разделителя ключа и значения") {
        const std::string error = getParserError({"server:", "  host localhost"});

        CHECK(containsText(error, "Строка 2"));
        CHECK(containsText(error, "ожидалась пара ключ-значение"));
    }

    SUBCASE("отступ резко перескакивает через уровень") {
        const std::string error = getParserError({"server:", "    host: localhost"});

        CHECK(containsText(error, "Строка 2"));
        CHECK(containsText(error, "слишком резкий скачок вложенности"));
    }

    SUBCASE("ключ пустой") {
        const std::string error = getParserError({": value"});

        CHECK(containsText(error, "Строка 1"));
        CHECK(containsText(error, "ключ не может быть пустым"));
    }
}

TEST_CASE("SecretValidator принимает ссылки на переменные окружения") {
    const ValidationReport report = validateSecrets({
        "database:",
        "  password: ${DB_PASSWORD}",
        "auth:",
        "  token: ${APP_TOKEN}",
        "  api_key: ${PAYMENT_API_KEY}",
    });

    CHECK_FALSE(report.hasErrors());
    CHECK(report.errorCount() == 0);
    CHECK(report.warningCount() == 0);
    CHECK(report.issueCount() == 0);
}

TEST_CASE("SecretValidator принимает граничные длины секретов") {
    const ValidationReport report = validateSecrets({
        "auth:",
        "  password: abcd123!",
        "  token: 1234567890abcdef",
    });

    CHECK_FALSE(report.hasErrors());
    CHECK(report.errorCount() == 0);
    REQUIRE(report.warningCount() == 2);
    CHECK(hasWarningAtLine(report, 2, "хранится прямо в YAML"));
    CHECK(hasWarningAtLine(report, 3, "хранится прямо в YAML"));
}

TEST_CASE("SecretValidator проверяет глубоко вложенные секреты") {
    const ValidationReport report = validateSecrets({
        "services:",
        "  payment-api:",
        "    database:",
        "      credentials:",
        "        password: ${PAYMENT_DB_PASSWORD}",
        "    auth:",
        "      token: ${PAYMENT_API_TOKEN}",
        "      private_key: ${PAYMENT_PRIVATE_KEY}",
        "  worker:",
        "    queue:",
        "      access_key: ${WORKER_QUEUE_ACCESS_KEY}",
    });

    CHECK_FALSE(report.hasErrors());
    CHECK(report.errorCount() == 0);
    CHECK(report.warningCount() == 0);
    CHECK(report.issueCount() == 0);
}

TEST_CASE("SecretValidator находит небезопасные секреты") {
    SUBCASE("слабый пароль короткий и очевидный") {
        const ValidationReport report = validateSecrets({"password: 123456"});

        REQUIRE(report.errorCount() == 5);
        CHECK(report.warningCount() == 1);
        CHECK(hasErrorAtLine(report, 1, "использует слабое значение"));
        CHECK(hasErrorAtLine(report, 1, "значение пароля короче 8 символов"));
        CHECK(hasErrorAtLine(report, 1, "пароль не должен состоять только из цифр"));
        CHECK(hasErrorAtLine(report, 1, "пароль должен содержать английские буквы"));
        CHECK(hasErrorAtLine(report, 1, "пароль должен содержать спецсимвол"));
        CHECK(hasWarningAtLine(report, 1, "хранится прямо в YAML"));
    }

    SUBCASE("пароль без спецсимвола считается слабым") {
        const ValidationReport report = validateSecrets({"password: abcdefghi"});

        REQUIRE(report.errorCount() == 1);
        CHECK(report.warningCount() == 1);
        CHECK(hasErrorAtLine(report, 1, "пароль должен содержать спецсимвол"));
        CHECK(hasWarningAtLine(report, 1, "хранится прямо в YAML"));
    }

    SUBCASE("пароль без английских букв считается слабым") {
        const ValidationReport report = validateSecrets({"password: 12345678!"});

        REQUIRE(report.errorCount() == 1);
        CHECK(report.warningCount() == 1);
        CHECK(hasErrorAtLine(report, 1, "пароль должен содержать английские буквы"));
        CHECK(hasWarningAtLine(report, 1, "хранится прямо в YAML"));
    }

    SUBCASE("токен слишком короткий") {
        const ValidationReport report = validateSecrets({"token: abc"});

        REQUIRE(report.errorCount() == 1);
        CHECK(report.warningCount() == 1);
        CHECK(hasErrorAtLine(report, 1, "значение токена или ключа короче 16"));
        CHECK(hasWarningAtLine(report, 1, "хранится прямо в YAML"));
    }

    SUBCASE("секретный ключ не может быть boolean или null") {
        const ValidationReport report = validateSecrets({
            "api_key: true",
            "private_key: null",
        });

        REQUIRE(report.errorCount() == 4);
        CHECK(report.warningCount() == 2);
        CHECK(hasErrorAtLine(report, 1, "не может быть true, false, none или null"));
        CHECK(hasErrorAtLine(report, 1, "значение токена или ключа короче 16"));
        CHECK(hasErrorAtLine(report, 2, "не может быть true, false, none или null"));
        CHECK(hasErrorAtLine(report, 2, "значение токена или ключа короче 16"));
    }

    SUBCASE("имя переменной окружения некорректное") {
        const ValidationReport report = validateSecrets({
            "password: ${1234}",
            "token: ${db password}",
        });

        REQUIRE(report.errorCount() == 2);
        CHECK(report.warningCount() == 0);
        CHECK(hasErrorAtLine(report, 1, "некорректная ссылка"));
        CHECK(hasErrorAtLine(report, 2, "некорректная ссылка"));
    }
}

TEST_CASE("ResourceValidator принимает обычные requests и limits") {
    const ValidationReport report = validateResources({
        "resources:",
        "  requests:",
        "    cpu: 500m",
        "    memory: 256Mi",
        "  limits:",
        "    cpu: 1",
        "    memory: 512Mi",
    });

    CHECK_FALSE(report.hasErrors());
    CHECK(report.errorCount() == 0);
    CHECK(report.issueCount() == 0);
}

TEST_CASE("ResourceValidator принимает равные граничные значения") {
    SUBCASE("kubernetes cpu и memory равны после пересчета") {
        const ValidationReport report = validateResources({
            "resources:",
            "  requests:",
            "    cpu: 1000m",
            "    memory: 1024Mi",
            "  limits:",
            "    cpu: 1",
            "    memory: 1Gi",
        });

        CHECK_FALSE(report.hasErrors());
        CHECK(report.issueCount() == 0);
    }

    SUBCASE("docker cpus с дробным значением равны после пересчета") {
        const ValidationReport report = validateResources({
            "deploy:",
            "  resources:",
            "    reservations:",
            "      cpus: \"0.5\"",
            "      memory: 512m",
            "    limits:",
            "      cpus: \"0.5\"",
            "      memory: 512m",
        });

        CHECK_FALSE(report.hasErrors());
        CHECK(report.issueCount() == 0);
    }
}

TEST_CASE("ResourceValidator принимает вложенные resources в стиле Docker") {
    const ValidationReport report = validateResources({
        "services:",
        "  api:",
        "    image: registry.example.com/payment-api:1.4.2",
        "    deploy:",
        "      replicas: 2",
        "      resources:",
        "        reservations:",
        "          cpus: \"0.25\"",
        "          mem_limit: 256m",
        "        limits:",
        "          cpus: \"0.75\"",
        "          mem_limit: 1g",
    });

    CHECK_FALSE(report.hasErrors());
    CHECK(report.errorCount() == 0);
    CHECK(report.issueCount() == 0);
}

TEST_CASE("ResourceValidator находит ошибки в resources") {
    SUBCASE("значение cpu записано в неправильном формате") {
        const ValidationReport report = validateResources({
            "resources:",
            "  requests:",
            "    cpu: half",
            "    memory: 256Mi",
            "  limits:",
            "    cpu: 1",
            "    memory: 512Mi",
        });

        REQUIRE(report.errorCount() == 1);
        CHECK(hasErrorAtLine(report, 3, "некорректное значение cpu"));
    }

    SUBCASE("значение memory записано без единицы измерения") {
        const ValidationReport report = validateResources({
            "resources:",
            "  requests:",
            "    cpu: 500m",
            "    memory: 256",
            "  limits:",
            "    cpu: 1",
            "    memory: 512Mi",
        });

        REQUIRE(report.errorCount() == 1);
        CHECK(hasErrorAtLine(report, 4, "некорректное значение memory"));
    }

    SUBCASE("блок limits отсутствует") {
        const ValidationReport report = validateResources({
            "resources:",
            "  requests:",
            "    cpu: 250m",
            "    memory: 128Mi",
        });

        REQUIRE(report.errorCount() == 1);
        CHECK(hasErrorAtLine(report, 1, "resources должен содержать блок limits"));
    }

    SUBCASE("kubernetes limits меньше requests") {
        const ValidationReport report = validateResources({
            "resources:",
            "  requests:",
            "    cpu: 1000m",
            "    memory: 1Gi",
            "  limits:",
            "    cpu: 500m",
            "    memory: 512Mi",
        });

        REQUIRE(report.errorCount() == 2);
        CHECK(hasErrorAtLine(report, 6, "лимит CPU не может быть меньше"));
        CHECK(hasErrorAtLine(report, 7, "лимит памяти не может быть меньше"));
    }

    SUBCASE("docker limits меньше reservations") {
        const ValidationReport report = validateResources({
            "deploy:",
            "  resources:",
            "    reservations:",
            "      cpus: \"0.75\"",
            "      memory: 1g",
            "    limits:",
            "      cpus: \"0.25\"",
            "      memory: 512m",
        });

        REQUIRE(report.errorCount() == 2);
        CHECK(hasErrorAtLine(report, 7, "лимит CPU не может быть меньше"));
        CHECK(hasErrorAtLine(report, 8, "лимит памяти не может быть меньше"));
    }
}
