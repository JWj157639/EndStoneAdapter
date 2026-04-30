# WebUI 完整指南

## WebUI 技术栈

- **框架**: React 18
- **样式**: TailwindCSS
- **构建工具**: Vite
- **语言**: TypeScript
- **打包**: 单文件输出（vite-plugin-singlefile）

## 项目结构

```
src/webui/
├── index.html              # HTML 入口
├── package.json            # 依赖配置
├── vite.config.ts          # Vite 配置
├── tailwind.config.js      # TailwindCSS 配置
├── tsconfig.json           # TypeScript 配置
├── src/
│   ├── main.tsx            # React 入口
│   ├── App.tsx             # 根组件
│   ├── index.css           # 全局样式
│   ├── components/         # 组件
│   │   ├── Header.tsx
│   │   ├── Sidebar.tsx
│   │   └── icons.tsx
│   ├── pages/              # 页面
│   │   ├── ConfigPage.tsx
│   │   ├── GroupsPage.tsx
│   │   └── StatusPage.tsx
│   ├── hooks/              # 自定义 Hook
│   │   ├── useStatus.ts
│   │   ├── useTheme.ts
│   │   └── useToast.ts
│   └── utils/              # 工具函数
│       └── api.ts
└── dist/                   # 构建输出
```

## API 调用

### noAuthFetch

用于调用无需鉴权的 API：

```typescript
// src/webui/src/utils/api.ts
export async function noAuthFetch<T = unknown>(
    path: string, 
    options?: RequestInit
): Promise<ApiResponse<T>> {
    const res = await fetch(`/plugin/${PLUGIN_NAME}/api${path}`, {
        ...options,
        headers: { 'Content-Type': 'application/json', ...options?.headers }
    });
    
    if (!res.ok) {
        throw new Error(`HTTP ${res.status}: ${res.statusText}`);
    }
    
    return res.json();
}
```

### 使用示例

```typescript
import { noAuthFetch } from '../utils/api';

// GET 请求
const { data } = await noAuthFetch<PluginStatus>('/status');
console.log(data?.pluginName, data?.uptime);

// POST 请求
const result = await noAuthFetch('/config', {
    method: 'POST',
    body: JSON.stringify({ enabled: true })
});

// 带类型
interface MyData {
    id: number;
    name: string;
}
const { data } = await noAuthFetch<MyData>('/users/123');
```

## 组件开发

### 状态管理

```typescript
import { useState, useEffect } from 'react';
import { noAuthFetch } from '../utils/api';

export default function StatusPage() {
    const [status, setStatus] = useState<PluginStatus | null>(null);
    const [loading, setLoading] = useState(true);

    useEffect(() => {
        fetchStatus();
    }, []);

    const fetchStatus = async () => {
        setLoading(true);
        try {
            const { data } = await noAuthFetch<PluginStatus>('/status');
            setStatus(data);
        } catch (error) {
            console.error('获取状态失败:', error);
        } finally {
            setLoading(false);
        }
    };

    if (loading) return <div>加载中...</div>;
    if (!status) return <div>加载失败</div>;

    return <div>插件状态: {status.enabled ? '运行中' : '已停用'}</div>;
}
```

### 自定义 Hook

```typescript
// src/webui/src/hooks/useStatus.ts
import { useState, useCallback } from 'react';
import { noAuthFetch } from '../utils/api';
import type { PluginStatus } from '../types';

export function useStatus() {
    const [status, setStatus] = useState<PluginStatus | null>(null);

    const fetchStatus = useCallback(async () => {
        try {
            const { data } = await noAuthFetch<PluginStatus>('/status');
            if (data) setStatus(data);
        } catch (e) {
            console.error('Status fetch failed:', e);
        }
    }, []);

    return { status, fetchStatus };
}
```

### Toast 通知

```typescript
// src/webui/src/hooks/useToast.ts
import { useSyncExternalStore } from 'react';

let toasts: Toast[] = [];
const listeners = new Set<() => void>();

export function addToast(message: string, type: ToastType = 'info') {
    const id = Date.now();
    toasts = [...toasts, { id, message, type }];
    emitChange();
    
    setTimeout(() => {
        toasts = toasts.filter(t => t.id !== id);
        emitChange();
    }, 3000);
}

function subscribe(listener: () => void) {
    listeners.add(listener);
    return () => listeners.delete(listener);
}

function getSnapshot() {
    return toasts;
}

export function useToasts() {
    return useSyncExternalStore(subscribe, getSnapshot);
}

// 使用
import { useToasts, addToast } from '../hooks/useToast';

function MyComponent() {
    const toasts = useToasts();
    
    const handleClick = () => {
        addToast('操作成功', 'success');
    };
    
    return (
        <div>
            {toasts.map(toast => (
                <div key={toast.id}>{toast.message}</div>
            ))}
            <button onClick={handleClick}>点击</button>
        </div>
    );
}
```

## 样式系统

### TailwindCSS 配置

```javascript
// tailwind.config.js
export default {
    content: [
        "./index.html",
        "./src/**/*.{js,ts,jsx,tsx}",
    ],
    darkMode: 'class',
    theme: {
        extend: {
            colors: {
                primary: '#FB7299',
                brand: {
                    50: '#fff1f3',
                    100: '#ffe0e6',
                    // ...
                    500: '#FB7299',
                    // ...
                }
            }
        }
    }
}
```

### 常用样式类

```typescript
// 卡片
<div className="bg-white dark:bg-gray-800 rounded-lg border border-gray-200 dark:border-gray-700 p-4">

// 按钮
<button className="bg-primary text-white px-4 py-2 rounded-lg hover:bg-brand-600 transition">

// 输入框
<input className="w-full px-3 py-2 rounded-lg border border-gray-300 dark:border-gray-600 bg-white dark:bg-gray-900 focus:border-primary focus:ring-2 focus:ring-primary/20 outline-none">

// 开关
<label className="relative inline-flex items-center cursor-pointer">
    <input type="checkbox" className="sr-only" />
    <div className="w-10 h-6 bg-gray-300 dark:bg-gray-600 rounded-full peer peer-checked:bg-primary transition-colors" />
</label>
```

## 构建配置

### Vite 配置

```typescript
// vite.config.ts
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
});
```

## 开发与构建

### 安装依赖

```bash
cd src/webui
pnpm install
```

### 开发模式

```bash
pnpm run dev
```

### 构建生产版本

```bash
pnpm run build
```

## 注册 WebUI

### 在插件中注册

```typescript
// src/index.ts
import type { NapCatPluginContext } from 'napcat-types/napcat-onebot/network/plugin/types';

export const plugin_init: PluginModule['plugin_init'] = async (ctx: NapCatPluginContext) => {
    // 托管前端静态资源
    ctx.router.static('/static', 'webui');

    // 注册仪表盘页面
    ctx.router.page({
        path: 'dashboard',
        title: '插件仪表盘',
        htmlFile: 'webui/index.html',
        description: '插件管理控制台',
    });

    ctx.logger.debug('WebUI 路由注册完成');
};
```

### 访问路径

- 静态文件: `/plugin/<plugin-id>/files/static/*`
- 页面: `/plugin/<plugin-id>/page/dashboard`

## 最佳实践

### 1. 类型定义

```typescript
// src/webui/src/types.ts
export interface PluginConfig {
    enabled: boolean;
    debug: boolean;
    commandPrefix: string;
    // ...
}

export interface PluginStatus {
    pluginName: string;
    uptime: number;
    config: PluginConfig;
    stats: {
        processed: number;
        todayProcessed: number;
    };
}
```

### 2. 错误处理

```typescript
const fetchStatus = async () => {
    try {
        const { data } = await noAuthFetch<PluginStatus>('/status');
        setStatus(data);
    } catch (error) {
        console.error('获取状态失败:', error);
        addToast('获取状态失败', 'error');
    }
};
```

### 3. 加载状态

```typescript
if (loading) {
    return (
        <div className="flex items-center justify-center p-8">
            <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-primary"></div>
        </div>
    );
}
```

### 4. 响应式设计

```typescript
<div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
    {/* 移动端：1列，平板：2列，桌面：4列 */}
</div>
```

### 5. 深色模式

```typescript
import { useEffect } from 'react';

export function useTheme() {
    useEffect(() => {
        const update = () => {
            const isDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
            document.documentElement.classList.toggle('dark', isDark);
        };
        update();
        const mq = window.matchMedia('(prefers-color-scheme: dark)');
        mq.addEventListener('change', update);
        return () => mq.removeEventListener('change', update);
    }, []);
}
```