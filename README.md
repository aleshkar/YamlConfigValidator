# YamlConfigValidator

YamlConfigValidator - это консольная программа на C++ для проверки YAML/YML
конфигов. Программа читает файл, проверяет его структуру, ищет проблемные
секреты, проверяет блок ресурсов и выводит общий отчет.

Проект не заменяет полный YAML-парсер. Он сделан для типичных конфигов
приложений: настройки сервера, базы данных, авторизация, список сервисов и
ресурсы.

## Возможности

- построение дерева YAML-файла;
- вывод дерева в консоль;
- проверка отступов и табуляции;
- проверка строк без `:`;
- проверка пустых ключей;
- проверка пустых блоков без значения и вложенных элементов;
- проверка повторяющихся ключей внутри одного блока;
- проверка паролей, токенов, секретов и API-ключей;
- предупреждение, если секрет записан прямо в YAML;
- проверка формата ресурсов `cpu/cpus` и `memory/mem_limit`;
- проверка блоков `resources`, `requests`, `reservations` и `limits`;
- итоговый отчет с количеством ошибок и предупреждений.

## Ограничения

Поддерживается не весь стандарт YAML, а простая и понятная часть:

- пары `key: value`;
- вложенные блоки через пробелы;
- списки вида `- item`;
- комментарии через `#`;
- пустые строки.

Сложные многострочные значения и все особенности YAML 1.2 специально не
реализованы, потому что цель проекта - валидатор структуры и безопасности
простых конфигов.

## Установка и сборка

Нужны:

- CMake;
- компилятор с поддержкой C++17;
- Git, если проект скачивается из репозитория.

Скачать проект можно так:

```bash
git clone https://github.com/aleshkar/YamlConfigValidator.git
cd YamlConfigValidator
```

Сборка из терминала:

```bash
cmake -S . -B build
cmake --build build
```

После сборки исполняемый файл будет находиться здесь:

```text
build/YamlConfigValidator
```

## Запуск

Программа принимает путь к YAML-файлу первым аргументом:

```bash
./build/YamlConfigValidator examples/demo_full_valid.yml
```

Вместо `examples/demo_full_valid.yml` можно указать путь к любому своему
`.yaml` или `.yml` файлу.

Пример запуска корректного конфига:

```bash
./build/YamlConfigValidator examples/demo_full_valid.yml
```

Пример запуска конфига с ошибками:

```bash
./build/YamlConfigValidator examples/invalid_config.yaml
```

Пример более крупного production-style конфига:

```bash
./build/YamlConfigValidator examples/demo_real_service.yaml
```

В CLion нужно запускать конфигурацию `YamlConfigValidator`, а в поле
`Program arguments` указать путь к файлу, например:

```text
/Users/aleshkaa/YAML-project/examples/demo_full_valid.yml
```

## Пример корректного конфига

```yaml
app:
  name: payment-gateway
  environment: production
  version: 1.4.2

database:
  host: postgres.internal
  port: 5432
  user: payment_service
  password: ${PAYMENT_DB_PASSWORD}

cache:
  host: redis.internal
  port: 6379
  password: ${REDIS_PASSWORD}

auth:
  token: ${PAYMENT_SERVICE_TOKEN}
  api_key: ${STRIPE_API_KEY}
  private_key: ${JWT_PRIVATE_KEY}

workers:
  - payment-events
  - invoice-sync
  - fraud-check

resources:
  requests:
    cpu: 750m
    memory: 512Mi
  limits:
    cpu: 2
    memory: 1Gi
```

В этом примере секреты берутся из переменных окружения, а лимиты ресурсов
больше или равны запрошенным ресурсам.

Ссылка на переменную окружения должна иметь вид `${DB_PASSWORD}`: имя внутри
скобок не пустое, начинается с буквы или `_`, а дальше содержит только буквы,
цифры и `_`. Поэтому `${APP_TOKEN_2}` считается корректной ссылкой, а
`${1234}` или `${db password}` - нет.

Ожидаемый итог:

```text
Результат: валиден
```

## Пример Docker-like ресурсов

В Kubernetes часто встречается формат `cpu: 500m`, а в Docker Compose похожая
настройка может выглядеть как `cpus: "0.5"`. Поэтому валидатор понимает оба
варианта, если они находятся внутри блока `resources`.

```yaml
services:
  api:
    image: registry.example.com/payment-api:1.4.2
    deploy:
      resources:
        reservations:
          cpus: "0.5"
          memory: 512m
        limits:
          cpus: "1.0"
          memory: 1g
```

Здесь `reservations` используется как Docker-like аналог `requests`, а
`limits` должен быть не меньше него.

## Пример конфига с ошибками

```yaml
database:
  user: admin
  password: 123456

auth:
  token: abc
  api_key: test
  private_key: null
  access_key: true

resources:
  requests:
    cpu: 1000m
    memory: 1Gi
  limits:
    cpu: 500m
    memory: 512Mi
```

Здесь есть несколько проблем:

- пароль слишком простой;
- пароль короче 8 символов;
- пароль состоит только из цифр;
- пароль не содержит английские буквы или спецсимвол;
- токен и API-ключ слишком короткие;
- `private_key` записан как `null`;
- `access_key` записан как `true`;
- секреты записаны прямо в YAML;
- `limits.cpu` меньше `requests.cpu`;
- `limits.memory` меньше `requests.memory`.

Ожидаемый итог:

```text
Результат: невалиден
```

## Пример более реального production-style конфига

В проекте есть файл `examples/demo_real_service.yaml`. Он сделан по мотивам
публичного production-style примера Helm values для Inngest: там есть сервис,
PostgreSQL, Redis, внутренние токены, API-ключи, workers и блок ресурсов.

Источник-ориентир:

```text
https://github.com/inngest/inngest-helm
```

Фрагмент примера:

```yaml
service:
  name: inngest-style-event-platform
  environment: production
  public_url: https://events.example.com

event_api:
  host: 0.0.0.0
  port: 8288
  api_key: ${INNGEST_EVENT_API_KEY}
  token: ${INNGEST_INTERNAL_TOKEN}

postgresql:
  host: postgres.internal
  port: 5432
  database: inngest
  user: inngest_app
  password: ${POSTGRES_PASSWORD}

redis:
  host: redis.internal
  port: 6379
  password: ${REDIS_PASSWORD}

workers:
  - event-ingestion
  - function-runner
  - retry-scheduler
  - metrics-exporter

resources:
  requests:
    cpu: 500m
    memory: 512Mi
  limits:
    cpu: 1000m
    memory: 1Gi
```

Этот пример удобно показывать как более живой конфиг: секреты вынесены в
переменные окружения, ресурсы заполнены корректно, а дерево получается
достаточно большим для демонстрации визуализации.

## Пример вывода

Для корректного файла программа выводит:

```text
Отчет проверки
Проблем не найдено
Ошибки: 0
Предупреждения: 0
Результат: валиден
```

После отчета печатается дерево YAML:

```text
demo_full_valid.yml
|-- app
|   |-- name: payment-service
|   |-- environment: production
|   `-- version: 1.2.0
|-- server
|   |-- host: 0.0.0.0
|   `-- port: 8080
|-- database
|   |-- host: db.internal
|   |-- port: 5432
|   |-- user: payment_user
|   `-- password: ${DB_PASSWORD}
|-- auth
|   |-- token: ${APP_TOKEN}
|   `-- api_key: ${PAYMENT_API_KEY}
|-- services
|   |-- - api
|   |-- - worker
|   `-- - scheduler
|-- logging
|   |-- level: info
|   `-- file: logs/payment.log
`-- resources
    |-- requests
    |   |-- cpu: 500m
    |   `-- memory: 512Mi
    `-- limits
        |-- cpu: 2
        `-- memory: 1Gi
```

Если файл содержит ошибки, отчет выглядит примерно так:

```text
[ОШИБКА] строка 3 | проверка-секретов | секретный ключ 'password' использует слабое значение
[ПРЕДУПРЕЖДЕНИЕ] строка 3 | проверка-секретов | секретный ключ 'password' хранится прямо в YAML; лучше использовать ${ENV_VAR}
[ОШИБКА] строка 15 | проверка-ресурсов | limits.cpu не может быть меньше requests.cpu
Ошибки: 3
Предупреждения: 1
Результат: невалиден
```

## Тесты

Тесты написаны с помощью doctest. Сам файл doctest лежит в папке
`third_party/doctest`, а тесты проекта находятся в `tests/ValidatorTests.cpp`.

Запуск тестов:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`ctest` запускает один тестовый исполняемый файл `ValidatorTests`. Чтобы
увидеть вывод самого doctest подробнее, можно запустить его напрямую:

```bash
./build/ValidatorTests
```

Более подробный вывод со всеми успешными проверками:

```bash
./build/ValidatorTests --success
```

Тестируются:

- разбор YAML-дерева;
- списки и комментарии;
- некорректные отступы;
- вывод дерева;
- проверка синтаксиса;
- проверка реальных файлов;
- проверка секретов;
- проверка ресурсов;
- формирование итогового отчета.

## Документация к коду

Документация к классам и функциям написана в формате Doxygen в заголовочных
файлах `include/*.hpp`.

Сгенерировать HTML-документацию можно командой:

```bash
doxygen Doxyfile
```

Главная страница документации:

```text
docs/html/index.html
```

Открыть ее на macOS можно так:

```bash
open docs/html/index.html
```

## Структура проекта

Основные части проекта:

```text
include/     объявления классов и функций
src/         реализация программы
tests/       doctest-тесты
examples/    примеры YAML-файлов
docs/        HTML-документация Doxygen
third_party/ сторонний файл doctest.h для тестов
```

Главные классы:

- `YamlParser` строит дерево из YAML-файла;
- `TreePrinter` печатает дерево в консоль;
- `SyntaxValidator` проверяет синтаксис и структуру;
- `SecretValidator` проверяет секреты;
- `ResourceValidator` проверяет блоки ресурсов;
- `ValidationReport` хранит ошибки и предупреждения.

Основной запуск идет через `src/main.cpp`: сначала проверяется синтаксис,
потом строится дерево, затем запускаются проверки секретов и ресурсов.

## Сторонний код

Основная программа не использует сторонние библиотеки. Для тестов используется
doctest - один заголовочный файл `doctest.h`, взятый из официального
репозитория doctest и размещенный в `third_party/doctest`.

Источник файла:

```text
https://github.com/doctest/doctest/blob/master/doctest/doctest.h
```

Этот файл не относится к основному функционалу приложения и используется
только для запуска тестов.

## Быстрая демонстрация

Пути для поля `Program arguments` в CLion:

```text
/Users/aleshkaa/YAML-project/examples/demo_full_valid.yml
```

Показывает пункты 1, 3, 4 и 5: дерево YAML, корректные секреты через
`${ENV_VAR}`, корректные `requests/limits`, итоговый отчет без ошибок.

```text
/Users/aleshkaa/YAML-project/examples/demo_real_service.yaml
```

Показывает пункты 1, 3, 4 и 5 на более крупном production-style примере:
сервис, база данных, Redis, workers, секреты через переменные окружения и
корректные ресурсы.

```text
/Users/aleshkaa/YAML-project/examples/demo_docker_resources.yaml
```

Показывает пункты 1, 3, 4 и 5 на Docker Compose-like примере: `deploy`,
`resources`, `reservations`, `limits`, десятичные `cpus` и память в формате
`512m/1g`.

```text
/Users/aleshkaa/YAML-project/examples/demo_syntax_errors.yaml
```

Показывает пункт 2 и 5: ошибки синтаксиса, неправильный отступ, строку без
`:`, пустой ключ и сообщение об ошибке.

```text
/Users/aleshkaa/YAML-project/examples/demo_structure_errors.yaml
```

Показывает пункт 2 и 5: пустые значения, пустой блок и повторяющийся ключ.

```text
/Users/aleshkaa/YAML-project/examples/demo_credentials_errors.yaml
```

Показывает пункт 3 и 5: слабые пароли, пароли без нужных символов, короткие
токены/API-ключи, `null/true` в секретах и предупреждения о хранении секретов
прямо в YAML.

```text
/Users/aleshkaa/YAML-project/examples/demo_resource_errors.yaml
```

Показывает пункт 4 и 5: неправильный формат `cpu/memory`, `limits` меньше
`requests`, Docker-like `cpus` меньше `reservations` и отсутствие блока
`limits`.

## Быстрая проверка требований

Показать, какой стиль clang-format используется:

```bash
cat .clang-format
```

Проверить, что код соответствует clang-format:

```bash
/Applications/CLion.app/Contents/plugins/clion-radler/DotFiles/macos-arm64/clang-format --dry-run --Werror src/*.cpp include/*.hpp tests/*.cpp
```

Проверить, что в коде программы нет абсолютных путей:

```bash
grep -RInE '(/Users/|/home/|[A-Za-z]:[\\/])' src include CMakeLists.txt
```

Проверить, что в коде программы нет путей с обратным слешем `\`:

```bash
grep -RInE '[A-Za-z0-9_.-]+\\[A-Za-z0-9_.-]+' src include CMakeLists.txt
```

Если команды ничего не вывели, значит таких путей в коде нет.

Показать, что путь к YAML-файлу задается пользователем при запуске:

```bash
./cmake-build-debug/YamlConfigValidator /Users/aleshkaa/YAML-project/examples/demo_full_valid.yml
```
