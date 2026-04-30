# 哔哩哔哩视频分享处理器

## 功能概述

自动识别群聊中的哔哩哔哩视频分享链接,解析视频并发送到群中,无需手动点击链接观看。

## 核心功能

### 1. 消息识别
- 通过 JSON 消息特征匹配识别哔哩哔哩分享
- 主要特征: `title === "哔哩哔哩"`
- 辅助特征(至少匹配2个):
  - `appid === "1109937557"`
  - `app === "com.tencent.miniapp_01"`
  - `view === "view_8C8E89B49BE609866298ADDFF2DBABA4"`
  - `shareTemplateId === "8C8E89B49BE609866298ADDFF2DBABA4"`

### 2. 链接提取
- 从 `meta.detail_1.qqdocurl` 提取分享链接
- 自动提取 b23.tv 短码用于 API 调用

### 3. 视频解析
- 调用第三方 API: `https://api.bugpk.com/api/bilibili`
- 支持超时控制(10秒)
- 优先使用 `data.video_url`,回退到 `data.url`

### 4. 视频发送
- 使用 NapCat 的 `video` 消息类型
- 自动包含视频封面图

## 权限控制

### 使用条件
1. **功能开关**: `bilibiliHandlerEnabled` 配置项必须启用
2. **群白名单**: 群必须在 `groupWhitelist` 配置中
3. **卡密授权**: 群必须通过卡密授权(当 `authSystemEnabled` 启用时)

### 主人特权
- 无需白名单
- 无需卡密授权
- 可在任何群使用

### 授权系统关闭时
当 `authSystemEnabled: false`:
- 只需要群白名单
- 不需要卡密授权

## 配置项

| 配置项 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| `bilibiliHandlerEnabled` | boolean | true | 是否启用哔哩哔哩视频解析功能 |
| `authSystemEnabled` | boolean | true | 是否启用授权系统 |
| `groupWhitelist` | string[] | [] | 群白名单 |

## 使用示例

### 用户操作
1. 在群聊中分享哔哩哔哩视频链接
2. 机器人自动识别并解析
3. 视频自动发送到群中

### 消息示例
```
[CQ:json,data={"ver":"1.0.0.19","prompt":"[QQ小程序]视频标题","app":"com.tencent.miniapp_01","meta":{"detail_1":{"title":"哔哩哔哩","qqdocurl":"https://b23.tv/xxx"}}}]
```

## 导出的函数

### `isBilibiliShareMessage(message: OB11Message): boolean`
检查消息是否为哔哩哔哩分享消息

### `extractBilibiliUrl(message: OB11Message): string | null`
从消息中提取哔哩哔哩分享链接

### `fetchBilibiliVideoInfo(shareUrl: string, ctx: NapCatPluginContext): Promise<BilibiliApiResponse | null>`
调用 API 解析视频信息

### `sendVideoMessage(ctx, groupId, videoUrl, coverUrl): Promise<void>`
发送视频消息到指定群

### `isGroupEnabled(groupId: string): boolean`
检查群是否在白名单中

### `isGroupAuthorized(groupId: string): boolean`
检查群是否已授权

### `handleBilibiliShareMessage(ctx, event): Promise<void>`
处理哔哩哔哩分享消息的主函数

## 技术细节

### 消息处理流程
1. 接收群聊消息
2. 检查功能是否启用
3. 验证群白名单和授权
4. 识别哔哩哔哩分享
5. 提取分享链接
6. 调用 API 解析
7. 发送视频到群

### 独立处理
- 哔哩哔哩分享消息由 `bilibili-handler` 独占处理
- 不经过指令系统,避免空跑
- 提升性能,减少无用日志

### 错误处理
- API 请求超时(10秒)
- 网络错误处理
- 无效数据检测
- 视频受保护提示

## 日志输出

### 调试模式
- 分享链接信息
- 提取的短码
- API 请求 URL
- API 响应状态
- 返回的数据结构

### 信息日志
- 视频解析成功
- 视频发送成功

### 警告日志
- 无法提取链接
- API 返回无效数据
- 视频受保护

### 错误日志
- API 请求失败
- 请求超时
- 发送视频失败

## 依赖关系

### 内部依赖
- `core/state.ts` - 全局状态管理
- `core/permission-checker.ts` - 权限检查系统

### 外部依赖
- `napcat-types` - NapCat 类型定义
- 第三方 API: `https://api.bugpk.com/api/bilibili`

## 注意事项

1. **API 限制**: 第三方 API 可能有调用限制
2. **视频保护**: 部分视频可能无法获取直接链接
3. **网络环境**: 需要稳定的网络连接
4. **权限管理**: 合理配置白名单和授权
5. **日志级别**: 生产环境建议关闭调试日志

## 故障排查

### 问题: 视频未自动发送
1. 检查 `bilibiliHandlerEnabled` 是否启用
2. 检查群是否在白名单中
3. 检查群是否已授权(如启用授权系统)
4. 查看日志确认消息是否被识别

### 问题: API 解析失败
1. 检查网络连接
2. 查看日志中的 API 响应状态
3. 确认分享链接格式是否正确
4. 检查是否为受保护视频

### 问题: 视频无法播放
1. 确认视频 URL 有效性
2. 检查视频是否受地区限制
3. 确认 NapCat 视频发送功能正常