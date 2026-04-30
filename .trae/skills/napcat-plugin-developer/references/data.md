# 数据持久化完整指南

## PluginState 单例

PluginState 是全局状态管理单例，提供配置持久化和运行时状态管理：

```typescript
import { pluginState } from '../core/state';

// 访问配置
const enabled = pluginState.config.enabled;

// 访问日志器
pluginState.logger.info('日志信息');

// 访问上下文
const ctx = pluginState.ctx;
```

## 配置管理

### 加载配置

```typescript
// 插件初始化时自动加载
pluginState.init(ctx);
```

### 保存配置

```typescript
// 保存当前配置
pluginState.saveConfig();

// 更新后自动保存
pluginState.updateConfig({ enabled: true });
```

### 更新配置

```typescript
// 更新单个配置项
pluginState.updateConfig({ enabled: true });
pluginState.updateConfig({ debug: false });
pluginState.updateConfig({ commandPrefix: '#cmd' });

// 批量更新
pluginState.updateConfig({
    enabled: true,
    debug: false,
    cooldownSeconds: 30
});

// 完整替换
pluginState.replaceConfig({
    enabled: true,
    debug: false,
    commandPrefix: '#cmd',
    cooldownSeconds: 60,
    groupConfigs: {}
});
```

### 配置清洗

```typescript
function sanitizeConfig(raw: unknown): PluginConfig {
    if (!isObject(raw)) return { ...DEFAULT_CONFIG, groupConfigs: {} };

    const out: PluginConfig = { ...DEFAULT_CONFIG, groupConfigs: {} };

    // 清洗基础配置
    if (typeof raw.enabled === 'boolean') out.enabled = raw.enabled;
    if (typeof raw.debug === 'boolean') out.debug = raw.debug;
    if (typeof raw.commandPrefix === 'string') out.commandPrefix = raw.commandPrefix;
    if (typeof raw.cooldownSeconds === 'number') out.cooldownSeconds = raw.cooldownSeconds;

    // 清洗群配置
    if (isObject(raw.groupConfigs)) {
        for (const [groupId, groupConfig] of Object.entries(raw.groupConfigs)) {
            if (isObject(groupConfig)) {
                const cfg: GroupConfig = {};
                if (typeof groupConfig.enabled === 'boolean') cfg.enabled = groupConfig.enabled;
                out.groupConfigs[groupId] = cfg;
            }
        }
    }

    return out;
}
```

## 数据文件读写

### 读取数据文件

```typescript
// 读取整个文件
const data = pluginState.loadDataFile('data.json', {
    key1: 'default1',
    key2: 'default2'
});

// 读取指定键
const value = pluginState.loadDataFile('data.json', 'key1', 'default');
```

### 保存数据文件

```typescript
// 保存整个文件
pluginState.saveDataFile('data.json', {
    key1: 'value1',
    key2: 'value2'
});

// 更新指定键
pluginState.saveDataFile('data.json', 'key1', 'new_value');
```

### 删除数据键

```typescript
pluginState.deleteKey('data.json', 'key1');
```

### 读取文本文件

```typescript
const htmlContent = pluginState.loadTextFile('template.html', '<div>默认内容</div>');
```

## 群配置管理

### 检查群是否启用

```typescript
if (pluginState.isGroupEnabled(groupId)) {
    // 群已启用，处理消息
} else {
    // 群已禁用，忽略消息
}
```

### 更新群配置

```typescript
// 更新单个群配置
pluginState.updateGroupConfig(groupId, {
    enabled: true,
    customPrefix: '#cmd'
});

// 批量更新
for (const id of groupIds) {
    pluginState.updateGroupConfig(id, { enabled: true });
}
```

### 好友配置管理

```typescript
// 检查好友是否启用
if (pluginState.isFriendEnabled(userId)) {
    // 好友已启用
}

// 更新好友配置
pluginState.updateFriendConfig(userId, {
    enabled: true,
    customPrefix: '#cmd'
});
```

## 统计信息

### 访问统计

```typescript
// 运行时长（毫秒）
const uptime = pluginState.getUptime();

// 格式化运行时长
const formatted = pluginState.getUptimeFormatted();
// 返回: "2天3小时" 或 "45分钟"

// 处理计数
const total = pluginState.stats.processed;
const today = pluginState.stats.todayProcessed;
const lastDay = pluginState.stats.lastUpdateDay;
```

### 增加处理计数

```typescript
// 消息处理后调用
pluginState.incrementProcessed();
```

## 数据文件组织

### 推荐的数据目录结构

```
筱筱吖/
├── 授权系统/
│   ├── 授权信息/
│   │   ├── {群号}.json      # 群授权信息
│   │   └── 私聊.json         # 私聊授权信息
│   ├── 卡密管理/
│   │   └── 卡密数据.json     # 卡密数据
│   └── 已用卡密.json         # 已使用卡密列表
├── 娱乐系统/
│   ├── 签到数据/
│   │   ├── 全服记录数量.json
│   │   ├── 累计次数.json
│   │   ├── 日期记录/
│   │   │   └── {日期}/
│   │   │       ├── 检测.json
│   │   │       ├── 排名.json
│   │   │       └── 详细时间.json
│   │   └── 连签记录/
│   │       ├── 上次签到/
│   │       │   └── 详细时间.json
│   │       └── 连签数量.json
│   └── 游戏数据/
│       ├── 归笺.json
│       └── 银行系统/
│           ├── 银行归笺.json
│           └── 储存时间.json
└── 音乐系统/
    ├── 使用音源.json
    ├── 点歌名字.json
    ├── 选歌范围.json
    ├── 临时歌单/
    │   └── {用户ID}.json
    └── 音乐收藏/
        └── {用户ID}.json
```

## 常见数据操作

### 卡密管理

```typescript
// 生成卡密
const card = {
    card: `QTBOT-${generateRandomString(12)}`,
    type: '天卡',
    days: 1,
    createTime: Math.floor(Date.now() / 1000)
};

// 保存卡密
pluginState.saveDataFile('筱筱吖/授权系统/卡密管理/卡密数据.json', card.card, card);

// 读取所有卡密
const allCards = pluginState.loadDataFile('筱筱吖/授权系统/卡密管理/卡密数据.json', {});

// 删除已使用的卡密
delete allCards[usedCard];
pluginState.saveDataFile('筱筱吖/授权系统/卡密管理/卡密数据.json', allCards);
```

### 授权管理

```typescript
// 激活授权
const authInfo = {
    type: '天卡',
    createTime: Math.floor(Date.now() / 1000),
    expireTime: Math.floor(Date.now() / 1000) + 86400
};

// 保存授权
pluginState.saveDataFile(
    `筱筱吖/授权系统/授权信息/${groupId}.json`,
    userId,
    authInfo
);

// 读取授权
const authInfo = pluginState.loadDataFile(
    `筱筱吖/授权系统/授权信息/${groupId}.json`,
    userId,
    null
);

// 检查授权是否过期
if (authInfo && authInfo.expireTime > Math.floor(Date.now() / 1000)) {
    // 授权有效
}
```

### 签到数据

```typescript
// 读取签到数据
const today = new Date().toISOString().split('T')[0];
const checkedIn = pluginState.loadDataFile(
    `筱筱吖/娱乐系统/签到数据/日期记录/${today}/检测.json`,
    userId,
    '未签到'
);

// 记录签到
pluginState.saveDataFile(
    `筱筱吖/娱乐系统/签到数据/日期记录/${today}/检测.json`,
    userId,
    '已签到'
);

// 更新累计次数
const count = pluginState.loadDataFile('筱筱吖/娱乐系统/签到数据/累计次数.json', userId, 0);
pluginState.saveDataFile('筱筱吖/娱乐系统/签到数据/累计次数.json', userId, count + 1);
```

### 货币管理

```typescript
// 读取余额
const balance = pluginState.loadDataFile('筱筱吖/娱乐系统/游戏数据/归笺.json', userId, 0);

// 增加余额
pluginState.saveDataFile('筱筱吖/娱乐系统/游戏数据/归笺.json', userId, balance + amount);

// 扣除余额
if (balance >= cost) {
    pluginState.saveDataFile('筱筱吖/娱乐系统/游戏数据/归笺.json', userId, balance - cost);
} else {
    // 余额不足
}
```

## 数据文件路径

### 获取完整路径

```typescript
// 获取数据文件完整路径
const dataPath = pluginState.getDataFilePath('data.json');
// 返回: C:/Users/xxx/.napcat/data/plugin-id/data.json
```

### 数据目录

```typescript
// 数据目录
const dataPath = pluginState.ctx.dataPath;
// 通常: C:/Users/xxx/.napcat/data/plugin-id/

// 配置文件
const configPath = pluginState.ctx.configPath;
// 通常: C:/Users/xxx/.napcat/config/plugin-id.json
```

## 错误处理

### 读取数据错误处理

```typescript
try {
    const data = pluginState.loadDataFile('data.json', defaultValue);
    // 处理数据
} catch (error) {
    pluginState.logger.error('读取数据文件失败:', error);
    // 使用默认值
    const data = defaultValue;
}
```

### 保存数据错误处理

```typescript
try {
    pluginState.saveDataFile('data.json', data);
} catch (error) {
    pluginState.logger.error('保存数据文件失败:', error);
    // 保存失败处理
}
```

## 数据迁移

### 兼容旧版数据

```typescript
// 检查旧版数据格式
const oldData = pluginState.loadDataFile('old_data.json', null);

if (oldData) {
    // 迁移到新格式
    const newData = migrateOldData(oldData);
    pluginState.saveDataFile('new_data.json', newData);
    
    // 可选：删除旧数据
    // pluginState.deleteKey('old_data.json');
}
```

## 最佳实践

### 1. 使用类型断言

```typescript
interface AuthInfo {
    type: string;
    createTime: number;
    expireTime: number;
}

const authInfo = pluginState.loadDataFile<AuthInfo>(
    `筱筱吖/授权系统/授权信息/${groupId}.json`,
    userId,
    null
) as AuthInfo | null;
```

### 2. 提供默认值

```typescript
// ✅ 好的做法
const data = pluginState.loadDataFile('data.json', { key: 'default' });

// ❌ 避免
const data = pluginState.loadDataFile('data.json'); // 可能返回 undefined
```

### 3. 批量操作

```typescript
// ✅ 批量更新后保存一次
const updates: Record<string, any> = {};
updates[key1] = value1;
updates[key2] = value2;
updates[key3] = value3;
pluginState.saveDataFile('data.json', updates);

// ❌ 避免多次保存
pluginState.saveDataFile('data.json', key1, value1);
pluginState.saveDataFile('data.json', key2, value2);
pluginState.saveDataFile('data.json', key3, value3);
```

### 4. 定期备份

```typescript
// 定期备份重要数据
const backupData = pluginState.loadDataFile('important_data.json', {});
const backupPath = `backup/backup_${Date.now()}.json`;
pluginState.saveDataFile(backupPath, backupData);
```

### 5. 数据验证

```typescript
// 读取后验证数据
const data = pluginState.loadDataFile('data.json', {});

if (!validateData(data)) {
    pluginState.logger.warn('数据验证失败，使用默认值');
    const validData = createDefaultData();
    pluginState.saveDataFile('data.json', validData);
}
```