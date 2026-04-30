# QTBOT 插件 - API 文档

## 目录

- [权限系统 API](#权限系统-api)
- [指令注册 API](#指令注册-api)
- [角色管理 API](#角色管理-api)
- [指令匹配 API](#指令匹配-api)
- [消息处理 API](#消息处理-api)
- [数据管理 API](#数据管理-api)

---

## 权限系统 API

### 核心函数：checkUserPermission()

判断用户是否允许执行某指令（三层权限校验）

**文件**: `src/core/permission-checker.ts`

```typescript
/**
 * 判断用户是否允许执行某指令
 *
 * 三层权限校验流程：
 * 1. 第一层：白名单校验（全局主授权）
 * 2. 第二层：卡密授权校验
 * 3. 第三层：角色权限校验
 *
 * @param userId 用户QQ号
 * @param command 指令名称或输入内容
 * @param groupId 群号（可选，私聊时为undefined）
 * @param messageType 消息类型（group 或 private）
 * @returns 检查结果
 */
export function checkUserPermission(
  userId: number,
  command: string,
  groupId?: number,
  messageType?: 'group' | 'private'
): PermissionCheckResult
```

**返回值**: `PermissionCheckResult`

```typescript
interface PermissionCheckResult {
  allow: boolean;              // 是否允许执行
  role: UserRole;              // 用户角色
  reason: string;              // 原因说明
  commandLevel?: CommandLevel; // 指令要求的权限等级
  expireTime?: number;         // 授权过期时间（如果有）
}
```

**使用示例**:

```typescript
import { checkUserPermission } from './core/permission-checker';

const result = checkUserPermission(
  123456,          // 用户QQ号
  '生成天卡授权',   // 指令名称
  789012,          // 群号（可选）
  'group'          // 消息类型
);

if (result.allow) {
  console.log('允许执行');
} else {
  console.log('拒绝:', result.reason);
}
```

### 第一层：checkWhitelist()

白名单校验

```typescript
export function checkWhitelist(
  messageType: 'group' | 'private',
  groupId?: number,
  userId?: number
): WhitelistCheckResult
```

**返回值**:

```typescript
interface WhitelistCheckResult {
  allowed: boolean;  // 是否通过
  reason: string;    // 原因说明
}
```

### 第二层：checkCardSecret()

卡密授权校验

```typescript
export function checkCardSecret(
  userId: number,
  groupId?: number
): CardSecretCheckResult
```

**返回值**:

```typescript
interface CardSecretCheckResult {
  authorized: boolean;      // 是否已授权
  authType: AuthType;       // 授权类型
  expireTime?: number;      // 过期时间（时间戳，秒）
  reason: string;           // 原因说明
}
```

### 第三层：checkPermission()

角色权限校验

```typescript
export function checkPermission(
  userId: number,
  commandLevel: CommandLevel,
  groupId?: number
): PermissionCheckResult
```

### 批量权限检查：checkBatchPermissions()

批量检查多个指令的权限

```typescript
export function checkBatchPermissions(
  userId: number,
  commands: string[],
  groupId?: number,
  messageType?: 'group' | 'private'
): Record<string, PermissionCheckResult>
```

**使用示例**:

```typescript
const results = checkBatchPermissions(
  123456,
  ['生成天卡授权', '授权判断', '菜单'],
  789012,
  'group'
);

// results = {
//   '生成天卡授权': { allow: false, role: 'user', reason: '需要 主人 权限，当前为 用户' },
//   '授权判断': { allow: true, role: 'user', reason: '权限校验通过' },
//   '菜单': { allow: true, role: 'user', reason: '权限校验通过' }
// }
```

### 获取可用指令：getAvailableCommands()

获取用户可执行的指令列表

```typescript
export function getAvailableCommands(
  userId: number,
  groupId?: number
): CommandMetadata[]
```

---

## 指令注册 API

### CommandRegistry 类

**文件**: `src/core/command-registry.ts`

#### 注册指令：register()

```typescript
/**
 * 注册单个指令
 * @param meta 指令元数据
 */
register(meta: CommandMetadata): void
```

**参数**: `CommandMetadata`

```typescript
interface CommandMetadata {
  name: string;              // 指令名称（唯一标识）
  pattern: string | RegExp;  // 指令匹配模式
  level: CommandLevel;       // 权限等级
  module: string;            // 所属模块
  description: string;       // 指令描述
  params?: CommandParam[];   // 参数说明
  handler: CommandHandler;   // 执行处理器
  requireWhitelist?: boolean;// 是否需要白名单
  requireAuth?: boolean;     // 是否需要卡密授权
  cooldown?: number;         // 冷却时间（秒）
}
```

#### 批量注册：registerBatch()

```typescript
/**
 * 批量注册指令
 * @param metas 指令元数据数组
 */
registerBatch(metas: CommandMetadata[]): void
```

#### 匹配指令：match()

```typescript
/**
 * 匹配指令
 * @param input 用户输入的消息内容
 * @returns 匹配结果，未匹配返回 null
 */
match(input: string): CommandMatchResult | null
```

**返回值**:

```typescript
interface CommandMatchResult {
  meta: CommandMetadata;              // 指令元数据
  parsed: Record<string, string>;     // 解析后的参数
}
```

#### 查询指令：getByName()

```typescript
getByName(name: string): CommandMetadata | undefined
```

#### 获取所有指令：getAll()

```typescript
getAll(): CommandMetadata[]
```

#### 根据模块获取：getByModule()

```typescript
getByModule(module: string): CommandMetadata[]
```

#### 根据权限等级获取：getByLevel()

```typescript
getByLevel(level: number): CommandMetadata[]
```

#### 获取指令数量：size()

```typescript
size(): number
```

#### 清空指令：clear()

```typescript
clear(): void
```

#### 移除指令：unregister()

```typescript
unregister(name: string): boolean
```

---

## 角色管理 API

### 获取用户角色：getUserRole()

**文件**: `src/core/user-role-manager.ts`

```typescript
export function getUserRole(userId: number, groupId?: number): UserRole
```

**返回值**: `UserRole`

```typescript
enum UserRole {
  OWNER = 'owner',  // 主人：拥有所有权限
  USER = 'user',    // 普通用户：已授权，可使用基础功能
  GUEST = 'guest'   // 游客：未授权，仅能使用公开功能
}
```

### 检查是否为主人：isOwner()

```typescript
export function isOwner(userId: number): boolean
```

### 检查是否有卡密授权：hasCardSecretAuth()

```typescript
export function hasCardSecretAuth(userId: number, groupId?: number): boolean
```

### 获取授权过期时间：getAuthExpireTime()

```typescript
export function getAuthExpireTime(userId: number, groupId?: number): number | undefined
```

### 获取角色等级：getRoleLevel()

```typescript
export function getRoleLevel(role: UserRole): number
```

**返回值**:

```typescript
// OWNER -> 2
// USER -> 1
// GUEST -> 0
```

### 获取角色名称：getRoleName()

```typescript
export function getRoleName(role: UserRole): string
```

**返回值**:

```typescript
// OWNER -> '主人'
// USER -> '用户'
// GUEST -> '游客'
```

---

## 指令匹配 API

### 提取消息文本：extractMessageText()

**文件**: `src/core/command-matcher.ts`

```typescript
export function extractMessageText(event: any): string
```

### 移除命令前缀：removeCommandPrefix()

```typescript
export function removeCommandPrefix(message: string): string
```

### 解析消息指令：parseMessageCommand()

```typescript
export function parseMessageCommand(event: any): string
```

**使用示例**:

```typescript
import { parseMessageCommand } from './core/command-matcher';

const command = parseMessageCommand(event);
// 如果 commandPrefix = "#cmd"
// 输入: "#cmd 生成天卡授权 5"
// 输出: "生成天卡授权 5"
```

---

## 消息处理 API

### 发送回复消息：sendReplyMessage()

**文件**: `src/handlers/message-handler.ts`

```typescript
export async function sendReplyMessage(
  ctx: NapCatPluginContext,
  event: OB11Message,
  message: string | MessageSegment[]
): Promise<void>
```

**参数**:

- `ctx`: NapCat 插件上下文
- `event`: 消息事件对象
- `message`: 要发送的消息（字符串或消息段数组）

**功能**: 自动添加 reply 段，回复原消息

### 处理消息：handleMessage()

```typescript
export async function handleMessage(
  ctx: NapCatPluginContext,
  event: OB11Message
): Promise<void>
```

**功能**: 消息处理主函数，包含完整的权限校验流程

---

## 数据管理 API

### PluginState 类

**文件**: `src/core/state.ts`

#### 加载配置：loadConfig()

```typescript
loadConfig(): void
```

#### 保存配置：saveConfig()

```typescript
saveConfig(): void
```

#### 更新配置：updateConfig()

```typescript
updateConfig(partial: Partial<PluginConfig>): void
```

#### 替换配置：replaceConfig()

```typescript
replaceConfig(config: PluginConfig): void
```

#### 读取数据文件：loadDataFile()

```typescript
/**
 * 读取指定键的值
 */
loadDataFile<T>(filename: string, key: string, defaultValue: T): T

/**
 * 读取整个文件
 */
loadDataFile<T>(filename: string, defaultValue: T): T
```

**使用示例**:

```typescript
import { pluginState } from './core/state';

// 读取指定键
const ownerQQs = pluginState.loadDataFile('config.json', 'ownerQQs', []);

// 读取整个文件
const config = pluginState.loadDataFile('config.json', {});
```

#### 保存数据文件：saveDataFile()

```typescript
/**
 * 更新指定键的值
 */
saveDataFile<T>(filename: string, key: string, value: T): void

/**
 * 保存整个文件
 */
saveDataFile<T>(filename: string, data: T): void
```

#### 删除数据键：deleteKey()

```typescript
deleteKey(filename: string, key: string): void
```

#### 读取文本文件：loadTextFile()

```typescript
loadTextFile(filename: string, defaultValue?: string): string
```

#### 检查群组是否启用：isGroupEnabled()

```typescript
isGroupEnabled(groupId: string): boolean
```

#### 检查好友是否启用：isFriendEnabled()

```typescript
isFriendEnabled(userId: string): boolean
```

#### 更新群组配置：updateGroupConfig()

```typescript
updateGroupConfig(groupId: string, config: Partial<GroupConfig>): void
```

#### 更新好友配置：updateFriendConfig()

```typescript
updateFriendConfig(userId: string, config: Partial<FriendConfig>): void
```

#### 增加处理计数：incrementProcessed()

```typescript
incrementProcessed(): void
```

#### 获取运行时长：getUptime()

```typescript
getUptime(): number  // 毫秒
```

#### 获取格式化运行时长：getUptimeFormatted()

```typescript
getUptimeFormatted(): string  // 如 "2天3小时"
```

---

## 权限等级常量

### CommandLevel 枚举

**文件**: `src/core/permission-types.ts`

```typescript
export enum CommandLevel {
  PUBLIC = 0,  // 公开：所有用户（包括游客）均可使用
  USER = 1,    // 用户：需要卡密授权的用户可使用
  OWNER = 2    // 主人：仅主人可使用
}
```

### UserRole 枚举

```typescript
export enum UserRole {
  OWNER = 'owner',  // 主人：拥有所有权限
  USER = 'user',    // 普通用户：已授权，可使用基础功能
  GUEST = 'guest'   // 游客：未授权，仅能使用公开功能
}
```

### AuthType 枚举

```typescript
export enum AuthType {
  OWNER = 'owner',  // 主人授权（无需卡密）
  CARD = 'card',    // 卡密授权
  NONE = 'none'     // 未授权
}
```

---

## 使用示例

### 完整的消息处理流程

```typescript
import { handleMessage } from './handlers/message-handler';
import type { OB11Message } from 'napcat-types/napcat-onebot';
import type { NapCatPluginContext } from 'napcat-types/napcat-onebot/network/plugin/types';

export const plugin_onmessage: PluginModule['plugin_onmessage'] = async (ctx, event) => {
  // 处理消息
  await handleMessage(ctx, event);
};
```

### 自定义指令注册

```typescript
import { commandRegistry, CommandMetadata, CommandLevel } from './core/command-registry';
import type { NapCatPluginContext, OB11Message } from 'napcat-types/napcat-onebot';

const myCommand: CommandMetadata = {
  name: '我的指令',
  pattern: /^我的指令(.*)$/,
  level: CommandLevel.USER,
  module: 'mymodule',
  description: '这是一个自定义指令',
  handler: async (message, event, ctx, parsed) => {
    // 解析参数
    const param = parsed.group0;

    // 业务逻辑
    // ...

    // 发送回复
    await sendReplyMessage(ctx, event, `收到参数: ${param}`);
  },
  requireAuth: true
};

// 注册指令
commandRegistry.register(myCommand);
```

### 权限检查示例

```typescript
import { checkUserPermission } from './core/permission-checker';

// 检查用户是否可以执行某个指令
const result = checkUserPermission(
  123456,          // 用户QQ
  '生成天卡授权',   // 指令
  789012,          // 群号
  'group'          // 消息类型
);

if (result.allow) {
  console.log('允许执行');
  console.log('角色:', result.role);
  console.log('过期时间:', result.expireTime);
} else {
  console.log('拒绝:', result.reason);
  console.log('需要权限等级:', result.commandLevel);
}
```

---

## 注意事项

1. **权限检查**: 所有敏感操作必须先进行权限检查
2. **错误处理**: 所有 API 调用都应该进行错误处理
3. **异步操作**: 消息处理、数据保存等都是异步操作，使用 await
4. **日志记录**: 重要操作应记录日志，便于调试
5. **数据一致性**: 修改数据后及时保存，避免数据丢失