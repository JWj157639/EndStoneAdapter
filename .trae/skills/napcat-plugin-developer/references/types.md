# 类型导入路径说明

## 插件框架类型

从 `napcat-types/napcat-onebot/network/plugin/types` 导入：

```typescript
import type {
    // 插件上下文 - 提供日志、路由、API调用等
    NapCatPluginContext,
    
    // 插件模块接口 - 定义生命周期函数
    PluginModule,
    
    // 配置 Schema 类型 - 用于 WebUI 配置界面
    PluginConfigSchema,
    
    // 配置 UI 控制器 - 动态控制配置界面
    PluginConfigUIController,
    
    // HTTP 请求类型 - API 路由的请求参数
    PluginHttpRequest,
    
    // HTTP 响应类型 - API 路由的响应对象
    PluginHttpResponse,
} from 'napcat-types/napcat-onebot/network/plugin/types';
```

### NapCatPluginContext

插件上下文，提供核心功能：

```typescript
interface NapCatPluginContext {
    // 日志记录器
    logger: PluginLogger;
    
    // 插件信息
    pluginName: string;
    pluginVersion: string;
    
    // 路由系统
    router: Router;
    
    // API 调用
    actions: Actions;
    
    // 适配器信息
    adapterName: string;
    
    // 插件管理器
    pluginManager: PluginManager;
    
    // 路径
    dataPath: string;      // 数据目录
    configPath: string;    // 配置文件路径
    
    // 配置构建器
    NapCatConfig: NapCatConfigBuilder;
}
```

## OneBot 消息和事件类型

从 `napcat-types/napcat-onebot` 导入：

```typescript
import type {
    // 消息事件基类
    OB11Message,
    
    // 发送消息参数
    OB11PostSendMsg,
    
    // 群消息事件
    OB11GroupMessageEvent,
    
    // 私聊消息事件
    OB11PrivateMessageEvent,
    
    // 消息段类型
    OB11MessageSegment,
    
    // 发送者信息
    OB11SenderInfo,
    
    // 通知事件
    OB11NoticeEvent,
    
    // 请求事件
    OB11RequestEvent,
} from 'napcat-types/napcat-onebot';
```

### OB11Message

消息事件基类：

```typescript
interface OB11Message {
    // 事件类型
    post_type: 'message';
    
    // 消息类型
    message_type: 'group' | 'private';
    
    // 消息 ID
    message_id: number;
    
    // 发送者 QQ
    user_id: number;
    
    // 发送者信息
    sender: OB11SenderInfo;
    
    // 群号（群消息）
    group_id?: number;
    
    // 原始消息内容
    raw_message: string;
    
    // 消息段数组
    message: OB11MessageSegment[];
    
    // 时间戳
    time: number;
    
    // 字体
    font: number;
}
```

### OB11PostSendMsg

发送消息参数：

```typescript
interface OB11PostSendMsg {
    // 消息类型
    message_type: 'group' | 'private';
    
    // 消息内容（消息段数组或字符串）
    message: OB11MessageSegment[] | string;
    
    // 群号（群消息必填）
    group_id?: string;
    
    // 用户 ID（私聊消息必填）
    user_id?: string;
    
    // 消息 ID（回复消息时使用）
    message_id?: number;
    
    // 是否自动提取 URL
    auto_escape?: boolean;
}
```

## 事件类型枚举

从 `napcat-types/napcat-onebot/event/index` 导入：

```typescript
import {
    // 事件类型
    EventType,
    
    // 通知类型
    NoticeType,
    
    // 请求类型
    RequestType,
} from 'napcat-types/napcat-onebot/event/index';
```

### EventType

```typescript
enum EventType {
    MESSAGE = 'message',    // 消息事件
    NOTICE = 'notice',      // 通知事件
    REQUEST = 'request',    // 请求事件
    META_EVENT = 'meta',    // 元事件
}
```

### NoticeType

```typescript
enum NoticeType {
    // 群通知
    GROUP_ADMIN = 'group_admin',              // 群管理员变动
    GROUP_DECREASE = 'group_decrease',        // 群成员减少
    GROUP_INCREASE = 'group_increase',        // 群成员增加
    GROUP_BAN = 'group_ban',                  // 群成员禁言
    GROUP_UPLOAD = 'group_upload',            // 群文件上传
    GROUP_CARD = 'group_card',                // 群名片修改
    GROUP_RECALL = 'group_recall',            // 群消息撤回
    
    // 好友通知
    FRIEND_ADD = 'friend_add',                // 好友添加
    FRIEND_RECALL = 'friend_recall',          // 好友消息撤回
    
    // 其他
    NOTIFY = 'notify',                        // 提醒事件
    POKE = 'poke',                            // 戳一戳
    LUCKY_KING = 'lucky_king',                // 群运气王
    HONOR = 'honor',                          // 群荣誉变更
    TITLE = 'title',                          // 群头衔变更
}
```

### RequestType

```typescript
enum RequestType {
    // 群请求
    GROUP = 'group',                          // 加群请求
    GROUP_INVITE = 'group_invite',            // 群邀请
    
    // 好友请求
    FRIEND = 'friend',                        // 好友请求
}
```

## 常见使用模式

### 判断消息类型

```typescript
import { EventType } from 'napcat-types/napcat-onebot/event/index';
import type { OB11GroupMessageEvent, OB11PrivateMessageEvent } from 'napcat-types/napcat-onebot';

export const plugin_onmessage: PluginModule['plugin_onmessage'] = async (
    ctx: NapCatPluginContext, 
    event: OB11Message
) => {
    // 判断是否为消息事件
    if (event.post_type !== EventType.MESSAGE) return;
    
    // 判断消息类型
    if (event.message_type === 'group') {
        const groupEvent = event as OB11GroupMessageEvent;
        // 处理群消息
    } else if (event.message_type === 'private') {
        const privateEvent = event as OB11PrivateMessageEvent;
        // 处理私聊消息
    }
};
```

### 处理通知事件

```typescript
import { NoticeType } from 'napcat-types/napcat-onebot/event/index';
import type { OB11NoticeEvent } from 'napcat-types/napcat-onebot';

export const plugin_onevent: PluginModule['plugin_onevent'] = async (
    ctx: NapCatPluginContext, 
    event: OB11Message
) => {
    if (event.post_type === EventType.NOTICE) {
        const notice = event as OB11NoticeEvent;
        
        switch (notice.notice_type) {
            case NoticeType.GROUP_INCREASE:
                // 新成员入群
                break;
            case NoticeType.FRIEND_ADD:
                // 好友添加
                break;
            case NoticeType.POKE:
                // 戳一戳
                break;
        }
    }
};
```

### 类型守卫

```typescript
import type { OB11GroupMessageEvent } from 'napcat-types/napcat-onebot';

function isGroupMessage(event: OB11Message): event is OB11GroupMessageEvent {
    return event.message_type === 'group';
}

// 使用
if (isGroupMessage(event)) {
    // TypeScript 知道 event.group_id 存在
    console.log(event.group_id);
}
```

## 类型导入速查表

| 类型分类 | 导入路径 | 主要用途 |
|---------|---------|---------|
| 插件框架 | `napcat-types/napcat-onebot/network/plugin/types` | 插件上下文、路由、配置 |
| 消息类型 | `napcat-types/napcat-onebot` | 消息事件、消息段、发送消息 |
| 事件枚举 | `napcat-types/napcat-onebot/event/index` | 事件类型、通知类型、请求类型 |

## 常见错误

### ❌ 错误：自己定义类型

```typescript
// 不要这样做
interface OB11Message {
    post_type: string;
    message_type: string;
    // ...
}
```

### ✅ 正确：从 napcat-types 导入

```typescript
// 正确做法
import type { OB11Message } from 'napcat-types/napcat-onebot';
```

### ❌ 错误：从错误路径导入

```typescript
// 不要这样做
import type { OB11Message } from 'napcat-types';
```

### ✅ 正确：使用完整路径

```typescript
// 正确做法
import type { OB11Message } from 'napcat-types/napcat-onebot';
```