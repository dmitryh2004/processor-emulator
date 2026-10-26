cmake_minimum_required (VERSION 3.8)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ProcessorEmulator")

# Включите подпроекты.
add_subdirectory ("ProcessorEmulator")

# --- ИСПРАВЛЕНИЕ: Создаем кастомную цель для копирования файлов ---
# Флаг ALL заставит эту команду выполняться при каждой сборке проекта.
add_custom_target(CopyDocumentation ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Readme.md"
    "$<TARGET_FILE_DIR:ProcessorEmulator>/Readme.md"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/docs"
    "$<TARGET_FILE_DIR:ProcessorEmulator>/docs"
    
    COMMENT "Копирование Readme.md и документации docs в выходную папку..."
)

# Указываем явную зависимость: файлы скопируются только ПОСЛЕ того, 
# как скомпилируется сам эмулятор.
add_dependencies(CopyDocumentation ProcessorEmulator)
