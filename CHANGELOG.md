# HuHoBot-EndStone-Adapter v0.1.3

fix(websocket): 修复WebSocket客户端线程安全与稳定性问题
- 修复`select error`报错：`select()`返回错误后不再继续操作已关闭的socket
- `status`改为`std::atomic`，消除主线程与接收线程之间的数据竞争
- 发送操作加`std::mutex`保护，防止多线程发送导致帧交错损坏
- 接收线程不再`detach()`改为`join()`，修复析构时use-after-free
- `Shutdown()`与`RecvLoop`使用原子操作竞争socket关闭权，防止double-close
- 修复payload length 127的字节序解析错误（小端架构）
- mask key改为每帧随机生成，符合RFC 6455规范

fix(client): 修复协议客户端异常处理
- 回调注册移到`Connect()`之前，防止消息在回调设好前到达导致丢失
- `onTextMsg`增加try-catch，非法JSON消息不再导致插件崩溃
- `reconnect()`增加异常处理，连接失败时自动进入重连流程而非崩溃
- `onError`不再直接触发重连，连接断开统一由`onLost`处理

fix(plugin): 修复内存泄漏和其他问题
- `BotClient`改用`std::unique_ptr`管理，修复内存泄漏
- 定时器Lambda捕获`[&]`改为`[this]`，修复异步回调中的悬垂引用
- `ConfigManager`使用单例引用而非拷贝构造

update(lib):
- 适配Endstone 0.10.18

---

# HuHoBot-EndStone-Adapter v0.1.2

update(lib):
- 适配Endstone 0.10.7

feat(config):
- 添加`callbackConvertImg`配置项
