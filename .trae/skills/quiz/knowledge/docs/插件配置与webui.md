本指南将介绍如何定义配置以及开发 WebUI 界面。

1. 配置文件定义
在 src/index.ts 中，你需要导出配置的 WebUI Schema 和可选的配置读写钩子。

定义配置接口
首先定义 TypeScript 接口，以便在代码中获得类型提示。


// src/types.ts
export interface PluginConfig {
  enable: boolean;
  prefix: string;
  adminList: number[];
}
定义 WebUI Schema
NapCat 使用 PluginConfigSchema 来自动生成配置面板。这是最推荐的配置方式，无需手动编写前端代码即可拥有一个漂亮的配置界面。

通过导出 plugin_config_ui 或 plugin_config_schema 来定义：


// src/index.ts
import { PluginConfigSchema } from 'napcat-types';

export const plugin_config_ui: PluginConfigSchema = [
  {
    key: 'enable',
    label: '启用插件',
    type: 'boolean',
    default: true,
    description: '是否启用本插件的核心功能'
  },
  {
    key: 'prefix',
    label: '命令前缀',
    type: 'string',
    default: '/',
    placeholder: '请输入命令前缀，例如 / 或 #',
    description: '触发命令的前缀符号'
  },
  {
    key: 'adminList',
    label: '管理员列表',
    type: 'multi-select',
    default: [],
    options: [
        { label: '用户A', value: 123456 },
        { label: '用户B', value: 654321 }
    ],
    description: '拥有管理员权限的用户 QQ 号'
  }
];
支持的控件类型
PluginConfigItem 的 type 字段支持以下值：

类型	说明
string	文本输入框
number	数字输入框
boolean	开关
select	下拉单选
multi-select	下拉多选
text	多行文本框
html	自定义 HTML 展示（不保存值）
PluginConfigItem 完整属性

interface PluginConfigItem {
    key: string;                    // 配置项唯一标识
    type: 'string' | 'number' | 'boolean' | 'select' | 'multi-select' | 'html' | 'text';
    label: string;                  // 显示标签
    description?: string;           // 描述说明
    default?: unknown;              // 默认值
    options?: { label: string; value: string | number }[]; // select/multi-select 的选项
    placeholder?: string;           // 输入框占位符
    reactive?: boolean;             // 标记为响应式：值变化时触发 schema 刷新
    hidden?: boolean;               // 是否隐藏此字段
}
使用 NapCatConfig 构建器
除了手动编写 Schema 数组，你还可以使用 ctx.NapCatConfig 提供的静态方法快速构建：


import { NapCatPluginContext, PluginConfigSchema } from 'napcat-types';

export const plugin_init = (ctx: NapCatPluginContext) => {
    // NapCatConfig 提供了便捷的构建方法
    const schema: PluginConfigSchema = ctx.NapCatConfig.combine(
        ctx.NapCatConfig.text('apiKey', 'API Key', '', 'OpenAI API 密钥'),
        ctx.NapCatConfig.number('maxRetries', '最大重试次数', 3),
        ctx.NapCatConfig.boolean('debug', '调试模式', false, '开启后输出详细日志'),
        ctx.NapCatConfig.select('model', '模型选择', [
            { label: 'GPT-3.5', value: 'gpt-3.5-turbo' },
            { label: 'GPT-4', value: 'gpt-4' }
        ], 'gpt-3.5-turbo'),
        ctx.NapCatConfig.multiSelect('features', '启用功能', [
            { label: '翻译', value: 'translate' },
            { label: '摘要', value: 'summary' }
        ], ['translate']),
        ctx.NapCatConfig.html('<p style="color:gray">这是一段说明文字</p>'),
        ctx.NapCatConfig.plainText('纯文本说明内容')
    );
};
自定义配置读写
默认情况下 NapCat 会自动管理配置读写。如果需要自定义行为，可以导出以下钩子：


import type { PluginModule } from 'napcat-types';
import { PluginConfig } from './types';

// 自定义配置读取
export const plugin_get_config: PluginModule['plugin_get_config'] = async (ctx) => {
    // 从自定义位置读取配置
    return loadMyConfig(ctx.configPath);
};

// 自定义配置保存
export const plugin_set_config: PluginModule['plugin_set_config'] = async (ctx, config) => {
    saveMyConfig(ctx.configPath, config);
};
配置 UI 动态控制
通过 plugin_config_controller 可以在运行时动态修改配置界面：


import type { PluginModule, PluginConfigUIController } from 'napcat-types';

export const plugin_config_controller: PluginModule['plugin_config_controller'] = async (_ctx, ui, initialConfig) => {
    // 根据初始配置动态显示/隐藏字段
    if (initialConfig.mode === 'simple') {
        ui.hideField('advancedOption');
    }

    // 返回清理函数（可选）
    return () => {
        // 清理资源
    };
};

// 配置变更时的回调
export const plugin_on_config_change: PluginModule['plugin_on_config_change'] = async (_ctx, ui, key, value, _currentConfig) => {
    // 当 mode 字段变化时，动态调整 UI
    if (key === 'mode') {
        if (value === 'advanced') {
            ui.showField('advancedOption');
        } else {
            ui.hideField('advancedOption');
        }
    }
};
PluginConfigUIController 提供以下方法：

方法	说明
updateSchema(schema)	更新整个配置 Schema
updateField(key, field)	更新指定字段的部分属性
removeField(key)	移除指定字段
addField(field, afterKey?)	添加新字段（可指定位置）
showField(key)	显示指定字段
hideField(key)	隐藏指定字段
getCurrentConfig()	获取当前配置值
2. 在 WebUI 中使用自定义前端
如果你需要更复杂的交互界面（如数据可视化、文件管理等），可以使用 React/Vue 开发完整的 SPA 页面。napcat-plugin-template 的 webui 目录正是为此准备的。

开启 WebUI 路由
在 src/index.ts 中注册 API 和静态资源：


// src/index.ts
import type { PluginModule } from "napcat-types";
import { registerApiRoutes } from './services/api-service';

export const plugin_init: PluginModule['plugin_init'] = async (ctx) => {
    // 1. 注册后端 API (供前端调用)
    registerApiRoutes(ctx);

    // 2. 托管静态文件 (前端 build 产物)
    // 假设你的前端 build output 在插件目录下的 webui/dist
    // 访问路径: http://host:port/plugin/<plugin-id>/files/<urlPath>/
    ctx.router.static('/static', 'webui/dist');
    
    // 3. 注册侧边栏页面入口 (可选)
    // 访问路径: http://host:port/plugin/<plugin-id>/page/dashboard
    ctx.router.page({
        title: '我的插件',
        path: 'dashboard',                // 路由路径
        htmlFile: 'webui/dist/index.html', // 入口 HTML 文件
        icon: '🔌',                       // 页面图标
        description: '插件管理面板'        // 页面描述
    });

    // 4. 也可以使用内存静态文件（适合动态生成内容）
    // 访问路径: http://host:port/plugin/<plugin-id>/mem/dynamic/config.json
    ctx.router.staticOnMem('/dynamic', [
        {
            path: '/config.json',
            content: () => JSON.stringify({ version: '1.0.0' }),
            contentType: 'application/json'
        }
    ]);
};
开发 WebUI 前端
模板中的 src/webui 是一个标准的 React + Vite 项目。

启动开发服务: 进入 src/webui 运行 npm install && npm run dev。
调用后端接口: 使用 fetch 或 axios 调用你在插件中注册的 API。
路由前缀说明
路由类型	前缀	注册方法
鉴权 API	/api/Plugin/ext/<plugin-id>/	ctx.router.get/post/put/delete/api
无鉴权 API	/plugin/<plugin-id>/api/	ctx.router.getNoAuth/postNoAuth/putNoAuth/deleteNoAuth/apiNoAuth
静态文件	/plugin/<plugin-id>/files/<urlPath>/	ctx.router.static(urlPath, localPath)
内存文件	/plugin/<plugin-id>/mem/<urlPath>/	ctx.router.staticOnMem(urlPath, files)
扩展页面	/plugin/<plugin-id>/page/<path>	ctx.router.page(definition)
建议在前端封装请求工具，自动处理前缀。

前后端通信示例
后端 (node):


// src/services/api-service.ts
import type { NapCatPluginContext } from "napcat-types";

export function registerApiRoutes(ctx: NapCatPluginContext) {
    // 注册需要鉴权的 GET 接口
    // 访问: /api/Plugin/ext/<plugin-id>/status
    ctx.router.get('/status', (_req, res) => {
        res.json({ status: 'ok', time: Date.now() });
    });

    // 注册无需鉴权的 POST 接口
    // 访问: /plugin/<plugin-id>/api/webhook
    ctx.router.postNoAuth('/webhook', async (req, res) => {
        const data = req.body;
        ctx.logger.info('收到 webhook:', data);
        res.status(200).json({ received: true });
    });
}
前端 (react):


// src/webui/src/App.tsx
useEffect(() => {
    // 假设封装了 fetcher 自动添加前缀
    fetcher.get('/status').then(data => {
        console.log('插件状态:', data);
    });
}, []);
3. 构建与发布
当你运行插件根目录的 npm run build 时，构建脚本通常会：

构建插件 TypeScript 代码 (src/index.ts 等)。
构建 WebUI 前端代码 (src/webui)。
将前端产物 (dist) 复制到插件输出目录 (dist/webui/dist)。
打包为 napcat-plugin-xxx.zip 或直接发布文件夹。
确保 package.json 中的 files 字段包含了所有必要的文件。