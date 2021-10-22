cmake_minimum_required(VERSION 3.12)
project(dbentitygenerator)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Xml REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/generator)

add_executable(${PROJECT_NAME}
    main.cpp

    AbstractGenerator.h
    AbstractGenerator.cpp

    ConfigLoader.h
    ConfigLoader.cpp

    Entity.h

    Generator.h
    Generator.cpp

    MySqlGenerator.h
    MySqlGenerator.cpp
    SqliteGenerator.h
    SqliteGenerator.cpp
    SqlServerGenerator.h
    SqlServerGenerator.cpp

    Resource.qrc
)

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Xml)

set(DB_ENTITY_GENERATOR_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "db entity generator executale file path")