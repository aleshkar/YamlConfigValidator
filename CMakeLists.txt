cmake_minimum_required(VERSION 3.16)
project(YamlConfigValidator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(yaml_validator_core
        src/YamlNode.cpp
        src/YamlParser.cpp
        src/TreePrinter.cpp
        src/YamlTextUtils.cpp
        src/ValidationIssue.cpp
        src/ValidationReport.cpp
        src/SyntaxValidator.cpp
        src/SecretValidator.cpp
        src/ResourceValidator.cpp)

target_include_directories(yaml_validator_core PUBLIC include)

add_executable(YamlConfigValidator src/main.cpp)
target_link_libraries(YamlConfigValidator PRIVATE yaml_validator_core)

enable_testing()

add_executable(ValidatorTests tests/ValidatorTests.cpp)
target_link_libraries(ValidatorTests PRIVATE yaml_validator_core)
target_include_directories(ValidatorTests PRIVATE third_party)

add_test(NAME ValidatorTests COMMAND ValidatorTests)
