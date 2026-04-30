# 权限系统详解

## 三层权限校验

NapCat 插件使用三层权限校验系统，确保安全性：

```
用户请求
    ↓
【第一层】白名单校验（全局主授权）
    ├─ 全局开关检查
    └─ 白名单验证
    ↓
【第二层】卡密授权校验
    ├─ 主人检查
    └─ 卡密有效性检查
    ↓
【第三层】角色权限校验
    ├─ 用户角色识别
    └─ 权限等级匹配
    ↓
允许/拒绝
```

## 权限类型

### UserRole（用户角色）

```typescript
enum UserRole {
    OWNER = 'owner',  // 主人：所有权限
    USER = 'user',    // 普通用户：已授权
    GUEST = 'guest'   // 游客：未授权
}
```

### CommandLevel（指令权限等级）

```typescript
enum CommandLevel {
    PUBLIC = 0,  // 公开：所有用户（包括游客）
    USER = 1,    // 用户：需要卡密授权
    OWNER = 2    // 主人：仅主人
}
```

### AuthType（授权类型）

```typescript
enum AuthType {
    OWNER = 'owner',  // 主人授权（无需卡密）
    CARD = 'card',    // 卡密授权
    NONE = 'none'     // 未授权
}
```

## 权限检查 API

### checkUserPermission（核心API）

```typescript
import { checkUserPermission } from '../core/permission-checker';

const result = checkUserPermission(
    userId,          // 用户QQ号
    'commandName',   // 指令名称或输入内容
    groupId,         // 群号（可选，私聊时为undefined）
    'group'          // 消息类型（group 或 private）
);
```

### 返回值

```typescript
interface PermissionCheckResult {
    allow: boolean;              // 是否允许执行
    role: UserRole;              // 用户角色
    reason: string;              // 原因说明
    commandLevel?: CommandLevel; // 指令要求的权限等级
    expireTime?: number;         // 授权过期时间（如果有）
}
```

### 使用示例

```typescript
const result = checkUserPermission(
    123456,          // 用户QQ号
    '生成天卡授权',   // 指令名称
    789012,          // 群号
    'group'          // 消息类型
);

if (result.allow) {
    console.log('允许执行');
    console.log('用户角色:', result.role);
    console.log('过期时间:', result.expireTime);
} else {
    console.log('拒绝:', result.reason);
    console.log('需要权限等级:', result.commandLevel);
}
```

## 第一层：白名单校验

### checkWhitelist

```typescript
import { checkWhitelist } from '../core/permission-checker';

const result = checkWhitelist(
    'group',         // 消息类型
    789012,          // 群号
    123456           // 用户QQ号（可选）
);
```

### 返回值

```typescript
interface WhitelistCheckResult {
    allowed: boolean;  // 是否通过
    reason: string;    // 原因说明
}
```

### 白名单配置

```typescript
// src/config.ts
ctx.NapCatConfig.multiSelect('groupWhitelist', '群白名单', [
    { label: '测试群A', value: '123456' },
    { label: '测试群B', value: '789012' },
], [], '选择允许使用插件的群');

ctx.NapCatConfig.multiSelect('friendWhitelist', '好友白名单', [
    { label: '用户A', value: '111222' },
    { label: '用户B', value: '333444' },
], [], '选择允许使用插件的好友');
```

### 白名单检查规则

1. **全局开关**：如果 `enabled: false`，拒绝所有请求
2. **白名单为空**：如果白名单为空，拒绝所有请求（安全考虑）
3. **主人指令**：如果 `requireWhitelist: false`，跳过白名单检查
4. **普通指令**：必须在白名单内才能执行

## 第二层：卡密授权校验

### checkCardSecret

```typescript
import { checkCardSecret } from '../core/permission-checker';

const result = checkCardSecret(
    123456,          // 用户QQ号
    789012           // 群号（可选）
);
```

### 返回值

```typescript
interface CardSecretCheckResult {
    authorized: boolean;      // 是否已授权
    authType: AuthType;       // 授权类型
    expireTime?: number;      // 过期时间（时间戳，秒）
    reason: string;           // 原因说明
}
```

### 卡密类型

| 卡密类型 | 有效期 | 类型标识 |
|---------|-------|---------|
| 天卡 | 1天 | 天卡 |
| 周卡 | 7天 | 周卡 |
| 月卡 | 30天 | 月卡 |
| 半年卡 | 180天 | 半年卡 |
| 年卡 | 365天 | 年卡 |
| 永久卡 | 永久 | 永久卡 |

### 卡密数据结构

```typescript
interface CardData {
    card: string;       // 卡密内容
    type: string;       // 卡密类型
    days: number;       // 有效天数
    createTime: number; // 创建时间戳
}
```

### 授权信息结构

```typescript
interface AuthInfo {
    type: string;       // 授权类型
    createTime: number; // 创建时间戳
    expireTime: number; // 过期时间戳
}
```

### 卡密授权检查规则

1. **主人检查**：主人无需卡密，直接通过
2. **卡密验证**：检查用户是否有有效卡密
3. **过期检查**：检查卡密是否过期
4. **授权状态**：返回授权类型和过期时间

## 第三层：角色权限校验

### checkPermission

```typescript
import { checkPermission } from '../core/permission-checker';

const result = checkPermission(
    123456,          // 用户QQ号
    CommandLevel.OWNER,  // 指令要求的权限等级
    789012           // 群号（可选）
);
```

### 权限等级比较

| 角色等级 | 角色类型 | 可执行的指令等级 |
|---------|---------|-----------------|
| 2 | OWNER | OWNER, USER, PUBLIC |
| 1 | USER | USER, PUBLIC |
| 0 | GUEST | PUBLIC |

### 角色权限匹配规则

- 用户角色等级 >= 指令权限等级 → 允许
- 用户角色等级 < 指令权限等级 → 拒绝

## 用户角色管理

### getUserRole

```typescript
import { getUserRole } from '../core/user-role-manager';

const role = getUserRole(123456, 789012);
// 返回: UserRole.OWNER | UserRole.USER | UserRole.GUEST
```

### isOwner

```typescript
import { isOwner } from '../core/user-role-manager';

if (isOwner(123456)) {
    // 是主人
}
```

### hasCardSecretAuth

```typescript
import { hasCardSecretAuth } from '../core/user-role-manager';

if (hasCardSecretAuth(123456, 789012)) {
    // 有卡密授权
}
```

### getAuthExpireTime

```typescript
import { getAuthExpireTime } from '../core/user-role-manager';

const expireTime = getAuthExpireTime(123456, 789012);
if (expireTime) {
    console.log('授权过期时间:', new Date(expireTime * 1000).toLocaleString());
}
```

## 指令权限配置

### 在指令定义中配置权限

```typescript
import { CommandLevel } from '../core/permission-types';

export const ownerCommand: CommandMetadata = {
    name: '生成天卡',
    pattern: /^生成天卡$/,
    level: CommandLevel.OWNER,        // 仅主人可用
    module: 'auth',
    description: '生成24小时卡密',
    handler: handleGenerateCard,
    requireAuth: false,                // 主人无需卡密
    requireWhitelist: false            // 主人无需白名单
};

export const userCommand: CommandMetadata = {
    name: '签到',
    pattern: /^签到$/,
    level: CommandLevel.USER,          // 需要卡密授权
    module: 'daily',
    description: '每日签到获取奖励',
    handler: handleCheckIn,
    requireAuth: true,                 // 需要卡密
    requireWhitelist: true             // 需要白名单
};

export const publicCommand: CommandMetadata = {
    name: '菜单',
    pattern: /^菜单$/,
    level: CommandLevel.PUBLIC,        // 公开指令
    module: 'system',
    description: '显示功能菜单',
    handler: handleMenu,
    requireAuth: false,                // 无需卡密
    requireWhitelist: true             // 需要白名单
};
```

## 完整权限检查流程

### 在消息处理中使用

```typescript
import { checkUserPermission } from '../core/permission-checker';
import type { OB11Message, NapCatPluginContext } from 'napcat-types/napcat-onebot';
import type { NapCatPluginContext as PluginContext } from 'napcat-types/napcat-onebot/network/plugin/types';

const handler: CommandHandler = async (
    message: string,
    event: OB11Message,
    ctx: PluginContext,
    parsed: Record<string, string>
) => {
    const userId = event.user_id;
    const groupId = event.group_id;
    const messageType = event.message_type;
    
    // 检查权限
    const result = checkUserPermission(
        userId,
        '生成天卡',
        groupId,
        messageType
    );
    
    if (!result.allow) {
        await sendReplyMessage(ctx, event, `❌ ${result.reason}`);
        return;
    }
    
    // 执行指令逻辑
    await sendReplyMessage(ctx, event, `✅ 权限验证通过\n角色：${result.role}`);
};
```

## 批量权限检查

### checkBatchPermissions

```typescript
import { checkBatchPermissions } from '../core/permission-checker';

const results = checkBatchPermissions(
    123456,                              // 用户QQ号
    ['生成天卡', '签到', '菜单'],          // 指令列表
    789012,                               // 群号
    'group'                               // 消息类型
);

// 返回示例：
// {
//   '生成天卡': { allow: false, role: 'user', reason: '需要 主人 权限，当前为 用户' },
//   '签到': { allow: true, role: 'user', reason: '权限校验通过' },
//   '菜单': { allow: true, role: 'user', reason: '权限校验通过' }
// }
```

### getAvailableCommands

```typescript
import { getAvailableCommands } from '../core/permission-checker';

const commands = getAvailableCommands(123456, 789012);
// 返回该用户可执行的所有指令
```

## 主人配置

### 在配置中设置主人

```typescript
// src/config.ts
ctx.NapCatConfig.multiSelect('ownerQQs', '主人QQ列表', [
    { label: '用户A', value: '123456' },
    { label: '用户B', value: '789012' },
], [], '设置拥有所有权限的主人账号');
```

### 检查主人权限

```typescript
import { isOwner } from '../core/user-role-manager';

if (isOwner(123456)) {
    // 主人权限，跳过卡密检查
}
```

## 权限系统最佳实践

### 1. 敏感操作使用 OWNER 级别

```typescript
{
    name: '删除授权',
    pattern: /^删除授权\s+(\d+)$/,
    level: CommandLevel.OWNER,        // 仅主人
    module: 'auth',
    description: '删除指定用户的授权',
    handler: handleDeleteAuth,
    requireAuth: false,
    requireWhitelist: false
}
```

### 2. 常用功能使用 USER 级别

```typescript
{
    name: '签到',
    pattern: /^签到$/,
    level: CommandLevel.USER,          // 需要授权
    module: 'daily',
    description: '每日签到获取奖励',
    handler: handleCheckIn,
    requireAuth: true,
    requireWhitelist: true
}
```

### 3. 帮助菜单使用 PUBLIC 级别

```typescript
{
    name: '菜单',
    pattern: /^菜单$/,
    level: CommandLevel.PUBLIC,        // 公开
    module: 'system',
    description: '显示功能菜单',
    handler: handleMenu,
    requireAuth: false,
    requireWhitelist: true
}
```

### 4. 始终进行权限检查

```typescript
// 即使是公开指令，也要检查权限（白名单）
const handler: CommandHandler = async (message, event, ctx, parsed) => {
    const result = checkUserPermission(
        event.user_id,
        '菜单',
        event.group_id,
        event.message_type
    );
    
    if (!result.allow) {
        await sendReplyMessage(ctx, event, `❌ ${result.reason}`);
        return;
    }
    
    // 执行指令
};
```