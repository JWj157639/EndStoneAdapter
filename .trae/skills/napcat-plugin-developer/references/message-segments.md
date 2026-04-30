# 消息段完整列表

## 消息段结构

```typescript
interface OB11MessageSegment {
    type: string;              // 消息段类型
    data: Record<string, any>;  // 消息段数据
}
```

## 文本消息段

```typescript
{ 
    type: 'text', 
    data: { text: 'Hello World' } 
}
```

## 图片消息段

```typescript
{ 
    type: 'image', 
    data: { 
        file: 'http://example.com/image.jpg',  // 图片 URL 或文件路径
        summary: '图片描述',                      // 可选：图片描述
        url: 'http://example.com/image.jpg'     // 可选：图片 URL
    } 
}

// 本地图片
{ 
    type: 'image', 
    data: { file: 'C:/path/to/image.jpg' } 
}

// Base64 图片
{ 
    type: 'image', 
    data: { file: 'base64://iVBORw0KG...' } 
}
```

## @用户消息段

```typescript
{ 
    type: 'at', 
    data: { 
        qq: '123456',           // 用户 QQ 号
        name: '用户昵称',        // 可选：用户昵称
        role: 'admin',          // 可选：用户角色
        type: 'all'             // 可选：at all 时为 'all'
    } 
}

// @所有人
{ 
    type: 'at', 
    data: { 
        qq: 'all',
        type: 'all' 
    } 
}
```

## 表情消息段

```typescript
{ 
    type: 'face', 
    data: { id: 1 } }  // 表情 ID
```

常用表情 ID：
- 1: 微笑
- 2: 龌嘴
- 3: 色
- 4: 发呆
- 5: 得意
- 6: 流泪
- 7: 害羞
- 8: 闭嘴
- 9: 睡
- 10: 大哭
- 11: 尴尬
- 12: 发怒
- 13: 调皮
- 14: 呵呵
- 15: 惊讶

更多表情 ID 参考：[QQ 表情 ID 列表](https://github.com/nonebot/adapter-onebot/blob/master/onebot/v11/consts.py)

## 回复消息段

```typescript
{ 
    type: 'reply', 
    data: { 
        id: '123456'           // 要回复的消息 ID
    } 
}
```

## 语音消息段

```typescript
{ 
    type: 'record', 
    data: { 
        file: 'http://example.com/voice.mp3',  // 语音 URL 或文件路径
        url: 'http://example.com/voice.mp3'     // 可选：语音 URL
    } 
}

// 本地语音
{ 
    type: 'record', 
    data: { file: 'C:/path/to/voice.mp3' } 
}

// Base64 语音
{ 
    type: 'record', 
    data: { file: 'base64://...' } 
}
```

## 视频消息段

```typescript
{ 
    type: 'video', 
    data: { 
        file: 'http://example.com/video.mp4',  // 视频 URL 或文件路径
        url: 'http://example.com/video.mp4'     // 可选：视频 URL
    } 
}
```

## 链接分享消息段

```typescript
{ 
    type: 'share', 
    data: { 
        url: 'https://example.com',      // 链接 URL
        title: '标题',                    // 链接标题
        content: '内容',                  // 链接描述
        image: 'http://example.com/img'  // 可选：预览图片
    } 
}
```

## 音乐分享消息段

```typescript
{ 
    type: 'music', 
    data: { 
        type: 'qq',                      // 音乐类型：qq, 163, custom
        id: '123456'                    // 音乐 ID（qq/163）或音频 URL（custom）
    } 
}

// 自定义音乐
{ 
    type: 'music', 
    data: { 
        type: 'custom',
        url: 'http://example.com/music.mp3',
        audio: 'http://example.com/music.mp3',
        title: '歌曲名称',
        content: '歌手名称',
        image: 'http://example.com/cover.jpg'
    } 
}
```

## 合并转发消息段

```typescript
{ 
    type: 'node', 
    data: { 
        user_id: '123456',              // 发送者 QQ
        nickname: '发送者昵称',          // 发送者昵称
        content: [                       // 转发的消息段数组
            { type: 'text', data: { text: '消息内容' } }
        ]
    } 
}
```

## XML 消息段

```typescript
{ 
    type: 'xml', 
    data: { 
        data: '<xml>...</xml>',         // XML 内容
        resId: '123456'                 // 可选：资源 ID
    } 
}

// 示例：卡片消息
{ 
    type: 'xml', 
    data: { 
        data: `
            <?xml version="1.0" encoding="UTF-8"?>
            <msg serviceID="60">
                <item bg="0">
                    <title>标题</title>
                    <summary>摘要</summary>
                    <picture cover="图片URL"/>
                    <url>链接URL</url>
                </item>
            </msg>
        `
    } 
}
```

## JSON 消息段

```typescript
{ 
    type: 'json', 
    data: { 
        data: '{"key":"value"}',         // JSON 字符串
        resId: '123456'                 // 可选：资源 ID
    } 
}

// 示例：小程序卡片
{ 
    type: 'json', 
    data: { 
        data: `
            {
                "app": "com.tencent.structmsg",
                "config": {
                    "ctime": 1700000000,
                    "token": "xxx"
                },
                "desc": "描述",
                "prompt": "[提示]",
                "title": "标题",
                "url": "https://example.com",
                "view": "news",
                "ver": "0.0.0.1"
            }
        `
    } 
}
```

## 位置消息段

```typescript
{ 
    type: 'location', 
    data: { 
        lat: 39.9042,                    // 纬度
        lon: 116.4074,                   // 经度
        title: '位置名称',                // 位置名称
        content: '位置描述'               // 位置描述
    } 
}
```

## 红包消息段

```typescript
{ 
    type: 'redbag', 
    data: { 
        title: '恭喜发财',                // 红包标题
        type: 'lucky',                   // 红包类型：lucky（拼手气）或 normal（普通）
        content: '恭喜发财，大吉大利'     // 红包祝福语
    } 
}
```

## 戳一戳消息段

```typescript
{ 
    type: 'poke', 
    data: { 
        type: '戳一戳',                  // 戳一戳类型
        id: '123456'                     // 对方 QQ
    } 
}
```

## 消息段组合

### 多条消息

```typescript
const message: OB11PostSendMsg['message'] = [
    { type: 'text', data: { text: 'Hello ' } },
    { type: 'at', data: { qq: '123456' } },
    { type: 'text', data: { text: '!' } },
    { type: 'face', data: { id: 1 } },
];
```

### 图片 + 文本

```typescript
const message: OB11PostSendMsg['message'] = [
    { type: 'text', data: { text: '看看这张图：' } },
    { type: 'image', data: { file: 'http://example.com/image.jpg' } },
];
```

### @所有人 + 提醒

```typescript
const message: OB11PostSendMsg['message'] = [
    { type: 'at', data: { qq: 'all' } },
    { type: 'text', data: { text: '大家请注意！' } },
];
```

## 发送消息示例

### 发送文本

```typescript
await ctx.actions.call('send_msg', {
    message_type: 'group',
    group_id: '123456',
    message: [{ type: 'text', data: { text: 'Hello World' } }]
}, ctx.adapterName, ctx.pluginManager.config);
```

### 发送图片

```typescript
await ctx.actions.call('send_msg', {
    message_type: 'group',
    group_id: '123456',
    message: [{
        type: 'image',
        data: { file: 'http://example.com/image.jpg' }
    }]
}, ctx.adapterName, ctx.pluginManager.config);
```

### 发送 @用户

```typescript
await ctx.actions.call('send_msg', {
    message_type: 'group',
    group_id: '123456',
    message: [
        { type: 'at', data: { qq: '123456' } },
        { type: 'text', data: { text: ' 你好！' } }
    ]
}, ctx.adapterName, ctx.pluginManager.config);
```

### 发送合并转发

```typescript
const nodes = [
    {
        type: 'node',
        data: {
            user_id: '111',
            nickname: '用户A',
            content: [
                { type: 'text', data: { text: '消息内容A' } }
            ]
        }
    },
    {
        type: 'node',
        data: {
            user_id: '222',
            nickname: '用户B',
            content: [
                { type: 'text', data: { text: '消息内容B' } }
            ]
        }
    }
];

await ctx.actions.call('send_group_forward_msg', {
    group_id: '123456',
    message: nodes
}, ctx.adapterName, ctx.pluginManager.config);
```

## 消息段速查表

| 类型 | 用途 | 主要字段 |
|-----|------|---------|
| text | 文本 | text |
| image | 图片 | file, url |
| at | @用户 | qq, type |
| face | 表情 | id |
| reply | 回复 | id |
| record | 语音 | file, url |
| video | 视频 | file, url |
| share | 链接分享 | url, title, content |
| music | 音乐分享 | type, id, url, title |
| node | 合并转发 | user_id, nickname, content |
| xml | XML 消息 | data, resId |
| json | JSON 消息 | data, resId |
| location | 位置 | lat, lon, title |
| redbag | 红包 | title, type, content |
| poke | 戳一戳 | type, id |