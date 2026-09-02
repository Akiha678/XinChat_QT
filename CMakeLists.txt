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
    WebSockets
)

qt_add_executable(XinChat
    src/main.cpp

    src/core/models.h
    src/core/Format.h
    src/core/Session.h
    src/core/Session.cpp
    src/core/ChatManager.h
    src/core/ChatManager.cpp

    src/network/ApiClient.h
    src/network/ApiClient.cpp
    src/network/WsClient.h
    src/network/WsClient.cpp
    src/network/TcpClient.h
    src/network/TcpClient.cpp

    src/ui/login/LoginWindow.h
    src/ui/login/LoginWindow.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp

    src/ui/components/button/button.h
    src/ui/components/button/button.cpp
    src/ui/components/dialog/dialog.h
    src/ui/components/dialog/dialog.cpp
    src/ui/components/messagebubble/MessageBubble.h
    src/ui/components/messagebubble/MessageBubble.cpp
    src/ui/components/conversationitem/ConversationItem.h
    src/ui/components/conversationitem/ConversationItem.cpp

    # 第三方库：QR Code 生成器（Nayuki，MIT 协议，无依赖）
    src/third_party/qrcodegen/qrcodegen.hpp
    src/third_party/qrcodegen/qrcodegen.cpp
)

target_include_directories(XinChat PRIVATE src)

target_link_libraries(XinChat PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
)
