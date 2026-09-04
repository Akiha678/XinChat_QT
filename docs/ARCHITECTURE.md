# XinChat 客户端架构

XinChat 采用面向功能域的分层架构。当前实现以 Qt Widgets 为表现层，以应用编排层统一管理窗口生命周期，以领域管理器承载业务状态，以网络客户端承载 REST/WebSocket 通信。

## 1. 分层关系

```text
Application / Bootstrap
  ApplicationController
        │ signals / commands
Presentation
  LoginWindow / MainWindow / components
        │ manager API
Domain / Application services
  ChatManager / ContactManager / Session
        │ DTO + signals
Infrastructure
  ApiClient / WsClient / TcpClient
```

依赖只能由上层指向下层：UI → Manager → Network。网络层不能反向依赖 UI，业务管理器不能依赖具体 QWidget。

## 2. 当前目录职责

```text
src/
├── app/
│   └── ApplicationController.*      # 应用启动、登录态切换、窗口生命周期
├── core/
│   ├── models.h                     # 跨层 DTO/领域数据结构
│   ├── Session.*                    # 当前登录态和用户信息
│   ├── ChatManager.*                # 会话、消息、实时消息业务状态
│   └── ContactManager.*             # 好友列表、用户搜索、好友申请
├── network/
│   ├── ApiClient.*                  # REST 传输与响应 DTO 解析
│   ├── WsClient.*                   # 实时消息通道
│   └── TcpClient.*                  # 预留的 TCP 传输适配器
├── ui/
│   ├── login/LoginWindow.*           # 登录表现层
│   ├── MainWindow.*                  # 主窗口壳和页面组合
│   ├── color/Theme.*                 # 主题系统
│   └── components/                  # 可复用 UI 组件
└── third_party/                     # 外部依赖
```

## 3. 关键生命周期

1. `ApplicationController` 创建登录窗口和主窗口，并监听双方信号。
2. 登录成功后，控制器展示主窗口；主窗口通过 `ChatManager` 和 `ContactManager` 加载业务数据。
3. 退出登录由主窗口发出 `logoutRequested`，控制器清理 `Session`、停止 WebSocket、清空内存缓存，再显示登录窗口。
4. 网络响应进入对应 Manager 前应验证登录态，避免退出后的异步响应污染下一次登录。

## 4. 扩展约定

- 新增功能先创建独立的 `*Manager`（例如 `GroupManager`、`FileTransferManager`），不要继续向 `ChatManager` 堆叠职责。
- 新增 REST 接口集中放在 `ApiClient`，由 Manager 转换成面向 UI 的信号和状态。
- UI 页面只负责布局、输入校验和展示，不持有网络请求细节。
- 跨层传递使用 `models.h` 中的明确 DTO，禁止把 `QNetworkReply` 传入 UI 或领域层。
- 本地持久化接入时新增 `repository/` 层，优先定义接口，再提供 SQLite 实现；不要让 Manager 直接操作 SQL。
- 复杂页面继续拆为 `page/`、`viewmodel/`、`components/`，避免 `MainWindow` 成为新的业务上帝对象。

## 5. 后续演进路线

1. 将 `ApiClient` 的单例替换为构造注入的 `IApiTransport`，便于单元测试和多环境切换。
2. 增加 `data/repository`：SQLite 消息缓存、会话快照和离线队列。
3. 为 Manager 增加请求状态（Idle/Loading/Success/Failure）和统一错误模型。
4. 按功能拆分主窗口页面，并为聊天、联系人、认证分别增加测试夹具。
