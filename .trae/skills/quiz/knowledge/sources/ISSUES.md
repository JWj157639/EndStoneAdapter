# 问题解决记录

## 问题 1：ZIP 文件上传后解压失败 - "invalid comment length"

### 现象
上传 ZIP 文件后报错：`Error: invalid comment length. expected: 3. found: 391`

### 原因
`parseMultipartFormData` 函数使用 `body.toString('binary')` 将二进制数据转换为字符串，导致 ZIP 文件损坏。

### 解决方案
使用纯 Buffer 操作，不转换为字符串：

```typescript
// ❌ 错误做法
const parts = body.toString('binary').split(boundaryLine);
const data = Buffer.from(content);

// ✅ 正确做法
const boundaryBuffer = Buffer.from(`--${boundary}\r\n`);
const nextBoundaryIndex = body.indexOf(boundaryBuffer, offset);
const partData = body.slice(offset, nextBoundaryIndex);
```

---

## 问题 2：ZIP 文件头搜索失败 - "文件已经是纯 ZIP 格式，无需清理"

### 现象
日志显示 `文件已经是纯 ZIP 格式，无需清理`，但实际文件有 HTTP 协议头污染。

### 原因
使用 `indexOf('PK')` 搜索 ZIP 文件头，在某些情况下无法正确搜索二进制数据。

### 解决方案
使用正确的二进制签名：

```typescript
// ❌ 错误做法
const pkIndex = zipBuffer.indexOf('PK');

// ✅ 正确做法
const zipSignature = Buffer.from([0x50, 0x4B, 0x03, 0x04]);
const pkIndex = zipBuffer.indexOf(zipSignature);
```

---

## 问题 3：首次上传插件失败 - "Plugin not found"

### 现象
首次上传 ZIP 文件后，插件目录被正确解压，但加载失败，日志显示 `Plugin napcat-plugin-mkbot not found`。

### 原因
NapCat 的新插件默认是**禁用状态**，必须先启用才能加载。

### 解决方案
按照 `napcat-plugin-debug-cli` 官方实现，执行三步操作：

```typescript
// 步骤1: 注册插件目录（参数是目录名，不是完整路径）
await pm.loadDirectoryPlugin(pluginId);

// 步骤2: 启用插件（新插件默认禁用）
try {
  await pm.setPluginStatus(pluginId, true);
} catch (statusError) {
  // 即使失败也继续
}

// 步骤3: 加载插件
try {
  await pm.loadPluginById(pluginId);
} catch (loadError) {
  // 即使失败也继续
}
```

### 关键点
1. `loadDirectoryPlugin` 参数是**目录名**（相对路径），不是完整路径
2. 新注册的插件默认禁用，必须 `setPluginStatus(pluginId, true)` 启用
3. 启用后才能 `loadPluginById` 加载
4. 步骤2和3用 try-catch 包裹，即使失败也继续流程

---

## 完整的上传流程

```
用户上传 ZIP
    ↓
接收 multipart 数据
    ↓
纯 Buffer 解析（不转字符串） ✅
    ↓
清理 HTTP 协议头（二进制签名搜索） ✅
    ↓
解压 ZIP 到插件目录
    ↓
检查插件是否已注册
    ├─ 已注册 → reloadPlugin
    └─ 未注册 → 三步走：
        1. loadDirectoryPlugin(pluginId)
        2. setPluginStatus(pluginId, true)
        3. loadPluginById(pluginId)
    ↓
成功！
```

---

## 参考资源

- [napcat-plugin-debug-cli](./node_modules/napcat-plugin-debug-cli/cli/vite.mjs) - 官方首次加载实现
- [napcat-plugin-debug](./napcat-plugin-debug-master/) - NapCat 插件调试服务