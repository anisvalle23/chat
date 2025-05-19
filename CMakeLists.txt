cmake_minimum_required(VERSION 3.16)

project(chat VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

# =======================
# ARCHIVOS COMPARTIDOS
# =======================
set(COMMON_SOURCES
    usuario.h usuario.cpp
    listaenlazada.h listaenlazada.cpp
    pila_template.h
    mensaje_guardado.h
    cola.h
    servidor_socket.h servidor_socket.cpp
    cliente_socket.h cliente_socket.cpp
)

# =======================
# EJECUTABLE CLIENTE (GUI)
# =======================
set(CLIENT_SOURCES
    main.cpp
    loginwindow.h loginwindow.cpp loginwindow.ui
    chatwindow.h chatwindow.cpp
    chatscreen.h chatscreen.cpp
    agregarcontactodialog.h agregarcontactodialog.cpp
    gestorcontactos.h gestorcontactos.cpp
    stickers.qrc
)

add_executable(chat
    ${CLIENT_SOURCES}
    ${COMMON_SOURCES}
    PasswordLineEdit.h PasswordLineEdit.cpp
    StyledComboBox.h
    gestorvistachat.h gestorvistachat.cpp
    gestornoleidos.h gestornoleidos.cpp
    ordenadorcontacto.h ordenadorcontacto.cpp
    gestornotificaciones.h gestornotificaciones.cpp

)

target_link_libraries(chat PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# =======================
# EJECUTABLE SERVIDOR
# =======================
add_executable(servidor
    main_servidor.cpp
    ${COMMON_SOURCES}
)

target_link_libraries(servidor PRIVATE Qt${QT_VERSION_MAJOR}::Network)

# =======================
# PROPIEDADES OPCIONALES (macOS / Windows)
# =======================
set_target_properties(chat PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.chat
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS chat
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
