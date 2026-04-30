# 模块开发完整指南

## 模块结构

### 标准模块文件

```
src/modules/
├── myfeature.module.ts      # 模块实现
├── myfeature.commands.ts    # 指令定义（可选）
└── index.ts                 # 模块统一入口
```

### 模块入口

```typescript
// src/modules/index.ts
export * from './myfeature.module';
export * from './otherfeature.module';
```

## 指令定义

### CommandMetadata 接口

```typescript
import type { CommandMetadata, CommandLevel, CommandHandler } from '../core/command-registry';

const handler: CommandHandler = async (
    message: string,                    // 原始消息内容
    event: OB11Message,                  // 消息事件对象
    ctx: NapCatPluginContext,           // 插件上下文
    parsed: Record<string, string>      // 解析后的参数
) => {
    // 业务逻辑
};
```

### 简单指令

```typescript
export const myCommand: CommandMetadata = {
    name: 'test',                       // 指令名称（唯一标识）
    pattern: /^test$/,                  // 匹配模式（正则表达式）
    level: CommandLevel.USER,          // 权限等级
    module: 'test',                     // 所属模块
    description: '测试命令',            // 指令描述
    handler,                            // 处理函数
    requireAuth: true,                  // 是否需要卡密授权
    requireWhitelist: true,             // 是否需要白名单
};
```

### 带参数的指令

```typescript
const handler: CommandHandler = async (message, event, ctx, parsed) => {
    const keyword = parsed.group0;  // 第一个捕获组
    const option = parsed.group1;   // 第二个捕获组
    
    await ctx.actions.call('send_msg', {
        message_type: event.message_type,
        group_id: event.group_id ? String(event.group_id) : undefined,
        message: [{
            type: 'text',
            data: { text: `关键词：${keyword}，选项：${option}` }
        }]
    }, ctx.adapterName, ctx.pluginManager.config);
};

export const searchCommand: CommandMetadata = {
    name: 'search',
    pattern: /^搜索\s+(.+?)\s+(.+)$/,  // 正则匹配：搜索 <关键词> <选项>
    level: CommandLevel.USER,
    module: 'search',
    description: '搜索功能',
    handler,
    requireAuth: true
};
```

### 多个指令

```typescript
// src/modules/commands.ts
export const commands: CommandMetadata[] = [
    {
        name: 'feature1',
        pattern: /^功能1$/,
        level: CommandLevel.USER,
        module: 'myfeature',
        description: '功能1描述',
        handler: async (message, event, ctx, parsed) => {
            // 功能1逻辑
        },
        requireAuth: true
    },
    {
        name: 'feature2',
        pattern: /^功能2$/,
        level: CommandLevel.USER,
        module: 'myfeature',
        description: '功能2描述',
        handler: async (message, event, ctx, parsed) => {
            // 功能2逻辑
        },
        requireAuth: true
    },
    {
        name: 'feature3',
        pattern: /^功能3\s+(.+)$/,
        level: CommandLevel.OWNER,
        module: 'myfeature',
        description: '功能3（仅主人）',
        handler: async (message, event, ctx, parsed) => {
            const param = parsed.group0;
            // 功能3逻辑
        },
        requireAuth: false,  // 主人无需卡密
        requireWhitelist: false
    }
];
```

## 完整模块示例

### 授权系统模块

```typescript
// src/modules/auth.module.ts
import type { CommandMetadata } from '../core/command-registry';
import type { OB11Message, NapCatPluginContext } from 'napcat-types/napcat-onebot';
import type { NapCatPluginContext as PluginContext } from 'napcat-types/napcat-onebot/network/plugin/types';
import { pluginState } from '../core/state';
import { sendReplyMessage } from '../handlers/message-utils';

// 处理授权判断
async function handleAuthCheck(
    message: string,
    event: OB11Message,
    ctx: PluginContext,
    parsed: Record<string, string>
): Promise<void> {
    const userId = String(event.user_id);
    const groupId = event.group_id ? String(event.group_id) : '私聊';
    
    // 检查授权状态
    const authInfo = pluginState.loadDataFile(
        `筱筱吖/授权系统/授权信息/${groupId}.json`,
        userId,
        null
    );
    
    if (authInfo) {
        await sendReplyMessage(ctx, event, `✅ 已授权\n类型：${authInfo.type}\n到期：${new Date(authInfo.expireTime * 1000).toLocaleString()}`);
    } else {
        await sendReplyMessage(ctx, event, `❌ 未授权\n请联系管理员获取卡密`);
    }
}

// 处理生成卡密
async function handleGenerateCard(
    message: string,
    event: OB11Message,
    ctx: PluginContext,
    parsed: Record<string, string>
): Promise<void> {
    const cardType = parsed.group0; // 卡密类型：天卡、周卡、月卡、年卡
    
    const cardMap: Record<string, { days: number; name: string }> = {
        '天卡': { days: 1, name: '天卡' },
        '周卡': { days: 7, name: '周卡' },
        '月卡': { days: 30, name: '月卡' },
        '年卡': { days: 365, name: '年卡' },
    };
    
    if (!cardMap[cardType]) {
        await sendReplyMessage(ctx, event, '❌ 卡密类型错误\n可用：天卡、周卡、月卡、年卡');
        return;
    }
    
    const card = generateCard(cardMap[cardType].days);
    pluginState.saveDataFile('筱筱吖/授权系统/卡密管理/卡密数据.json', card.card, card);
    
    await sendReplyMessage(ctx, event, `✅ 生成${cardMap[cardType].name}成功\n卡密：${card.card}\n有效期：${card.days}天`);
}

// 处理使用卡密
async function handleUseCard(
    message: string,
    event: OB11Message,
    ctx: PluginContext,
    parsed: Record<string, string>
): Promise<void> {
    const card = parsed.group0; // 卡密内容
    const userId = String(event.user_id);
    const groupId = event.group_id ? String(event.group_id) : '私聊';
    
    // 验证卡密
    const cardData = pluginState.loadDataFile(
        '筱筱吖/授权系统/卡密管理/卡密数据.json',
        card,
        null
    );
    
    if (!cardData) {
        await sendReplyMessage(ctx, event, '❌ 卡密无效或已使用');
        return;
    }
    
    // 激活授权
    const authInfo = {
        type: cardData.type,
        createTime: Math.floor(Date.now() / 1000),
        expireTime: Math.floor(Date.now() / 1000) + cardData.days * 86400
    };
    
    pluginState.saveDataFile(`筱筱吖/授权系统/授权信息/${groupId}.json`, userId, authInfo);
    
    // 删除已使用的卡密
    const allCards = pluginState.loadDataFile('筱筱吖/授权系统/卡密管理/卡密数据.json', {});
    delete allCards[card];
    pluginState.saveDataFile('筱筱吖/授权系统/卡密管理/卡密数据.json', allCards);
    
    await sendReplyMessage(ctx, event, `✅ 激活成功\n类型：${authInfo.type}\n到期：${new Date(authInfo.expireTime * 1000).toLocaleString()}`);
}

// 导出所有指令
export const authCommands: CommandMetadata[] = [
    {
        name: '授权判断',
        pattern: /^授权判断$/,
        level: CommandLevel.USER,
        module: 'auth',
        description: '查看当前授权状态',
        handler: handleAuthCheck,
        requireAuth: false  // 查看授权不需要卡密
    },
    {
        name: '生成天卡',
        pattern: /^生成天卡$/,
        level: CommandLevel.OWNER,
        module: 'auth',
        description: '生成24小时卡密',
        handler: handleGenerateCard,
        requireAuth: false,
        requireWhitelist: false
    },
    {
        name: '生成周卡',
        pattern: /^生成周卡$/,
        level: CommandLevel.OWNER,
        module: 'auth',
        description: '生成7天卡密',
        handler: handleGenerateCard,
        requireAuth: false,
        requireWhitelist: false
    },
    {
        name: '生成月卡',
        pattern: /^生成月卡$/,
        level: CommandLevel.OWNER,
        module: 'auth',
        description: '生成30天卡密',
        handler: handleGenerateCard,
        requireAuth: false,
        requireWhitelist: false
    },
    {
        name: '生成年卡',
        pattern: /^生成年卡$/,
        level: CommandLevel.OWNER,
        module: 'auth',
        description: '生成365天卡密',
        handler: handleGenerateCard,
        requireAuth: false,
        requireWhitelist: false
    },
    {
        name: '使用卡密',
        pattern: /^使用卡密\s+(.+)$/,
        level: CommandLevel.USER,
        module: 'auth',
        description: '使用卡密激活授权',
        handler: handleUseCard,
        requireAuth: false  // 使用卡密不需要已有授权
    }
];

// 辅助函数
function generateCard(days: number): { card: string; days: number; type: string } {
    const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
    let card = '';
    for (let i = 0; i < 12; i++) {
        card += chars.charAt(Math.floor(Math.random() * chars.length));
    }
    return {
        card: `QTBOT-${card}`,
        days,
        type: days === 1 ? '天卡' : days === 7 ? '周卡' : days === 30 ? '月卡' : '年卡'
    };
}
```

## 模块注册

### 在插件初始化时注册

```typescript
// src/index.ts
import { commandRegistry } from './core/command-registry';
import { authCommands } from './modules/auth.module';
import { musicCommands } from './modules/music.module';
import { dailyCommands } from './modules/daily.module';

export const plugin_init: PluginModule['plugin_init'] = async (ctx: NapCatPluginContext) => {
    // 初始化状态
    pluginState.init(ctx);
    
    // 注册所有指令
    commandRegistry.registerBatch(authCommands);
    commandRegistry.registerBatch(musicCommands);
    commandRegistry.registerBatch(dailyCommands);
    
    ctx.logger.info('插件初始化完成');
};
```

## 模块组织

### 按功能分组

```
src/modules/
├── auth.module.ts          # 授权系统
├── music.module.ts         # 音乐系统
├── daily.module.ts         # 日常功能
├── bank.module.ts          # 银行系统
├── group.module.ts         # 群管系统
├── event.module.ts         # 事件管理
├── invite.module.ts        # 邀请统计
├── bottle.module.ts        # 漂流瓶
└── system.module.ts        # 系统管理
```

### 指令和实现分离

```
src/modules/
├── auth/
│   ├── index.ts            # 模块入口
│   ├── commands.ts         # 指令定义
│   └── handlers.ts         # 处理器实现
├── music/
│   ├── index.ts
│   ├── commands.ts
│   └── handlers.ts
└── ...
```

## 最佳实践

### 1. 指令命名规范

```typescript
// ✅ 好的命名
name: '授权判断'
name: '生成天卡'
name: '点歌'

// ❌ 避免
name: 'checkAuth'
name: 'genDayCard'
name: 'music'
```

### 2. 模式匹配

```typescript
// ✅ 简单匹配
pattern: /^签到$/

// ✅ 带参数
pattern: /^点歌\s+(.+)$/

// ✅ 可选参数
pattern: /^搜索\s+(.+?)(?:\s+(.+))?$/

// ❌ 避免过于复杂的正则
pattern: /^(?:功能1|功能2|功能3)\s+(?:\d+|[a-zA-Z]+)(?:\s+(?:\d+|[a-zA-Z]+))?$/
```

### 3. 错误处理

```typescript
const handler: CommandHandler = async (message, event, ctx, parsed) => {
    try {
        // 业务逻辑
    } catch (error) {
        ctx.logger.error('处理指令失败:', error);
        await sendReplyMessage(ctx, event, '❌ 处理失败，请稍后重试');
    }
};
```

### 4. 参数验证

```typescript
const handler: CommandHandler = async (message, event, ctx, parsed) => {
    const amount = parsed.group0;
    
    // 验证参数
    if (!amount || isNaN(Number(amount))) {
        await sendReplyMessage(ctx, event, '❌ 参数错误，请输入有效的数字');
        return;
    }
    
    const numAmount = Number(amount);
    if (numAmount < 0 || numAmount > 10000) {
        await sendReplyMessage(ctx, event, '❌ 金额范围：0-10000');
        return;
    }
    
    // 处理逻辑
};
```

### 5. 权限检查

```typescript
import { checkUserPermission } from '../core/permission-checker';

const handler: CommandHandler = async (message, event, ctx, parsed) => {
    const userId = event.user_id;
    const groupId = event.group_id;
    
    // 检查权限
    const result = checkUserPermission(userId, '指令名称', groupId, event.message_type);
    
    if (!result.allow) {
        await sendReplyMessage(ctx, event, `❌ ${result.reason}`);
        return;
    }
    
    // 处理逻辑
};
```

## 模块测试

### 测试指令匹配

```typescript
import { commandRegistry } from '../core/command-registry';

// 测试指令匹配
const testMessage = '生成天卡';
const match = commandRegistry.match(testMessage);

if (match) {
    console.log('匹配到指令:', match.meta.name);
    console.log('解析参数:', match.parsed);
} else {
    console.log('未匹配到指令');
}
```

### 测试权限检查

```typescript
import { checkUserPermission } from '../core/permission-checker';

const result = checkUserPermission(123456, '生成天卡', 789012, 'group');
console.log('权限检查结果:', result);
```