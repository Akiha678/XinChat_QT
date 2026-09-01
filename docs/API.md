# XinChat 后端接口文档

本文档按当前仓库代码整理，面向 IM 服务端的 REST 接口与实时通道。

## 1. 基础信息

- 基础地址：`http://127.0.0.1:8080`
- 鉴权头：`Authorization: Bearer <token>`
- 默认语言：中文

## 2. 响应约定

### 2.1 通用包装

部分接口返回统一包装：

```json
{
  "data": {},
  "code": 1000,
  "message": "success"
}
```

对应类型是 `NetworkResponse<T>`。

### 2.2 错误响应

- `/auth`、`/chat`、`/socket.io` 下的异常，通常会返回 `NetworkResponse.failure(...)`
- 其他 REST 接口通常返回 `ErrorResponse`

### 2.3 鉴权规则

- 除注册、登录、验证码接口外，其余 REST 接口都需要登录态
- 服务端从 `Authorization` 头解析当前用户，不接受客户端手动传入当前用户 ID

---

## 3. 认证模块 `/auth`

### 3.1 注册（邮箱验证码方式）

流程：① 客户端输入邮箱 → ② 调用发送验证码接口 → ③ 服务端向邮箱发送 4 位数字验证码（5 分钟内有效）→ ④ 客户端输入验证码和密码调用注册接口。**无需输入用户名和昵称**，注册成功后由服务端自动生成（用户名 = 邮箱，昵称 = 邮箱 @ 前缀），后续可在应用层面修改。

##### 1. 发送注册验证码

`POST /auth/register/code`

请求体：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| email | string | 是 | 尚未注册的邮箱 |

返回 `NetworkResponse<String>`：

- 配置了 SMTP：`data` 为 `"验证码已发送到邮箱"`
- 未配置 SMTP（开发环境）：`data` 直接返回 4 位数字验证码

失败场景：

| 场景 | code | message |
| --- | --- | --- |
| 邮箱已注册 | 409 | 该邮箱已被注册 |
| 邮箱不存在，SMTP 拒绝投递（如账号不存在被 550 退回） | 400 | 邮箱不存在或无法接收邮件，请检查邮箱地址 |

> 说明：仅当 SMTP 同步拒绝投递时能立即感知邮箱不存在；部分邮件服务器（尤其域名本身不存在的地址）会先接受邮件再异步退信，服务端无法同步感知，此时会返回"验证码已发送到邮箱"，但实际不会收到邮件。

##### 2. 提交注册

`POST /auth/register`

请求体 `RegisterRequest`：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| email | string | 是 | 邮箱 |
| code | string | 是 | 邮箱收到的 4 位数字验证码 |
| password | string | 是 | 密码，6-72 字符 |

成功返回 `LoginResponse`（HTTP 201）：

| 字段 | 说明 |
| --- | --- |
| id | 用户 ID |
| username | 自动生成的用户名（= 邮箱） |
| displayName | 自动生成的昵称（邮箱 @ 前缀） |
| email | 邮箱 |
| avatarColor | 头像色值 |
| accessToken | 登录令牌 |
| expiresAt | 过期时间 |

失败场景：

| 场景 | code | message |
| --- | --- | --- |
| 邮箱已注册 | 409 | 邮箱已被注册 |
| 验证码错误 | 400 | 验证码错误 |
| 验证码过期（5 分钟）或已被使用 | 400 | 验证码已过期，请重新获取 |
| 参数校验失败 | 400 | 对应字段校验提示 |

### 3.2 用户名密码登录

`POST /login`

请求体 `LoginRequest`：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| username | string | 是 | 登录账号，**支持用户名或邮箱**，二者任一均可 |
| password | string | 是 | 密码 |

成功返回 `LoginResponse`。账号不存在或密码错误时统一返回 `401 用户名或密码错误`（不提示具体是哪种错误，避免泄露账号是否存在）。

### 3.3 兼容登录

`POST /auth/login/password`

当前实现接受一个普通 JSON 对象，账号字段可用：

- `username`
- `account`
- `phone`
- `phoneNumber`
- `mobile`

成功返回 `NetworkResponse<AuthResponse>`。

`AuthResponse` 字段：

| 字段 | 说明 |
| --- | --- |
| token | 访问令牌 |
| refreshToken | 当前实现中与 token 相同 |
| expire | 过期秒数 |
| refreshExpire | 当前实现中与 expire 相同 |
| createdAt | 签发时间戳 |

### 3.4 图形验证码（防人机验证）

##### 1. 获取验证码图片

`GET /auth/login/captcha`

返回 `NetworkResponse<CaptchaResponse>`：

| 字段 | 说明 |
| --- | --- |
| data | 验证码图片，`data:image/png;base64,...` 格式，可直接用于 `<img src>` |
| captchaId | 验证码唯一 ID，提交时需回传 |

```json
{
  "data": "data:image/png;base64,iVBORw0KGgo...",
  "captchaId": "3f9c8e2a-...",
  "code": 1000,
  "message": "success"
}
```

验证码为 4 位字符（数字 + 大写字母，已排除 0/O/1/I 易混淆字符），图片含干扰线和噪点；**5 分钟过期、一次性使用**。

##### 2. 校验验证码

`POST /auth/login/captcha/verify`

请求体：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| captchaId | string | 是 | 获取接口返回的验证码 ID |
| code | string | 是 | 用户输入的验证码（不区分大小写） |

返回 `NetworkResponse<Boolean>`，正确时 `data` 为 `true`。

失败场景：

| 场景 | code | message |
| --- | --- | --- |
| 答案错误 | 400 | 图形验证码错误 |
| 已过期或 captchaId 无效（含已使用） | 400 | 图形验证码已过期，请刷新 |

> 前端用法：登录/注册页先调获取接口展示图片，用户输入后随登录/注册请求一起提交 `captchaId` 和输入值，服务端在业务校验前调用 `CaptchaService.verifyAndConsume` 完成人机验证。

`POST /auth/login/smsCode`

请求体支持 `phone` / `phoneNumber` / `mobile` 其中任一字段，返回 `NetworkResponse<String>`，内容为当前生成的验证码。

### 3.5 当前不支持

- `POST /auth/login/phone`
- `POST /auth/login/refreshToken`

这两个接口当前都会返回 400。

---

## 4. 好友模块 `/contact`

### 4.1 搜索用户

`GET /contact/users/search?username=xxx`

返回 `List<UserSummaryResponse>`。

`UserSummaryResponse`：

| 字段 | 说明 |
| --- | --- |
| id | 用户 ID |
| name | 昵称 |
| username | 用户名 |
| email | 邮箱 |
| avatarColor | 头像色值 |

### 4.2 我的好友

`GET /contact/friends`

返回好友列表，结构同 `UserSummaryResponse`。

### 4.3 好友申请列表

`GET /contact/friend-requests/incoming`

`GET /contact/friend-requests/outgoing`

返回 `List<FriendRequestResponse>`。

`FriendRequestResponse`：

| 字段 | 说明 |
| --- | --- |
| id | 申请 ID |
| requester | 申请人 |
| addressee | 被申请人 |
| status | `PENDING` / `ACCEPTED` / `REJECTED` |
| message | 申请留言 |
| createdAt | 创建时间 |

### 4.4 发送好友申请

`POST /contact/friend-requests`

请求体 `CreateFriendRequest`：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| addresseeId | long | 是 | 对方用户 ID |
| message | string | 否 | 申请留言 |

成功返回 `FriendRequestResponse`，状态码 `201`。

### 4.5 处理好友申请

`POST /contact/friend-requests/{requestId}/accept`

`POST /contact/friend-requests/{requestId}/reject`

返回 `FriendRequestResponse`。

---

## 5. 聊天模块 `/chat`

### 5.0 会话列表

`GET /chat/session`

返回 `NetworkResponse<List<ConversationResponse>>`，按最近消息时间倒序。

`ConversationResponse`：

| 字段 | 说明 |
| --- | --- |
| id | 会话 ID |
| peerId | 对端用户 ID |
| name | 会话展示名（单聊为对方昵称） |
| preview | 最近一条消息内容 |
| lastMessageAt | 最近消息时间 |
| unreadCount | 未读数 |
| colorSeed | 头像色值种子 |

### 5.1 创建单聊会话

`POST /chat/conversation`

请求体 `CreateDirectConversationRequest`：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| friendId | long | 是 | 好友用户 ID |

返回 `NetworkResponse<ConversationResponse>`。已存在的会话会直接复用（幂等）。

失败场景：

| 场景 | code | message |
| --- | --- | --- |
| 与自己创建 | 400 | 不能和自己创建单聊 |
| 非好友 | 403 | 只有好友之间可以创建聊天 |
| 用户不存在 | 404 | 用户不存在 |

### 5.2 当前会话

`POST /chat/session`

返回 `NetworkResponse<ChatSessionResponse>`。

`ChatSessionResponse`：

| 字段 | 说明 |
| --- | --- |
| id | 会话 ID |
| userId | 对端用户 ID |
| lastMsg | 最近一条消息 |
| unreadCount | 未读数 |
| nickName | 会话展示名 |
| avatarUrl | 会话头像，当前实现为空串 |
| createTime | 创建时间 |
| updateTime | 更新时间 |

### 5.2 会话消息分页

`POST /chat/message/page`

请求体 `MessagePageRequest`：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| sessionId | long | 会话 ID |
| page | int | 页码，默认 1 |
| size | int | 每页条数，默认 20，最大 100 |

> 消息固定按时间倒序（最新在前），翻页越翻越早。

返回 `NetworkResponse<PageData<MsgResponse>>`。

`PageData`：

| 字段 | 说明 |
| --- | --- |
| list | 当前页数据 |
| pagination | 分页元信息 |

`PageMeta`：

| 字段 | 说明 |
| --- | --- |
| total | 总数 |
| size | 页大小 |
| page | 当前页 |

`MsgResponse`：

| 字段 | 说明 |
| --- | --- |
| id | 消息 ID |
| userId | 发送者 ID |
| sessionId | 会话 ID |
| status | 已读状态，`1` 表示已读 |
| nickName | 发送者昵称 |
| avatarUrl | 发送者头像，当前实现为空串 |
| createTime | 创建时间 |
| content | 消息内容 |
| type | 消息方向，`0` 表示本人发送 |
| updateTime | 更新时间 |

`MessageContentResponse`：

| 字段 | 说明 |
| --- | --- |
| type | 当前固定为 `text` |
| data | 文本内容 |

### 5.3 标记已读

`POST /chat/message/read`

请求体 `ReadMessageRequest`：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| ids | long[] | 需要推进已读游标的消息 ID 列表 |

返回 `NetworkResponse<Boolean>`。

### 5.4 未读总数

`GET /chat/message/unread`

返回 `NetworkResponse<Integer>`。

### 5.5 发送消息

`POST /chat/session/{sessionId}/message`

请求体 `SendMessageRequest`：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| content | string | 文本内容，最长 2000 字符 |

返回 `NetworkResponse<MsgResponse>`。

### 5.6 用户资料与退出登录

推荐使用以下前缀：

- `GET /chat/person`
- `POST /chat/logoff`
- `POST /chat/updatePassword`

兼容别名：

- `GET /user/info/person`
- `POST /user/info/logoff`
- `POST /user/info/updatePassword`
- `POST /auth/updatePassword`（认证模块下的等价入口）

#### 修改密码（邮箱验证码方式，无需登录）

流程：① 客户端输入邮箱 → ② 调用发送验证码接口 → ③ 服务端向邮箱发送 4 位数字验证码（5 分钟内有效）→ ④ 客户端输入验证码和新密码调用修改密码接口。

##### 1. 发送邮箱验证码

`POST /auth/password/code`

请求体：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| email | string | 是 | 已注册的邮箱 |

返回 `NetworkResponse<String>`：

- 配置了 SMTP（`MAIL_HOST` 环境变量）：`data` 为 `"验证码已发送到邮箱"`，验证码只发到邮箱
- 未配置 SMTP（开发环境）：`data` 直接返回 4 位数字验证码，便于本地联调

##### 2. 提交新密码

`POST /auth/updatePassword`（兼容别名：`POST /chat/updatePassword`、`POST /user/info/updatePassword`）

请求体 `UpdatePasswordRequest`：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| email | string | 是 | 已注册的邮箱 |
| code | string | 是 | 邮箱收到的 4 位数字验证码 |
| newPassword | string | 是 | 新密码，6-72 字符，不能与旧密码相同 |

返回 `NetworkResponse<Boolean>`，成功时 `data` 为 `true`：

```json
{
  "data": true,
  "code": 1000,
  "message": "success"
}
```

失败场景：

| 场景 | code | message |
| --- | --- | --- |
| 邮箱未注册 | 400 | 该邮箱未注册 |
| 验证码错误 | 400 | 验证码错误 |
| 验证码过期（5 分钟）或已被使用 | 400 | 验证码已过期，请重新获取 |
| 新密码与旧密码相同 | 400 | 新密码不能与旧密码相同 |
| 参数校验失败 | 400 | 对应字段校验提示 |

> 安全说明：验证码一次性有效，5 分钟过期。修改成功后该用户**所有设备**的会话都会被吊销，需用新密码重新登录。

当前不支持的接口：

- `POST /chat/updatePersonInfo`
- `POST /chat/updatePerson`
- `POST /chat/bindPhone`

这些接口都会返回 400。

`UserResponse` 当前用于返回个人资料，字段偏前端兼容格式：

| 字段 | 说明 |
| --- | --- |
| id | 用户 ID |
| unionid | 当前填充为用户名 |
| avatarUrl | 头像地址，当前为空 |
| nickName | 昵称 |
| phone | 当前填充为用户名 |
| gender | 性别，当前固定为 0 |
| status | 状态，1 表示启用 |
| loginType | 当前固定为 `0` |
| password | 当前为空 |
| createTime | 创建时间 |
| updateTime | 更新时间 |

---

## 6. 实时通道

### 6.1 原生 WebSocket

`WS /ws/chat`

- 连接时携带 `Authorization: Bearer <token>`
- 用于接收服务端事件推送
- 只负责通知，不负责写入消息

### 6.2 Socket.IO 风格通道

`WS /socket.io` 和 `WS /socket.io/`

握手后服务端会发送初始包：

```text
0{"sid":"<sessionId>","upgrades":[],"pingInterval":25000,"pingTimeout":60000}
```

认证消息：

```text
40/chat,{"token":"<token>"}
```

发消息：

```text
42/chat,["send",{"sessionId":1,"content":{"type":"text","data":"hello"}}]
```

服务端回消息：

```text
42/chat,["msg",{...MsgResponse...}]
```

### 6.3 实时事件

`message.created`

```json
{
  "type": "message.created",
  "data": {}
}
```

`friendship.accepted`

```json
{
  "type": "friendship.accepted",
  "data": { "conversationId": 1 }
}
```

`friend.request.changed`

```json
{
  "type": "friend.request.changed",
  "data": { "requestId": 12 }
}
```

`conversation.read`

```json
{
  "type": "conversation.read",
  "data": { "conversationId": 1 }
}
```

---

## 7. 推荐联调用法

1. `POST /auth/register` 或 `POST /auth/login`
2. 使用返回的 `accessToken`
3. `GET /contact/users/search` 搜索好友
4. `POST /contact/friend-requests`
5. 对方同意后 `POST /chat/session`
6. 通过 `/chat/session/{sessionId}/message` 发消息
7. 用 WebSocket 接收实时推送

