cmake_minimum_required(VERSION 3.20)
project(ProcessorEmulator)

# ИСПРАВЛЕНИЕ 1: SFML 3 требует стандарт C++20 или выше! 
# При C++17 сигнатуры функций вроде std::filesystem::path не совпадают.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем путь к SFML 3.1.0
set(SFML_DIR "C:/lib/SFML-3.1.0/lib/cmake/SFML") 

# ИСПРАВЛЕНИЕ 2: СТРОКУ add_compile_definitions(SFML_STATIC) УДАЛИЛИ, так как библиотека динамическая

find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME} "ProcessorEmulator.cpp" "ProcessorEmulator.h" "ResourceManager.h" "ResourceManager.cpp" "SFML/BaseObject.h" "SFML/Button.h" "SFML/Image.h" "SFML/Text.h" "SFML/Panel.h" "SFML/Utilites/SFMLUtility.cpp" "SFML/Utilites/SFMLUtility.h" "SFML/InputField.h")

target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Window SFML::Audio SFML::System)
target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics OpenGL::GL)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )

    # Автоматическое копирование всей папки Assets в выходную директорию
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets"
        COMMENT "Синхронизация папки ресурсов Assets..."
    )
    
    # Автоматическое копирование всей папки SFML/Shaders в выходную директорию
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/SFML/Shaders"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SFML/Shaders"
        COMMENT "Синхронизация папки SFML/Shaders..."
    )
endif()
