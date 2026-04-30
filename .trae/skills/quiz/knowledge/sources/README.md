# MKbot 插件

这是一个基于原始MKbot11插件重构的现代化QQ机器人插件，用于NapCat框架。

## 功能特性

### 1. 授权系统
- 卡密管理（天卡、周卡、月卡、半年卡、年卡、永久卡）
- 授权状态查询
- 多种授权模式

### 2. 群管系统
- 禁言/解禁功能
- 踢人/拉黑功能
- 头官管理
- 入群审核
- 黑名单系统
- 邒饭群员清理

### 3. 娱乐系统
- 今日运势（支持HTML渲染）
- 签到系统
- 银行系统（存款、取款、利息计算）
- 漂流瓶功能
- 伪造聊天

### 4. 音乐系统
- 多平台点歌（QQ、网易云、酷我、汽水）
- 选歌功能
- 音乐收藏

### 5. 事件系统
- 11种事件类型管理
- 全局/群聊事件开关
- 自定义事件配置

### 6. 账号系统
- 账号信息查询
- 账号设置

### 7. 群老婆系统
- 老婆设置
- 老婆查询

### 8. 群公告系统
- 发布群公告
- 图文公告支持
- 自动置顶

### 6. 定时任务
- 整点报时
- 全群打卡
- 签到统计重置
- 系统状态检查

### 7. 账号系统
- 账号信息查询
- 账号管理

### 8. 群老婆系统
- 群老婆设置
- 群老婆查询

### 9. 群公告系统
- 群公告发布（支持文本）
- 群公告发布（支持图片）
- 群公告置顶显示

### 10. WebUI 管理界面
- 群聊/好友开关管理
- 基础设置
- 更新公告
- 配置管理

## 技术特性

- 现代化TypeScript架构
- 模块化设计
- 完整的类型安全
- 定时任务管理
- HTML模板渲染支持
- WebUI集成

## 使用方法

1. 安装NapCat框架
2. 将插件文件放入插件目录
3. 启动NapCat并加载插件
4. 通过WebUI或指令配置插件

## 指令列表

### 授权相关
- `授权系统` - 授权菜单
- `授权判断` - 检查授权状态
- `使用卡密[卡密]` - 使用授权卡密

### 娱乐功能
- `今日运势` - 获取今日运势
- `签到`/`打卡` - 签到
- `我的信息` - 查看个人信息
- `银行系统` - 银行功能菜单

### 群管功能
- `群管系统` - 群管功能菜单
- `菜单` - 主菜单

### 音乐功能
- `音乐系统` - 音乐功能菜单
- `点歌[歌曲名]` - 点歌
- `选歌[序号]` - 选歌

### 事件管理
- `事件管理` - 事件管理菜单
- `开启/关闭[事件名]` - 开启/关闭事件

### 账号管理
- `账号系统` - 账号管理菜单
- `我的账号` - 查看账号信息

### 群老婆系统
- `群老婆系统` - 群老婆功能菜单
- `设置老婆` - 设置群老婆
- `查看老婆` - 查看群老婆

### 群公告系统
- `群公告系统` - 群公告系统菜单
- `设置群公告 [内容] [可选图片]` - 发布群公告

## 配置

插件支持通过WebUI进行配置，包括：
- 主人QQ设置
- 非主人回复设置
- 群聊/好友开关管理

## 定时任务

插件包含以下定时任务：
- 每5分钟检查系统状态
- 每日00:00重置签到统计
- 每小时整点报时
- 全群打卡（00:00）

所有定时任务在插件卸载时会自动清理，避免内存泄漏。

## 项目结构

```
napcat-plugin-QTBOT/
├── dist/                     # 构建输出目录
│   ├── index.mjs            # 核心插件文件
│   ├── html/                # HTML模板文件
│   │   ├── 今日运势.html
│   │   ├── 入群身份.html
│   │   ├── 导航菜单.html
│   │   ├── 状态.html
│   │   └── text/
│   │       ├── 运势.json
│   │       └── URL.json
│   └── webui/               # WebUI构建产物
│       └── index.html       # WebUI界面
├── MKbot11/                 # 原始MKbot11文件
│   ├── index.mjs            # 原始插件文件
│   ├── package.json
│   └── data/                # 原始数据文件
│       └── html/
│           ├── 导航菜单.html
│           ├── 今日运势.html
│           ├── 入群身份.html
│           ├── 状态.html
│           └── text/
│               ├── 运势.json
│               └── URL.json
├── src/                     # 源代码目录
│   ├── config.ts            # 配置定义
│   ├── index.ts             # 插件主入口
│   ├── types.ts             # 类型定义
│   ├── core/                # 核心模块
│   │   ├── auth-system.ts   # 授权系统
│   │   ├── scheduler.ts     # 定时任务
│   │   └── state.ts         # 全局状态管理
│   ├── handlers/            # 消息处理模块
│   │   ├── event-handler.ts # 事件处理
│   │   ├── message-handler.ts # 消息处理
│   │   └── message-utils.ts # 消息工具
│   ├── html/                # HTML模板
│   │   ├── 导航菜单.html
│   │   ├── 今日运势.html
│   │   ├── 入群身份.html
│   │   ├── 状态.html
│   │   └── text/
│   │       ├── 运势.json
│   │       └── URL.json
│   ├── modules/             # 功能模块
│   │   ├── account.module.ts # 账号管理
│   │   ├── auth.module.ts   # 授权系统
│   │   ├── announcement.commands.ts # 群公告指令
│   │   ├── announcement.module.ts # 群公告系统
│   │   ├── bank.commands.ts # 银行系统指令
│   │   ├── bank.module.ts   # 银行系统
│   │   ├── bottle.commands.ts # 漂流瓶指令
│   │   ├── bottle.module.ts # 漂流瓶
│   │   ├── daily.commands.ts # 日常功能指令
│   │   ├── daily.module.ts  # 日常功能
│   │   ├── event.commands.ts # 事件管理指令
│   │   ├── event.module.ts  # 事件管理
│   │   ├── group.commands.ts # 群管系统指令
│   │   ├── group.module.ts  # 群管系统
│   │   ├── index.ts         # 模块入口
│   │   ├── invite.commands.ts # 邀请统计指令
│   │   ├── invite.module.ts # 邀请统计
│   │   ├── music.commands.ts # 音乐系统指令
│   │   ├── music.module.ts  # 音乐系统
│   │   ├── system.commands.ts # 系统管理指令
│   │   ├── system.module.ts # 系统管理
│   │   └── waifu.module.ts  # 群老婆系统
│   ├── services/            # 服务模块
│   │   ├── api-service.ts   # API服务
│   │   └── puppeteer-service.ts # HTML渲染服务
│   ├── types/               # 类型定义
│   │   └── napcat-fix.d.ts
│   └── webui/               # WebUI源码
│       ├── dashboard.html   # WebUI界面
│       ├── index.html       # WebUI入口
│       ├── package.json
│       ├── tsconfig.json
│       ├── vite.config.ts
│       ├── src/
│       │   ├── App.tsx
│       │   ├── main.tsx
│       │   ├── types.ts
│       │   ├── components/
│       │   ├── hooks/
│       │   ├── pages/
│       │   └── utils/
│       └── static/
├── .example/                # 插件示例
├── .github/                 # GitHub配置
├── scripts/                 # 构建脚本
├── package.json             # 项目配置
├── tsconfig.json            # TypeScript配置
├── vite.config.ts           # 构建配置
├── README.md                # 项目说明
├── 介绍.txt                 # 插件介绍
└── LICENSE                  # 许可证
```

## 安装与构建

```bash
# 安装依赖
pnpm install

# 构建插件
pnpm run build

# 构建WebUI
pnpm run build:webui
```

## 许可证

MIT