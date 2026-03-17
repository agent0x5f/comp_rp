cmake_minimum_required(VERSION 3.20)
project(AppVisionCenidet)

# 1. Estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Rutas para Mac M4 (Homebrew)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

# --- 3. CONFIGURAR LIBRERÍAS ---

# wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# OpenCV
find_package(OpenCV REQUIRED)

# --- 4. LISTAR ARCHIVOS DEL PROYECTO ---
set(SOURCE_FILES
        main.cpp
        GUI.cpp
        GUI.h
        maxmin.cpp
        maxmin.h
        graficador.cpp
        graficador.h
        io.cpp
        io.h
        chainmap.cpp
        chainmap.h
        resource.rc
        kmeans.cpp
        kmeans.h
        isodata.cpp
        isodata.h
        dbscan.cpp
        dbscan.h
)

# --- 5. CREAR EL EJECUTABLE ---
set(RESOURCE_FILES "resource.rc")  #v6 manifest
add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})

# --- 6. INCLUDES Y LINKER ---

# Usamos SYSTEM para que el compilador ignore advertencias dentro de OpenCV/wxWidgets
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${OpenCV_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${wxWidgets_INCLUDE_DIRS})

# Enlace de librerías
target_link_libraries(${PROJECT_NAME}
        ${wxWidgets_LIBRARIES}
        ${OpenCV_LIBS}
)

# --- 7. BANDERAS DE ADVERTENCIA ESTRICTAS (MODO EXPERTO) ---

if(MSVC)
    # Configuración para GitHub Actions (Windows)
    # No activamos /WX (Werror) aquí para evitar que el CI falle por avisos menores
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    # Configuración para CLion (Mac M4)
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror      # Advertencias se vuelven errores
            -Wno-unused-parameter # Opcional: silencia parámetros de eventos no usados
    )
endif()