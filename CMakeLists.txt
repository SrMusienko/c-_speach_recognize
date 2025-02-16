cmake_minimum_required(VERSION 3.16)
project(pro)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем автоматическую обработку Qt MOC, UIC и RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Находим необходимые компоненты Qt6
find_package(Qt6 COMPONENTS 
    Core
    Widgets
    Multimedia
    REQUIRED
)

# Указываем путь к заголовочным файлам, включая vosk_api.h
include_directories(
    "C:/Qt/6.8.2/mingw_64/include"
    "C:/llama/c++"
    "C:/llama/c++/vosk-api"  # Путь до папки, содержащей vosk_api.h
)

# Указываем исходные файлы проекта
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

# Создаём исполнимый файл
add_executable(pro ${PROJECT_SOURCES})

# Линкуем необходимые библиотеки Qt6
target_link_libraries(pro PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
)

# Линкуем библиотеку vosk
target_link_libraries(pro PRIVATE "C:/llama/c++/libvosk.a")

# Если используется MinGW, добавляем флаг для обработки исключений
if(MINGW)
    target_compile_options(pro PRIVATE -fexceptions)
endif()

# Если компилируем на Windows, копируем необходимые DLL файлы Qt в директорию с исполняемым файлом
if(WIN32)
    set(DEBUG_SUFFIX)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif()

    # Находим путь установки Qt
    set(QT_INSTALL_PATH "${Qt6_DIR}/../../../")
    if(NOT EXISTS "${QT_INSTALL_PATH}")
        set(QT_INSTALL_PATH "${Qt6_DIR}/../../")
    endif()

    if(EXISTS "${QT_INSTALL_PATH}")
        add_custom_command(TARGET pro POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_INSTALL_PATH}/bin/Qt6Core${DEBUG_SUFFIX}.dll"
                "${QT_INSTALL_PATH}/bin/Qt6Widgets${DEBUG_SUFFIX}.dll"
                "${QT_INSTALL_PATH}/bin/Qt6Gui${DEBUG_SUFFIX}.dll"
                "${QT_INSTALL_PATH}/bin/Qt6Multimedia${DEBUG_SUFFIX}.dll"
                $<TARGET_FILE_DIR:pro>
        )
    endif()
endif()
