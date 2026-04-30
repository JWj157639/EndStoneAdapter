# 构建与部署完整指南

## 开发环境设置

### 安装依赖

```bash
# 安装项目依赖
pnpm install

# 安装 WebUI 依赖
cd src/webui
pnpm install
cd ../..
```

### 环境要求

- Node.js >= 16
- pnpm >= 8
- TypeScript >= 5

## 开发模式

### 热重载开发

```bash
# 启动开发模式（监听文件变化）
pnpm run dev
```

开发模式特点：
- 自动监听 TypeScript 文件变化
- 自动编译到 `dist/` 目录
- 支持热重载，修改代码后自动重新加载插件

### WebUI 开发

```bash
# 进入 WebUI 目录
cd src/webui

# 启动开发服务器
pnpm run dev
```

WebUI 开发服务器：
- 端口：默认 5173
- 代理：自动代理 `/api` 和 `/plugin` 请求到 NapCat 后端
- 热更新：修改 React 组件后自动刷新

## 生产构建

### 完整构建

```bash
# 构建 WebUI
cd src/webui && pnpm run build && cd ../..

# 构建插件
pnpm run build
```

### 单独构建 WebUI

```bash
cd src/webui
pnpm run build
```

### 单独构建插件

```bash
pnpm run build
```

## 构建输出

### 插件输出

```
dist/
├── index.js              # 插件主文件
├── index.js.map          # 源码映射
├── core/                 # 核心模块
├── handlers/             # 处理器
├── modules/              # 功能模块
├── services/             # 服务
├── utils/                # 工具
└── types/                # 类型
```

### WebUI 输出

```
src/webui/dist/
└── index.html            # 单文件 HTML（包含所有 CSS 和 JS）
```

## 构建配置

### Vite 配置（插件）

```typescript
// vite.config.ts
import { defineConfig } from 'vite';
import { resolve } from 'path';

export default defineConfig({
    plugins: [],
    build: {
        outDir: 'dist',
        emptyOutDir: true,
        sourcemap: true,
        rollupOptions: {
            input: {
                index: resolve(__dirname, 'src/index.ts')
            },
            output: {
                entryFileNames: '[name].js',
                chunkFileNames: '[name].js',
                assetFileNames: '[name].[ext]'
            }
        }
    }
});
```

### Vite 配置（WebUI）

```typescript
// src/webui/vite.config.ts
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { viteSingleFile } from 'vite-plugin-singlefile';
import { resolve } from 'path';

export default defineConfig({
    plugins: [react(), viteSingleFile()],
    base: './',
    build: {
        outDir: 'dist',
        emptyOutDir: true,
        cssCodeSplit: false,
        assetsInlineLimit: 100000000,
        rollupOptions: {
            input: {
                main: resolve(__dirname, 'index.html'),
            },
            output: {
                inlineDynamicImports: true,
            },
        },
    },
    server: {
        proxy: {
            '/api': 'http://localhost:6099',
            '/plugin': 'http://localhost:6099',
        },
    },
});
```

## TypeScript 配置

### 主项目配置

```json
{
  "compilerOptions": {
    "target": "ES2020",
    "module": "ESNext",
    "moduleResolution": "node",
    "strict": true,
    "esModuleInterop": true,
    "skipLibCheck": true,
    "forceConsistentCasingInFileNames": true,
    "resolveJsonModule": true,
    "isolatedModules": true,
    "outDir": "./dist",
    "rootDir": "./src"
  },
  "include": ["src/**/*"],
  "exclude": ["node_modules", "dist", "src/webui"]
}
```

### WebUI 配置

```json
{
  "compilerOptions": {
    "target": "ES2020",
    "useDefineForClassFields": true,
    "lib": ["ES2020", "DOM", "DOM.Iterable"],
    "module": "ESNext",
    "skipLibCheck": true,
    "moduleResolution": "bundler",
    "allowImportingTsExtensions": true,
    "resolveJsonModule": true,
    "isolatedModules": true,
    "noEmit": true,
    "jsx": "react-jsx",
    "strict": true,
    "noUnusedLocals": false,
    "noUnusedParameters": false,
    "noFallthroughCasesInSwitch": true
  },
  "include": ["src"],
  "references": [{ "path": "./tsconfig.node.json" }]
}
```

## 调试

### 插件调试

1. **启用调试模式**

在 WebUI 中启用"调试模式"或在配置文件中设置：

```typescript
{
  debug: true
}
```

2. **查看日志**

调试模式下，插件会输出详细日志到 NapCat 控制台。

3. **检查构建输出**

```bash
# 查看编译后的代码
cat dist/index.js
```

### WebUI 调试

1. **浏览器开发工具**

在 WebUI 中使用 F12 打开浏览器开发者工具。

2. **React DevTools**

```bash
cd src/webui
pnpm add -D @types/react-dom
```

3. **Console 日志**

在代码中使用 `console.log` 输出调试信息。

## 常见问题

### 构建失败

```bash
# 清理缓存
rm -rf dist
rm -rf node_modules/.vite
pnpm install
pnpm run build
```

### WebUI 构建失败

```bash
cd src/webui
rm -rf dist
rm -rf node_modules/.vite
pnpm install
pnpm run build
```

### 类型错误

```bash
# 检查类型
pnpm run type-check

# 自动修复部分类型错误
pnpm run lint --fix
```

### 模块找不到

检查 `package.json` 中的依赖：

```bash
# 查看依赖
cat package.json

# 重新安装
pnpm install
```

## 部署

### NapCat 插件部署

1. **构建插件**

```bash
pnpm run build
```

2. **复制到插件目录**

将 `dist/` 目录复制到 NapCat 的插件目录：

```
NapCat/data/plugins/your-plugin-name/
└── (dist 目录内容)
```

3. **重启 NapCat**

重启 NapCat 以加载插件。

### WebUI 部署

WebUI 构建为单文件 HTML，无需单独部署，包含在插件构建产物中。

## 性能优化

### 减小包体积

```typescript
// vite.config.ts
export default defineConfig({
    build: {
        minify: 'terser',
        terserOptions: {
            compress: {
                drop_console: true,  // 生产环境移除 console
                drop_debugger: true
            }
        }
    }
});
```

### 代码分割

```typescript
// 按需导入
import { heavyFunction } from './heavy';
// 而不是
import * as heavy from './heavy';
```

### Tree Shaking

确保使用 ES Module 格式：

```json
{
  "type": "module"
}
```

## CI/CD

### GitHub Actions 示例

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - uses: pnpm/action-setup@v2
        with:
          version: 8
      - run: pnpm install
      - run: cd src/webui && pnpm install && pnpm run build && cd ../..
      - run: pnpm run build
      - uses: actions/upload-artifact@v2
        with:
          name: dist
          path: dist/
```

## 版本管理

### 版本号

在 `package.json` 中维护版本号：

```json
{
  "name": "napcat-plugin-example",
  "version": "1.0.0"
}
```

### 发布流程

1. 更新版本号
2. 更新 CHANGELOG.md
3. 提交代码
4. 打 Tag
5. 构建发布

```bash
# 更新版本
npm version patch  # 1.0.0 -> 1.0.1
npm version minor  # 1.0.0 -> 1.1.0
npm version major  # 1.0.0 -> 2.0.0

# 提交
git add .
git commit -m "Release v1.0.1"

# 打 Tag
git tag v1.0.1
git push origin v1.0.1
```