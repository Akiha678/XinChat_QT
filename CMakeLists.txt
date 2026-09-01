cmake_minimum_required(VERSION 3.16)

project(XinChat
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Network
)

qt_add_executable(XinChat
    src/main.cpp

    src/core/models.h
    src/core/Session.h
    src/core/Session.cpp
    src/core/UserManager.h
    src/core/UserManager.cpp

    src/network/ApiClient.h
    src/network/ApiClient.cpp
    src/network/TcpClient.h
    src/network/TcpClient.cpp

    src/ui/login/LoginWindow.h
    src/ui/login/LoginWindow.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp

    src/ui/widgets/MessageBubble.h
    src/ui/widgets/MessageBubble.cpp
    src/ui/widgets/ConversationItem.h
    src/ui/widgets/ConversationItem.cpp
)

target_include_directories(XinChat PRIVATE src)

target_link_libraries(XinChat PRIVATE
    Qt6::Widgets
    Qt6::Network
)
