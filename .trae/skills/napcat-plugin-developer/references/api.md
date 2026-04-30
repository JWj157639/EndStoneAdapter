# API 路由完整指南

## 路由注册

### 无鉴权路由

用于插件自带 WebUI 页面调用，因为页面本身已在 NapCat WebUI 内嵌展示：

```typescript
import type {
    NapCatPluginContext,
    PluginHttpRequest,
    PluginHttpResponse
} from 'napcat-types/napcat-onebot/network/plugin/types';

export function registerApiRoutes(ctx: NapCatPluginContext): void {
    const router = ctx.router;

    // GET 请求
    router.getNoAuth('/status', (req: PluginHttpRequest, res: PluginHttpResponse) => {
        res.json({ code: 0, data: { status: 'ok' } });
    });

    // POST 请求
    router.postNoAuth('/config', (req: PluginHttpRequest, res: PluginHttpResponse) => {
        res.json({ code: 0, message: 'ok' });
    });
}
```

访问路径：`/plugin/<plugin-id>/api/status`

### 需要鉴权的路由

用于需要 NapCat WebUI 登录认证的接口：

```typescript
router.get('/admin', (req: PluginHttpRequest, res: PluginHttpResponse) => {
    res.json({ code: 0, data: 'admin data' });
});
```

访问路径：`/api/Plugin/ext/<plugin-id>/admin`

## 路由方法

### GET 路由

```typescript
router.getNoAuth('/status', (_req, PluginHttpRequest, res: PluginHttpResponse) => {
    res.json({ 
        code: 0, 
        data: {
            pluginName: ctx.pluginName,
            uptime: pluginState.getUptime(),
            config: pluginState.config,
            stats: pluginState.stats,
        },
    });
});
```

### POST 路由

```typescript
router.postNoAuth('/config', async (req: PluginHttpRequest, res: PluginHttpResponse) => {
    try {
        const body = req.body as Record<string, unknown>;
        
        // 参数验证
        if (!body) {
            return res.status(400).json({ 
                code: -1, 
                message: '请求体为空' 
            });
        }
        
        // 处理数据
        pluginState.updateConfig(body as Partial<PluginConfig>);
        ctx.logger.info('配置已保存');
        
        res.json({ 
            code: 0, 
            message: 'ok',
            data: body
        });
    } catch (err) {
        ctx.logger.error('保存配置失败:', err);
        res.status(500).json({ 
            code: -1, 
            message: '保存配置失败',
            error: String(err)
        });
    }
});
```

### 路径参数

```typescript
router.getNoAuth('/users/:id', (req: PluginHttpRequest, res: PluginHttpResponse) => {
    const userId = req.params?.id;
    
    if (!userId) {
        return res.status(400).json({ 
            code: -1, 
            message: '缺少用户 ID' 
        });
    }
    
    // 处理逻辑
    const userData = getUserData(userId);
    
    res.json({ 
        code: 0, 
        data: userData 
    });
});

// 访问：/plugin/<plugin-id>/api/users/123
```

### 查询参数

```typescript
router.getNoAuth('/search', (req: PluginHttpRequest, res: PluginHttpResponse) => {
    const query = req.query;
    const keyword = query?.keyword as string;
    const page = Number(query?.page) || 1;
    const limit = Number(query?.limit) || 10;
    
    // 处理逻辑
    const results = search(keyword, page, limit);
    
    res.json({ 
        code: 0, 
        data: results 
    });
});

// 访问：/plugin/<plugin-id>/api/search?keyword=test&page=1&limit=10
```

### PUT 路由

```typescript
router.putNoAuth('/users/:id', (req: PluginHttpRequest, res: PluginHttpResponse) => {
    const userId = req.params?.id;
    const body = req.body as Record<string, unknown>;
    
    // 更新逻辑
    const updated = updateUser(userId, body);
    
    res.json({ 
        code: 0, 
        data: updated 
    });
});
```

### DELETE 路由

```typescript
router.deleteNoAuth('/users/:id', (req: PluginHttpRequest, res: PluginHttpResponse) => {
    const userId = req.params?.id;
    
    // 删除逻辑
    deleteUser(userId);
    
    res.json({ 
        code: 0, 
        message: '删除成功' 
    });
});
```

## 响应格式

### 成功响应

```typescript
res.json({ 
    code: 0, 
    message: '操作成功',
    data: { /* 数据 */ } 
});
```

### 错误响应

```typescript
// 400 Bad Request
res.status(400).json({ 
    code: -1, 
    message: '参数错误' 
});

// 404 Not Found
res.status(404).json({ 
    code: -1, 
    message: '资源不存在' 
});

// 500 Internal Server Error
res.status(500).json({ 
    code: -1, 
    message: '服务器错误',
    error: String(err)
});
```

## 常用 API 模式

### 获取群列表

```typescript
router.getNoAuth('/groups', async (_req: PluginHttpRequest, res: PluginHttpResponse) => {
    try {
        const groups = await ctx.actions.call(
            'get_group_list',
            {},
            ctx.adapterName,
            ctx.pluginManager.config
        ) as Array<{ 
            group_id: number; 
            group_name: string; 
            member_count: number; 
            max_member_count: number; 
        }>;

        const groupsWithConfig = (groups || []).map((group) => {
            const groupId = String(group.group_id);
            return {
                group_id: group.group_id,
                group_name: group.group_name,
                member_count: group.member_count,
                max_member_count: group.max_member_count,
                enabled: pluginState.isGroupEnabled(groupId),
            };
        });

        res.json({ code: 0, data: groupsWithConfig });
    } catch (e) {
        ctx.logger.error('获取群列表失败:', e);
        res.status(500).json({ code: -1, message: String(e) });
    }
});
```

### 批量操作

```typescript
router.postNoAuth('/groups/bulk-config', async (req: PluginHttpRequest, res: PluginHttpResponse) => {
    try {
        const body = req.body as Record<string, unknown>;
        const { enabled, groupIds } = body || {};

        if (typeof enabled !== 'boolean' || !Array.isArray(groupIds)) {
            return res.status(400).json({ 
                code: -1, 
                message: '参数错误：enabled 需为布尔值，groupIds 需为数组' 
            });
        }

        // 批量更新
        for (const groupId of groupIds) {
            pluginState.updateGroupConfig(String(groupId), { enabled });
        }

        ctx.logger.info(`批量更新群配置完成 | 数量: ${groupIds.length}, enabled=${enabled}`);
        res.json({ 
            code: 0, 
            message: 'ok',
            data: { updatedCount: groupIds.length }
        });
    } catch (err) {
        ctx.logger.error('批量更新群配置失败:', err);
        res.status(500).json({ code: -1, message: String(err) });
    }
});
```

### 文件上传

```typescript
import formidable from 'formidable';

router.postNoAuth('/upload', async (req: PluginHttpRequest, res: PluginHttpResponse) => {
    try {
        const form = formidable({ multiples: true });
        const [fields, files] = await form.parse(req);
        
        // 处理文件
        const uploadedFile = files.file as formidable.File;
        // 保存文件...
        
        res.json({ 
            code: 0, 
            message: '上传成功',
            data: { filename: uploadedFile.originalFilename }
        });
    } catch (err) {
        res.status(500).json({ code: -1, message: '上传失败' });
    }
});
```

## 调用 OneBot API

### 调用 API

```typescript
// 获取好友列表
const friends = await ctx.actions.call(
    'get_friend_list',
    {},
    ctx.adapterName,
    ctx.pluginManager.config
);

// 获取群成员信息
const member = await ctx.actions.call(
    'get_group_member_info',
    { group_id: '123456', user_id: '789012' },
    ctx.adapterName,
    ctx.pluginManager.config
);

// 设置群成员名片
await ctx.actions.call(
    'set_group_card',
    { group_id: '123456', user_id: '789012', card: '新名片' },
    ctx.adapterName,
    ctx.pluginManager.config
);

// 禁言成员
await ctx.actions.call(
    'set_group_ban',
    { group_id: '123456', user_id: '789012', duration: 1800 },
    ctx.adapterName,
    ctx.pluginManager.config
);

// 踢出成员
await ctx.actions.call(
    'set_group_kick',
    { group_id: '123456', user_id: '789012', reject_add_request: false },
    ctx.adapterName,
    ctx.pluginManager.config
);
```

### API 列表

常用 OneBot 11 API：

| API | 功能 | 参数 |
|-----|------|------|
| get_login_info | 获取登录号信息 | - |
| get_friend_list | 获取好友列表 | - |
| get_group_list | 获取群列表 | - |
| get_group_member_info | 获取群成员信息 | group_id, user_id |
| get_group_member_list | 获取群成员列表 | group_id |
| send_msg | 发送消息 | message_type, group_id/user_id, message |
| send_group_msg | 发送群消息 | group_id, message |
| send_private_msg | 发送私聊消息 | user_id, message |
| set_group_card | 设置群成员名片 | group_id, user_id, card |
| set_group_ban | 禁言成员 | group_id, user_id, duration |
| set_group_kick | 踢出成员 | group_id, user_id |
| set_group_admin | 设置管理员 | group_id, user_id, enable |

更多 API 参考：[OneBot 11 API 文档](https://github.com/botuniverse/onebot-11/blob/master/api/public.md)

## 错误处理

### 统一错误处理

```typescript
function handleError(res: PluginHttpResponse, err: unknown, message: string = '操作失败'): void {
    ctx.logger.error(message, err);
    res.status(500).json({ 
        code: -1, 
        message: message,
        error: err instanceof Error ? err.message : String(err)
    });
}

// 使用
router.postNoAuth('/config', async (req: PluginHttpRequest, res: PluginHttpResponse) => {
    try {
        // 处理逻辑
    } catch (err) {
        handleError(res, err, '保存配置失败');
    }
});
```

### 参数验证

```typescript
function validateBody(body: unknown, requiredFields: string[]): boolean {
    if (!body || typeof body !== 'object') return false;
    const obj = body as Record<string, unknown>;
    return requiredFields.every(field => field in obj && obj[field] !== undefined);
}

// 使用
router.postNoAuth('/config', (req: PluginHttpRequest, res: PluginHttpResponse) => {
    const body = req.body as Record<string, unknown>;
    
    if (!validateBody(body, ['enabled', 'debug'])) {
        return res.status(400).json({ 
            code: -1, 
            message: '缺少必要参数' 
        });
    }
    
    // 处理逻辑
});
```

## 路由组织

### 按功能分组

```typescript
export function registerApiRoutes(ctx: NapCatPluginContext): void {
    const router = ctx.router;

    // ==================== 配置管理 ====================
    
    router.getNoAuth('/config', getConfig);
    router.postNoAuth('/config', updateConfig);
    router.postNoAuth('/config/restore', restoreConfig);

    // ==================== 群管理 ====================
    
    router.getNoAuth('/groups', getGroups);
    router.getNoAuth('/groups/:id', getGroup);
    router.postNoAuth('/groups/:id/config', updateGroupConfig);
    router.postNoAuth('/groups/bulk-config', bulkUpdateGroupConfig);

    // ==================== 统计信息 ====================
    
    router.getNoAuth('/stats', getStats);
    router.getNoAuth('/stats/daily', getDailyStats);

    // ==================== 插件信息 ====================
    
    router.getNoAuth('/status', getStatus);
    router.getNoAuth('/version', getVersion);
}
```

## 中间件

### 日志中间件

```typescript
function logMiddleware(req: PluginHttpRequest, res: PluginHttpResponse, next: () => void) {
    const start = Date.now();
    
    // 记录请求
    ctx.logger.info(`${req.method} ${req.path}`);
    
    // 记录响应
    const originalJson = res.json.bind(res);
    res.json = function(data) {
        const duration = Date.now() - start;
        ctx.logger.info(`${req.method} ${req.path} - ${duration}ms`);
        return originalJson(data);
    };
    
    next();
}
```

### 认证中间件

```typescript
function authMiddleware(req: PluginHttpRequest, res: PluginHttpResponse, next: () => void) {
    const token = req.headers?.authorization;
    
    if (!token) {
        return res.status(401).json({ 
            code: -1, 
            message: '未授权' 
        });
    }
    
    // 验证 token
    if (!validateToken(token)) {
        return res.status(403).json({ 
            code: -1, 
            message: '无效的 token' 
        });
    }
    
    next();
}
```