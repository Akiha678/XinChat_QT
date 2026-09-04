# XinChat

基于 Qt 6 (C++17) 的类微信 IM 通信软件（macOS）。

当前阶段：**客户端业务版本** —— 登录窗口 + 三栏主界面（导航栏 / 会话列表 / 聊天区），
已接入真实登录、聊天、WebSocket 实时消息、好友搜索与好友申请。

---

## 一、环境要求

| 依赖 | 版本 | 说明 |
|---|---|---|
| macOS | 14.0+ | 与 Homebrew Qt 6.9 的最低系统要求一致 |
| Qt 6 | 6.9.x | 通过 Homebrew 安装：`brew install qt` |
| CMake | 3.16+ | `brew install cmake` |
| 编译器 | Apple Clang | 随 Xcode / CommandLineTools 提供 |

> 注意：本机 `PATH` 中 `/opt/anaconda3/bin/qmake` 会遮蔽 Homebrew 的 qmake
> （`which qmake` 指向 anaconda）。本项目使用 CMake 构建，不受影响，
> 但手动执行 `qmake` 时请使用完整路径 `/opt/homebrew/opt/qt/bin/qmake`。

## 二、构建

项目使用 CMake Presets 管理配置（见 `CMakePresets.json`），一条命令完成配置 + 编译：

```bash
# 配置（生成 build/macos-debug/ 与 compile_commands.json）
cmake --preset macos-debug

# 编译
cmake --build build/macos-debug
```

编译产物：`build/macos-debug/XinChat`

## 三、运行

> 登录功能依赖后端服务（`http://127.0.0.1:8080`），请先启动 XinChat 后端
> （位于 `/Users/akiha/SpringBoot/xinchat`，`./mvnw spring-boot:run`）。

```bash
./build/macos-debug/XinChat
```

使用流程：

1. 登录窗口输入账号密码，点击「登录」或回车提交（调用后端 `POST /auth/login`）
   - 测试账号（后端种子数据）：`admin1 / 123456`
   - 后端未启动时提示"无法连接服务器"；账号或密码错误时提示服务端返回的错误信息
2. 登录成功后进入主窗口：
   - 左侧导航：消息 / 通讯录 / 设置
   - 中间点击会话切换聊天对象
   - 底部输入文字，点「发送」或按 `Ctrl+Enter` 发送
   - 对方会自动回复（演示逻辑，模拟真实收发链路）

> 后端地址可用环境变量覆盖：`XINCHAT_API_BASE=http://192.168.x.x:8080 ./build/macos-debug/XinChat`

## 四、VSCode 使用（消除 IntelliSense 红波浪线）

1. 已配置 `.vscode/settings.json`，让 C/C++ 扩展读取
   `build/macos-debug/compile_commands.json` 获取 Qt 头文件路径
2. 首次克隆/拉取代码后，先执行一次构建生成编译数据库：
   ```bash
   cmake --preset macos-debug && cmake --build build/macos-debug
   ```
3. 若仍有红波浪线：`Cmd+Shift+P` → `Developer: Reload Window`，
   还不行就运行 `C/C++: Reset IntelliSense Database`
4. 安装微软 C/C++ 扩展与 CMake Tools 扩展后，也可直接点击状态栏的
   Preset（macos-debug）一键配置构建

## 五、常见问题

### 1. `find_package(Qt6)` 找不到 Qt / 配置失败

`CMakePresets.json` 中 `CMAKE_PREFIX_PATH` 必须指向 Qt 的**安装前缀**，
而不是 qmake 可执行文件：

```json
"CMAKE_PREFIX_PATH": "/opt/homebrew/opt/qt"
```

### 2. 链接报错 `ld: framework 'AGL' not found`

Qt 6 Widgets 在 macOS 上会链接遗留的 `AGL.framework`，而 Xcode 26 的 SDK
已移除该框架。解决方案：使用仍包含 AGL 的旧 SDK 作为 sysroot：

```json
"CMAKE_OSX_SYSROOT": "/Library/Developer/CommandLineTools/SDKs/MacOSX15.sdk",
"CMAKE_OSX_DEPLOYMENT_TARGET": "14.0"
```

### 3. 更改源码后如何重新构建

```bash
cmake --build build/macos-debug
```

CMake 会自动重编译变更的文件；新增/删除源文件后需要重新执行
`cmake --preset macos-debug` 刷新工程。

## 六、项目结构

```
src/
├── main.cpp                        # 入口：创建 QApplication 与 ApplicationController
├── app/                            # 应用编排层
│   └── ApplicationController.*      # 登录/退出与窗口生命周期
├── core/                           # 领域与应用服务层（不依赖界面）
│   ├── models.h                    # ChatMessage / Conversation / UserInfo 等数据结构
│   ├── Format.h                    # 时间解析/格式化 + 头像色板工具
│   ├── Session.h/.cpp              # 全局会话（登录用户信息 + accessToken）
│   ├── ChatManager.h/.cpp          # 会话/消息/发送/实时接收
│   └── ContactManager.h/.cpp       # 好友列表/搜索/好友申请
├── network/                        # 网络层
│   ├── ApiClient.h/.cpp            # REST 客户端（登录/会话/消息，自动携带 Bearer token）
│   └── WsClient.h/.cpp             # WebSocket 实时通道（/ws/chat，message.created 推送）
└── ui/                             # 界面层
    ├── login/LoginWindow.h/.cpp    # 登录窗口（账号登录 + 二维码展示）
    ├── MainWindow.h/.cpp           # 三栏主窗口
    ├── color/Theme.h/.cpp          # 主题组件：语义化色板（浅色/深色）+ 全局 QSS 生成
    └── components/                 # 可复用 UI 组件库（每个组件一个文件夹）
        ├── button/button.h/.cpp    # 主按钮组件（微信绿 + 加载态）
        ├── dialog/dialog.h/.cpp    # 弹窗工具组件（统一提示/确认样式）
        ├── messagebubble/          # 聊天气泡（自绘）
        └── conversationitem/       # 会话列表项（自绘 + 未读角标）

third_party/qrcodegen/              # 二维码生成库（Nayuki，MIT，无依赖）
```

## 七、开发路线

| 阶段 | 内容 | 状态 |
|---|---|---|
| 1 | 项目结构重构 + 登录/主窗口骨架 | ✅ 完成 |
| 2 | 三栏主界面 + 消息气泡 | ✅ 完成 |
| 3 | 真实登录（对接后端 POST /auth/login） | ✅ 完成 |
| 4 | 会话/消息接入：真实会话列表、历史消息、发送、WebSocket 实时接收 | ✅ 完成 |
| 5 | SQLite 本地缓存（消息历史持久化、离线展示） | ⬜ 待做 |
| 6 | 好友管理（列表、搜索、好友申请、创建单聊） | ✅ 完成 |
| 7 | 功能迭代（未读同步、文件传输、群聊等） | ⬜ 待做 |
