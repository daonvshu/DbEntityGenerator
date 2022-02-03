cmake_minimum_required(VERSION 3.12)
project(dbentitygenerator)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Xml REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/generator)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    file(GLOB entityFiles ${CMAKE_CURRENT_SOURCE_DIR}/test/*.xml)
    file(COPY ${entityFiles} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test)
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp

    src/AbstractGenerator.h
    src/AbstractGenerator.cpp

    src/ConfigLoader.h
    src/ConfigLoader.cpp

    src/Entity.h

    src/Generator.h
    src/Generator.cpp

    src/MySqlGenerator.h
    src/MySqlGenerator.cpp
    src/SqliteGenerator.h
    src/SqliteGenerator.cpp
    src/SqlServerGenerator.h
    src/SqlServerGenerator.cpp

    src/Resource.qrc
)

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Xml)

set(DB_ENTITY_GENERATOR_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "db entity generator executale file path")