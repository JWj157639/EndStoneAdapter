# Puppeteer 集成说明

## 概述

本插件集成了 Puppeteer 功能，用于将 HTML 模板渲染为图片，支持入群欢迎图片、今日运势卡片等功能。

## 功能特性

- ✅ HTML 模板渲染为图片
- ✅ 支持模板变量插值
- ✅ 自定义视口大小
- ✅ Base64 图片输出
- ✅ 自动降级机制（Puppeteer 不可用时使用文本）
- ✅ 多种模板支持（入群身份、今日运势等）

## 前置要求

### 1. 安装 Puppeteer 插件

确保已安装并运行 `napcat-plugin-puppeteer` 插件。

**插件下载**:
- GitHub: https://github.com/NapNeko/NapCatQQ
- 或从 NapCat 插件市场安装

**服务地址**: `http://localhost:6099`

### 2. 检查服务状态

访问 Puppeteer WebUI 检查服务状态：
```
http://localhost:6099
```

查看"系统状态"确认：
- 浏览器是否连接
- 服务是否正常运行

## API 接口

### 核心渲染接口

#### /render（HTML 模板渲染）

**请求方式**: POST

**请求体**:
```json
{
  "html": "<div>Hello {{name}}</div>",
  "data": {
    "name": "World"
  },
  "encoding": "base64"
}
```

**参数说明**:

| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| html | string | YES | HTML 模板字符串 |
| data | object | NO | Handlebars 模板插值数据 |
| encoding | string | NO | 编码方式，默认 "base64" |

**响应**:
```json
{
  "code": 0,
  "data": "Base64String...",  // Base64 编码的图片数据
  "message": "OK",
  "time": 150              // 耗时(ms)
}
```

### 其他接口

#### /screenshot（通用截图）

**请求方式**: POST

**请求体**:
```json
{
  "file": "url | htmlString | file | auto",
  "file_type": "url | htmlString | file | auto",
  "selector": "body",
  "omitBackground": false,
  "data": {},
  "waitSelector": "",
  "setViewport": {
    "width": 1280,
    "height": 800
  }
}
```

#### /browser/status（浏览器状态）

**请求方式**: GET

**响应**:
```json
{
  "code": 0,
  "data": {
    "connected": true,
    "version": "Chrome...",
    "pageCount": 1,
    "pid": 12345,
    "executablePath": "..."
  }
}
```

## 模板文件

### 入群身份模板

**文件位置**: `dist/html/入群身份.html`

**模板变量**:
```handlebars
{{qq}}      - 用户QQ号
{{name}}    - 用户昵称
{{sex}}     - 性别
{{rrrr}}    - 生日（年-月-日）
{{age}}     - 年龄
{{denji}}   - QQ等级
{{zhuce}}   - 注册时间
{{jiaqun}}   - 加群时间
```

**渲染尺寸**: 1400 x 850

**特点**:
- 手写艺术字体
- 随机颜色边框（每次渲染不同）
- 响应式布局
- QQ头像显示

### 今日运势模板

**文件位置**: `dist/html/运势.html`（如果存在）

**模板变量**: 根据具体实现而定

## 使用方法

### 1. 配置插件

在 WebUI 或配置文件中启用 HTML 渲染：

**配置文件**:
```json
{
  "useHtmlRendering": true
}
```

**WebUI**:
1. 进入 QTBOT 管理面板
2. 进入"系统设置"
3. 开启"启用HTML渲染"

### 2. 开启事件

开启需要使用图片功能的事件：

**入群图片**:
```
开启入群图片
```

**今日运势**:
```
开启今日运势
```

### 3. 查看事件状态

```
事件管理
```

## 代码实现

### 导入服务

```typescript
import { renderTemplateToImage } from '../services/puppeteer-service';
```

### 渲染模板

```typescript
// 构建渲染数据
const renderData = {
  qq: String(userId),
  name: String(userInfo.nick || ''),
  sex: String(genderMap[userInfo.sex] || '未知'),
  // ... 其他数据
};

// 调用渲染
const imageData = await renderTemplateToImage(
  '入群身份',  // 模板名称（不含.html扩展名）
  renderData,
  { width: 1400, height: 850 }
);

if (imageData) {
  // 发送图片
  const message = `[CQ:image,file=base64://${imageData}]`;
  await ctx.actions.call('send_group_msg', {
    group_id: groupId,
    message: message
  }, ...);
}
```

### 自动降级

```typescript
if (!pluginState.config.useHtmlRendering) {
  // HTML 渲染未启用，使用文本消息
  await sendTextWelcome(event);
  return;
}

if (imageData) {
  // 渲染成功，发送图片
  await sendImageWelcome(event, imageData);
} else {
  // 渲染失败，降级为文本消息
  await sendTextWelcome(event);
}
```

## 服务接口说明

### 无认证 API（供其他插件调用）

#### 路径格式
```
{host}/plugin/napcat-plugin-puppeteer/api/{endpoint}
```

#### 端点列表

| 端点 | 方法 | 说明 |
|------|------|------|
| `/render` | POST | HTML 模板渲染 |
| `/screenshot` | POST | 通用截图 |
| `/browser/status` | GET | 浏览器状态 |

### 需认证 API（WebUI 管理）

#### 路径格式
```
{host}/api/Plugin/ext/napcat-plugin-puppeteer/{endpoint}
```

#### 端点列表

| 端点 | 方法 | 说明 |
|------|------|------|
| `/browser/start` | POST | 启动浏览器 |
| `/browser/stop` | POST | 停止浏览器 |
| `/browser/restart` | POST | 重启浏览器 |
| `/config/status` | GET | 获取配置 |
| `/config` | POST | 更新配置 |

## 调试技巧

### 1. 检查服务状态

```typescript
const response = await fetch('http://localhost:6099/plugin/napcat-plugin-puppeteer/api/browser/status');
const result = await response.json();
console.log('Puppeteer 状态:', result);
```

### 2. 测试渲染

```typescript
const testData = {
  html: '<div style="padding:20px;background:#fff;"><h1>测试</h1></div>',
  data: {},
  encoding: 'base64'
};

const response = await fetch('http://localhost:6099/plugin/napcat-plugin-puppeteer/api/render', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify(testData)
});

const result = await response.json();
console.log('渲染结果:', result);
```

### 3. 查看日志

开启调试模式查看详细日志：

**配置文件**:
```json
{
  "debug": true
}
```

**日志输出**:
```
[DEBUG] [EventHandler] 执行入群图片: 群号=749590057, 用户=2028467646
[DEBUG] [Function] 调用 Puppeteer 服务: ...
```

## 错误处理

### 常见错误

#### 1. HTML内容为空
```
[ERROR] HTML内容为空: 入群身份
```
**原因**: 模板文件不存在或读取失败
**解决**: 确认 `dist/html/` 目录下有对应的模板文件

#### 2. Puppeteer 服务连接失败
```
[ERROR] 调用 Puppeteer 服务失败: connect ECONNREFUSED
```
**原因**: Puppeteer 插件未启动或端口错误
**解决**: 
- 检查 Puppeteer 插件是否安装
- 确认服务运行在 `http://localhost:6099`
- 检查防火墙设置

#### 3. 渲染超时
```
[ERROR] Puppeteer 渲染失败: timeout
```
**原因**: 渲染时间过长或网络问题
**解决**: 
- 检查 HTML 模板是否过于复杂
- 检查网络连接
- 增加超时时间

#### 4. 内存不足
```
[ERROR] Puppeteer 渲染失败: JavaScript heap out of memory
```
**原因**: 内存不足
**解决**:
- 减小视口尺寸
- 简化 HTML 模板
- 重启 Puppeteer 服务

### 错误处理示例

```typescript
try {
  const imageData = await renderTemplateToImage('入群身份', renderData);
  
  if (imageData) {
    // 渲染成功
    await sendImage(groupId, imageData);
  } else {
    // 渲染失败，使用备用方案
    ctx.logger.warn('Puppeteer 渲染失败，使用文本欢迎消息');
    await sendTextWelcome(groupId, userInfo);
  }
} catch (error) {
  ctx.logger.error('处理入群图片失败:', error);
  // 发送简单欢迎消息
  await ctx.actions.call('send_group_msg', {
    group_id: groupId,
    message: `[CQ:at,qq=${userId}] 欢迎加入群聊～`
  }, ...);
}
```

## 性能优化

### 1. 减少视口尺寸

```typescript
const imageData = await renderTemplateToImage(
  '入群身份',
  renderData,
  { width: 800, height: 600 }  // 较小的尺寸渲染更快
);
```

### 2. 简化 HTML 模板

- 移除不必要的 CSS 劳画
- 减少外部资源引用
- 使用纯 CSS 样式

### 3. 批量处理

对于需要生成多张图片的场景，使用队列控制并发数：

```typescript
const queue = [];
for (const item of items) {
  queue.push(renderTemplateToImage(item.template, item.data));
}

// 每次最多同时处理 3 个
const results = [];
for (let i = 0; i < queue.length; i += 3) {
  const batch = queue.slice(i, i + 3);
  const batchResults = await Promise.all(batch);
  results.push(...batchResults);
}
```

## 注意事项

1. **服务依赖**: 所有 HTML 渲染功能依赖 Puppeteer 服务
2. **模板文件**: 确保 HTML 模板文件已打包到 `dist/html/` 目录
3. **配置开关**: 需要同时开启 `useHtmlRendering` 配置和对应事件
4. **网络连接**: Puppeteer 服务必须可访问外部资源（字体、图片等）
5. **内存管理**: 大量渲染可能占用较多内存，注意控制并发
6. **错误处理**: 始终提供降级方案（文本消息）
7. **调试模式**: 开启调试模式查看详细日志

## 扩展功能建议

1. **更多模板**: 添加更多 HTML 模板（如生日祝福、节日卡片等）
2. **模板编辑器**: 提供在线编辑 HTML 模板的功能
3. **图片缓存**: 缓存渲染结果，避免重复渲染
4. **模板预览**: 在 WebUI 中预览模板渲染效果
5. **批量生成**: 支持批量生成多张图片
6. **自定义字体**: 支持使用自定义字体
7. **动态尺寸**: 根据内容动态调整渲染尺寸
8. **水印功能**: 在图片上添加水印

## 更新日志

### v1.2.0 (2026-02-26)
- 修复 Puppeteer API 调用错误（使用正确的 /render 端点）
- 实现入群图片事件处理
- 添加自动降级机制
- 添加 Puppeteer 服务集成
- 添加 HTML 模板支持
- 添加调试日志支持