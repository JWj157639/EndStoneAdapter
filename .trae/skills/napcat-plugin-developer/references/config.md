# 配置系统详解

## 配置 Schema

使用 `NapCatConfig` 构建器创建配置界面：

```typescript
import type { NapCatPluginContext, PluginConfigSchema } from 'napcat-types/napcat-onebot/network/plugin/types';
import type { PluginConfig } from './types';

export function buildConfigSchema(ctx: NapCatPluginContext): PluginConfigSchema {
    return ctx.NapCatConfig.combine(
        // 所有配置项
    );
}

// 导出配置 UI Schema
export let plugin_config_ui: PluginConfigSchema = [];
```

## 配置项类型

### 1. HTML 内容

用于显示说明、警告或分隔线：

```typescript
ctx.NapCatConfig.html(`
    <div style="padding: 16px; background: #FB7299; border-radius: 12px; margin-bottom: 20px; color: white;">
        <h3 style="margin: 0 0 6px 0; font-size: 18px; font-weight: 600;">插件名称</h3>
        <p style="margin: 0; font-size: 13px; opacity: 0.85;">插件描述</p>
    </div>
`),

// 分隔线
ctx.NapCatConfig.html('<hr style="margin: 20px 0; border: none; border-top: 1px solid #e5e7eb;">'),

// 警告信息
ctx.NapCatConfig.html(`
    <div style="padding: 12px; background: #fef3c7; border-radius: 8px; margin: 16px 0; color: #92400e;">
        <strong>⚠️ 注意：</strong>修改此配置需要重启插件
    </div>
`),
```

### 2. 布尔值（开关）

```typescript
ctx.NapCatConfig.boolean(
    'enabled',           // 配置键
    '启用插件',          // 显示标签
    true,                // 默认值
    '是否启用插件功能',   // 描述
    true                 // reactive: 是否响应式（可选，默认false）
)
```

### 3. 文本输入

```typescript
ctx.NapCatConfig.text(
    'apiKey',                    // 配置键
    'API 密钥',                  // 显示标签
    '',                          // 默认值
    '用于访问外部 API 的密钥',   // 描述
    true                         // reactive: 是否响应式（可选）
)
```

### 4. 数字输入

```typescript
ctx.NapCatConfig.number(
    'maxRetries',           // 配置键
    '最大重试次数',          // 显示标签
    3,                      // 默认值
    '请求失败后的重试次数',  // 描述
    true                    // reactive: 是否响应式（可选）
)
```

### 5. 下拉单选

```typescript
ctx.NapCatConfig.select(
    'model',                                    // 配置键
    'AI 模型',                                  // 显示标签
    [                                           // 选项数组
        { label: 'GPT-3.5 Turbo', value: 'gpt-3.5-turbo' },
        { label: 'GPT-4', value: 'gpt-4' },
        { label: 'Claude 3', value: 'claude-3' },
    ],
    'gpt-3.5-turbo',                            // 默认值
    '选择用于生成回复的 AI 模型',               // 描述
)
```

### 6. 下拉多选

```typescript
ctx.NapCatConfig.multiSelect(
    'features',                                 // 配置键
    '启用功能',                                  // 显示标签
    [                                           // 选项数组
        { label: '自动回复', value: 'auto-reply' },
        { label: '表情识别', value: 'emoji' },
        { label: '图片处理', value: 'image' },
        { label: '语音转文字', value: 'stt' },
    ],
    ['auto-reply'],                            // 默认值
    '选择要启用的功能模块',                     // 描述
)
```

### 7. 纯文本说明

```typescript
ctx.NapCatConfig.plainText('以下为高级配置选项'),
ctx.NapCatConfig.plainText('────────────────────────────────'),
```

## 配置接口定义

在 `src/types.ts` 中定义配置类型：

```typescript
export interface PluginConfig {
    // 基础配置
    enabled: boolean;
    debug: boolean;
    commandPrefix: string;
    cooldownSeconds: number;
    
    // 高级配置
    apiKey?: string;
    maxRetries: number;
    model: string;
    features: string[];
    
    // 群配置
    groupConfigs: Record<string, GroupConfig>;
}

export interface GroupConfig {
    enabled?: boolean;
    customPrefix?: string;
}
```

## 配置默认值

在 `src/config.ts` 中定义默认值：

```typescript
export const DEFAULT_CONFIG: PluginConfig = {
    enabled: true,
    debug: false,
    commandPrefix: '#cmd',
    cooldownSeconds: 60,
    maxRetries: 3,
    model: 'gpt-3.5-turbo',
    features: [],
    groupConfigs: {},
};
```

## 配置清洗

在 `src/core/state.ts` 中清洗配置：

```typescript
function sanitizeConfig(raw: unknown): PluginConfig {
    if (!isObject(raw)) return { ...DEFAULT_CONFIG, groupConfigs: {} };

    const out: PluginConfig = { ...DEFAULT_CONFIG, groupConfigs: {} };

    // 清洗基础配置
    if (typeof raw.enabled === 'boolean') out.enabled = raw.enabled;
    if (typeof raw.debug === 'boolean') out.debug = raw.debug;
    if (typeof raw.commandPrefix === 'string') out.commandPrefix = raw.commandPrefix;
    if (typeof raw.cooldownSeconds === 'number') out.cooldownSeconds = raw.cooldownSeconds;
    
    // 清洗高级配置
    if (typeof raw.apiKey === 'string') out.apiKey = raw.apiKey;
    if (typeof raw.maxRetries === 'number') out.maxRetries = raw.maxRetries;
    if (typeof raw.model === 'string') out.model = raw.model;
    if (Array.isArray(raw.features)) out.features = raw.features;

    // 清洗群配置
    if (isObject(raw.groupConfigs)) {
        for (const [groupId, groupConfig] of Object.entries(raw.groupConfigs)) {
            if (isObject(groupConfig)) {
                const cfg: GroupConfig = {};
                if (typeof groupConfig.enabled === 'boolean') cfg.enabled = groupConfig.enabled;
                if (typeof groupConfig.customPrefix === 'string') cfg.customPrefix = groupConfig.customPrefix;
                out.groupConfigs[groupId] = cfg;
            }
        }
    }

    return out;
}
```

## 配置操作

### 读取配置

```typescript
// 直接访问
const enabled = pluginState.config.enabled;
const prefix = pluginState.config.commandPrefix;

// 通过配置钩子
export const plugin_get_config: PluginModule['plugin_get_config'] = async (ctx) => {
    return pluginState.config;
};
```

### 更新配置

```typescript
// 更新单个配置项
pluginState.updateConfig({ enabled: true });
pluginState.updateConfig({ debug: true });

// 批量更新
pluginState.updateConfig({
    enabled: true,
    debug: false,
    maxRetries: 5
});

// 完整替换
export const plugin_set_config: PluginModule['plugin_set_config'] = async (ctx, config) => {
    pluginState.replaceConfig(config as PluginConfig);
    ctx.logger.info('配置已更新');
};
```

### 配置变更回调

```typescript
export const plugin_on_config_change: PluginModule['plugin_on_config_change'] = async (
    ctx, ui, key, value, currentConfig
) => {
    try {
        pluginState.updateConfig({ [key]: value });
        ctx.logger.debug(`配置项 ${key} 已更新`);
        
        // 动态控制 UI
        if (key === 'mode' && value === 'advanced') {
            ui.showField('advancedOption');
        } else if (key === 'mode' && value === 'simple') {
            ui.hideField('advancedOption');
        }
    } catch (err) {
        ctx.logger.error(`更新配置项 ${key} 失败:`, err);
    }
};
```

## 动态配置 UI

### PluginConfigUIController 方法

```typescript
export const plugin_config_controller: PluginModule['plugin_config_controller'] = async (
    _ctx, ui, initialConfig
) => {
    // 根据初始配置显示/隐藏字段
    if (initialConfig.mode === 'simple') {
        ui.hideField('advancedOption');
    }

    // 动态添加字段
    ui.addField({
        key: 'dynamicField',
        type: 'text',
        label: '动态字段',
        default: ''
    });

    // 返回清理函数
    return () => {
        // 清理资源
    };
};
```

### UI 控制器方法

```typescript
// 显示字段
ui.showField('fieldName');

// 隐藏字段
ui.hideField('fieldName');

// 更新字段
ui.updateField('fieldName', {
    label: '新标签',
    description: '新描述'
});

// 删除字段
ui.removeField('fieldName');

// 更新整个 Schema
ui.updateSchema(newSchema);

// 获取当前配置
const currentConfig = ui.getCurrentConfig();
```

## 配置保存

配置自动保存到 `config.json` 文件：

```typescript
// 手动保存
pluginState.saveConfig();

// 更新后自动保存
pluginState.updateConfig({ enabled: true }); // 会自动保存
```

## 完整示例

```typescript
// src/config.ts
import type { NapCatPluginContext, PluginConfigSchema } from 'napcat-types/napcat-onebot/network/plugin/types';
import type { PluginConfig } from './types';

export const DEFAULT_CONFIG: PluginConfig = {
    enabled: true,
    debug: false,
    commandPrefix: '#cmd',
    cooldownSeconds: 60,
    apiEndpoint: 'https://api.example.com',
    maxRetries: 3,
    model: 'gpt-3.5-turbo',
    features: [],
    groupConfigs: {},
};

export function buildConfigSchema(ctx: NapCatPluginContext): PluginConfigSchema {
    return ctx.NapCatConfig.combine(
        // 插件信息
        ctx.NapCatConfig.html(`
            <div style="padding: 16px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); border-radius: 12px; margin-bottom: 20px; color: white;">
                <h3 style="margin: 0 0 6px 0; font-size: 18px; font-weight: 600;">🤖 AI 智能助手</h3>
                <p style="margin: 0; font-size: 13px; opacity: 0.9;">基于 GPT 的 QQ 机器人插件</p>
            </div>
        `),
        
        // 基础配置
        ctx.NapCatConfig.boolean('enabled', '启用插件', true, '全局开关，关闭后不响应任何消息'),
        ctx.NapCatConfig.boolean('debug', '调试模式', false, '启用后输出详细日志'),
        ctx.NapCatConfig.text('commandPrefix', '命令前缀', '#cmd', '触发命令的前缀符号'),
        ctx.NapCatConfig.number('cooldownSeconds', '冷却时间（秒）', 60, '同一命令请求冷却时间，0 表示不限制'),
        
        // 分隔线
        ctx.NapCatConfig.html('<hr style="margin: 20px 0; border: none; border-top: 1px solid #e5e7eb;">'),
        
        // API 配置
        ctx.NapCatConfig.text('apiEndpoint', 'API 端点', 'https://api.example.com', '外部 API 服务地址'),
        ctx.NapCatConfig.number('maxRetries', '最大重试次数', 3, '请求失败后的重试次数'),
        ctx.NapCatConfig.select('model', 'AI 模型', [
            { label: 'GPT-3.5 Turbo', value: 'gpt-3.5-turbo' },
            { label: 'GPT-4', value: 'gpt-4' },
            { label: 'Claude 3', value: 'claude-3' },
        ], 'gpt-3.5-turbo', '选择用于生成回复的 AI 模型'),
        
        // 功能开关
        ctx.NapCatConfig.multiSelect('features', '启用功能', [
            { label: '自动回复', value: 'auto-reply' },
            { label: '表情识别', value: 'emoji' },
            { label: '图片处理', value: 'image' },
            { label: '语音转文字', value: 'stt' },
        ], ['auto-reply'], '选择要启用的功能模块'),
    );
}
```