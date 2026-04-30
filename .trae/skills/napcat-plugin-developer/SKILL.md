---
name: napcat-plugin-developer
description: NapCat QQ机器人插件开发助手。提供完整的插件开发工作流，包括项目结构规范、类型导入、生命周期管理、配置系统、消息处理、API开发、WebUI构建和权限系统。使用场景：创建新插件、添加功能模块、开发WebUI、实现权限校验、处理消息事件、管理数据持久化、构建部署插件。所有代码示例使用正确的napcat-types导入路径，确保与官方框架完全兼容。
---

# NapCat 插件开发

## 快速开始

### 项目结构

```
napcat-plugin-{name}/
├── src/
│   ├── index.ts              # 插件主入口
│   ├── config.ts             # 配置定义
│   ├── types.ts              # 类型定义
│   ├── core/                 # 核心模块
│   │   ├── state.ts          # 全局状态管理
│   │   ├── command-registry.ts   # 指令注册表
│   │   └── permission-checker.ts # 权限校验
│   ├── handlers/             # 消息/事件处理
│   ├── modules/              # 功能模块
│   ├── services/             # API服务
│   └── webui/                # WebUI前端
├── package.json
├── tsconfig.json
└── vite.config.ts
```

### 基础插件模板

```typescript
// src/index.ts
import type {
    PluginModule,
    NapCatPluginContext,
} from 'napcat-types/napcat-onebot/network/plugin/types';
import { EventType } from 'napcat-types/napcat-onebot/event/index';
import type { OB11Message } from 'napcat-types/napcat-onebot';

export const plugin_init: PluginModule['plugin_init'] = async (ctx: NapCatPluginContext) => {
    ctx.logger.info('插件初始化');
};

export const plugin_onmessage: PluginModule['plugin_onmessage'] = async (
    ctx: NapCatPluginContext, 
    event: OB11Message
) => {
    if (event.post_type !== EventType.MESSAGE) return;
    // 处理消息
};
```

## 核心开发规范

### 类型导入（重要）

所有 NapCat 类型必须从 `napcat-types` 导入：

```typescript
// 插件框架类型
import type {
    NapCatPluginContext,
    PluginModule,
    PluginConfigSchema,
    PluginHttpRequest,
    PluginHttpResponse,
} from 'napcat-types/napcat-onebot/network/plugin/types';

// 消息类型
import type {
    OB11Message,
    OB11PostSendMsg,
    OB11GroupMessageEvent,
    OB11PrivateMessageEvent,
} from 'napcat-types/napcat-onebot';

// 事件枚举
import {
    EventType,
    NoticeType,
    RequestType,
} from 'napcat-types/napcat-onebot/event/index';
```

完整类型参考：[类型导入路径说明](references/types.md)

### 配置系统

```typescript
import type { NapCatPluginContext, PluginConfigSchema } from 'napcat-types/napcat-onebot/network/plugin/types';

export function buildConfigSchema(ctx: NapCatPluginContext): PluginConfigSchema {
    return ctx.NapCatConfig.combine(
        ctx.NapCatConfig.boolean('enabled', '启用插件', true, '全局开关'),
        ctx.NapCatConfig.text('prefix', '命令前缀', '#cmd', '触发前缀'),
        ctx.NapCatConfig.number('cooldown', '冷却时间', 60, '秒数，0为不限制'),
        ctx.NapCatConfig.select('mode', '模式', [
            { label: '简单', value: 'simple' },
            { label: '高级', value: 'advanced' }
        ], 'simple', '运行模式'),
    );
}
```

详细配置选项：[配置系统详解](references/config.md)

### 调试与日志

如果日志需要在控制台输出就需要使用 `ctx.logger` 输出日志，日志会自动带上插件名称前缀，方便排查问题。

`ctx.logger` 实现了 PluginLogger 接口，提供以下方法：

```typescript
ctx.logger.log("普通日志");     // 通用日志输出
ctx.logger.debug("调试信息");   // 仅在 debug 模式显示
ctx.logger.info("普通信息");    // 信息级别
ctx.logger.warn("警告信息");    // 警告级别
ctx.logger.error("错误信息", new Error("oops")); // 错误级别
```

在开发过程中，建议开启 `debug: true` 配置以便通过控制台查看详细的调试日志。

### 消息发送

```typescript
// 群消息
await ctx.actions.call('send_group_msg', {
    group_id: '123456',
    message: [{ type: 'text', data: { text: 'Hello' } }]
}, ctx.adapterName, ctx.pluginManager.config);

// 私聊消息
await ctx.actions.call('send_private_msg', {
    user_id: '123456',
    message: [{ type: 'text', data: { text: 'Hello' } }]
}, ctx.adapterName, ctx.pluginManager.config);
```

消息段类型：[消息段完整列表](references/message-segments.md)

### API 路由

```typescript
import type {
    NapCatPluginContext,
    PluginHttpRequest,
    PluginHttpResponse
} from 'napcat-types/napcat-onebot/network/plugin/types';

// 无鉴权 GET
router.getNoAuth('/status', (_req, res) => {
    res.json({ code: 0, data: { uptime: Date.now() } });
});

// 无鉴权 POST
router.postNoAuth('/config', (req, res) => {
    const body = req.body as Record<string, unknown>;
    // 处理数据
    res.json({ code: 0, message: 'ok' });
});

// 路径参数
router.getNoAuth('/users/:id', (req, res) => {
    const userId = req.params?.id;
    res.json({ code: 0, data: { userId } });
});
```

API 开发详解：[API 路由完整指南](references/api.md)

## 常见任务

### 创建功能模块

1. 创建 `src/modules/xxx.module.ts`
2. 定义指令处理器
3. 在 `src/modules/index.ts` 导出

```typescript
export const myCommand: CommandMetadata = {
    name: 'test',
    pattern: /^test$/,
    level: CommandLevel.USER,
    module: 'test',
    description: '测试命令',
    handler: async (message, event, ctx, parsed) => {
        await ctx.actions.call('send_msg', {
            message_type: event.message_type,
            group_id: event.group_id ? String(event.group_id) : undefined,
            message: [{ type: 'text', data: { text: 'OK' } }]
        }, ctx.adapterName, ctx.pluginManager.config);
    },
    requireAuth: true
};
```

模块开发：[模块开发完整指南](references/modules.md)

### 权限检查

```typescript
import { checkUserPermission } from '../core/permission-checker';

const result = checkUserPermission(
    userId,          // 用户QQ号
    'commandName',   // 指令名称
    groupId,         // 群号（可选）
    'group'          // 消息类型
);

if (result.allow) {
    // 执行操作
} else {
    // 拒绝，显示 result.reason
}
```

权限系统：[权限系统详解](references/permissions.md)

### 数据管理

```typescript
// 读取数据
const data = pluginState.loadDataFile('data.json', { key: 'default' });

// 保存数据
pluginState.saveDataFile('data.json', { key: 'value' });

// 更新配置
pluginState.updateConfig({ enabled: true });

// 检查群是否启用
if (pluginState.isGroupEnabled(groupId)) { }
```

数据管理：[数据持久化完整指南](references/data.md)

### WebUI 开发

```typescript
// src/webui/src/utils/api.ts
export async function noAuthFetch<T>(path: string): Promise<ApiResponse<T>> {
    const res = await fetch(`/plugin/${PLUGIN_NAME}/api${path}`, {
        headers: { 'Content-Type': 'application/json' }
    });
    return res.json();
}

// 使用
const { data } = await noAuthFetch<MyType>('/status');
```

WebUI 开发：[WebUI 完整指南](references/webui.md)

## 构建与部署

```bash
# 安装依赖
pnpm install

# 构建 WebUI
cd src/webui && pnpm install && pnpm run build && cd ../..

# 开发模式
pnpm run dev

# 生产构建
pnpm run build
```

构建详解：[构建与部署完整指南](references/build.md)

## 详细参考

- [类型导入路径说明](references/types.md) - 完整的类型导入路径和常用类型列表
- [配置系统详解](references/config.md) - 配置 Schema、所有配置选项、动态配置
- [消息段完整列表](references/message-segments.md) - 所有消息段类型和使用示例
- [API 路由完整指南](references/api.md) - REST API、WebSocket、鉴权、错误处理
- [模块开发完整指南](references/modules.md) - 创建模块、指令注册、模块组织
- [权限系统详解](references/permissions.md) - 三层权限校验、角色管理、白名单
- [数据持久化完整指南](references/data.md) - 配置保存、数据文件读写、状态管理
- [WebUI 完整指南](references/webui.md) - React开发、TailwindCSS、构建配置
- [构建与部署完整指南](references/build.md) - 开发环境、生产构建、热重载、调试

## 最佳实践

1. **类型安全** - 所有 NapCat 类型从 `napcat-types` 导入，不要自己定义
2. **错误处理** - 所有异步操作都要 try-catch，记录错误日志
3. **资源清理** - 在 `plugin_cleanup` 中清理定时器、连接等
4. **日志记录** - 使用 `ctx.logger` 记录重要操作和错误
5. **数据持久化** - 修改配置或数据后及时保存
6. **权限校验** - 敏感操作前进行权限检查
7. **模块化** - 保持代码模块化，每个文件职责单一
8. **类型注解** - 为函数参数和返回值添加类型注解