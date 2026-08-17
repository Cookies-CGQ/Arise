# Arise 分布式游戏服务器

基于 C++ 的分布式游戏服务器框架，采用多进程 + 多线程架构，包含登录、游戏逻辑、世界（地图）、数据库管理等服务，并附带机器人压测工具。

## 架构组成

| 服务 | 进程 | 端口 | 职责 |
|---|---|---|---|
| appmgr | `appmgr[d]` | 5500 (TCP) / 7071 (HTTP) | 服务注册中心、世界分配、HTTP 登录入口 `/login` |
| dbmgr | `dbmgr[d]` | 5800 | 数据库服务（MySQL 角色数据 + Redis 在线状态/Token） |
| login | `login[d] -sid=101` | 5401 | 登录验证（第三方验证 + 角色列表 + Token 签发） |
| game | `game[d] -sid=201` | 5601 | 游戏逻辑、世界代理 |
| space | `space[d] -sid=301` | 5701 | 世界（地图）实例、玩家位置与广播 |
| robots | `robots[d]` | — | 机器人压测工具（交互式控制台） |
| nginx + php-fpm | — | 8080 | 第三方账号验证接口（`web/member_login_t.php`） |

> Debug 版二进制带 `d` 后缀（`logind`），Release 版不带（`login`）。`allinone[d]` 为单进程一体化版本（本地测试用）。

## 环境要求

- Ubuntu（或任意 Linux），gcc/g++（C++14）、CMake ≥ 3.10
- Redis（服务使用 **DB 1**）
- MySQL（需账号有建库权限，库表自动创建）
- PHP 8.x（php-fpm）+ nginx
- 依赖库：protobuf、yaml-cpp、log4cplus、jsoncpp、hiredis、libuuid、libmysqlclient

```bash
sudo apt install cmake g++ redis-server mysql-server nginx php-fpm \
    libprotobuf-dev protobuf-compiler libyaml-cpp-dev liblog4cplus-dev \
    libjsoncpp-dev libhiredis-dev uuid-dev libmysqlclient-dev
```

## 快速开始

```bash
# 1. 编译（Release 版，输出到 bin/）
bash make-all.sh release

# 2. 配置 res/engine.yaml（数据库地址、url_login），详见下文

# 3. 一键启动（自动拉起 nginx + php-fpm + 5 个服务进程）
bash run-all.sh release

# 4. 压测验证
cd bin && ./robots
login -ex robot 500        # 500 个机器人完整登录流程

# 5. 停止
bash stop-all.sh
```

## 编译

```bash
bash make-all.sh            # Debug 版 → bin/*d
bash make-all.sh release    # Release 版 → bin/*
bash make-all.sh clean      # 清理构建产物
```

修改 `src/libs/libserver/protobuf/` 下的 `.proto` 后需重新生成：

```bash
cd src/libs/libserver/protobuf
protoc --cpp_out=./ proto_id.proto db.proto msg.proto
```

## 运行

```bash
bash run-all.sh [release]        # 分布式启动（appmgr→dbmgr→login→space→game）
bash run-allinone.sh [release]   # 一体化单进程（与上面二选一）
bash stop-all.sh                 # 停止所有服务（Debug/Release 一起停）
```

端口确认（启动成功 = 7 个端口 LISTEN）：

```bash
ss -tln | grep -E ':(5500|5800|5401|5601|5701|7071|8080)'
```

## 配置（res/engine.yaml）

| 配置项 | 说明 |
|---|---|
| `dbmgr.dbs` | Redis/MySQL 地址、账号密码；库表由 dbmgr 启动时自动创建（`e_gamedata`） |
| `login.url_login` | 第三方账号验证 URL（本项目配套 `web/member_login_t.php`） |
| `*.apps` | 各服务的 id/ip/port 列表，启动参数 `-sid=` 必须与之对应（login 101、game 201、space 301） |

## 测试（机器人压测工具）

```bash
cd bin && ./robots
```

| 命令 | 说明 |
|---|---|
| `login -a test1` | 单机器人完整登录（建角色→进游戏→进世界） |
| `login -ex robot 500` | 批量压测：500 个机器人（账号 robot0~robot499） |
| `world -enter 3` | 进入江夏地图（world id=3） |
| `http -check test1 123456` | 单独测试 PHP 验证接口 |
| `help` | 命令帮助 |

批量压测前清理旧数据（**注意 Redis 是 DB 1**）：

```bash
redis-cli -n 1 keys 'engine::*' | xargs -r redis-cli -n 1 DEL
mysql -u<用户> -p -e "DELETE FROM e_gamedata.player WHERE account LIKE 'robot%';"
```

### 如何判断测试通过

`bin/logs/robots.log` 中：

1. 状态机每个阶段都出现 `over.` 行（"全部机器人通过该阶段"的耗时）；
2. 结尾汇总表 `[Space] EnterWorld : 500`（数字 = 机器人数），其余状态全 0；
3. 单机器人模式下可见 `account:test1 enter world. id:2`（大厅）和 `id:3`（江夏）。

性能参考（本机 Release 版实测）：**500 机器人 1.3s、1000 机器人 2.2s 全部进世界，零错误**。

## 登录流程与日志对照

| 阶段 | 证据 |
|---|---|
| HTTP 登录入口（appmgr /login） | `robots.log`：`Http-Connected over.` |
| 连接 login + 在线检查 | `log4_login.log`：`get engine::token::xxx type:4`（NIL 属正常） |
| 第三方验证（连接池→nginx→php-fpm） | 成功静默；失败见 `verify timeout` |
| 进入大厅 / 查角色列表 | `robots.log`：`enter world. id:2`（单机器人模式） |
| 创建角色 | `log4_login.log`：`create. account:xxx`；`log4_dbmgr.log`：`HandlePlayerCreate sn:... account:xxx` |
| 选角色 + Token | `robots.log`：`Login-SelectPlayer over.` |
| 连接 game 验证 Token | `robots.log`：`Game-Logined over.` |
| 世界创建 + 进图 | `robots.log`：`enter world. id:3`；汇总表 `[Space] EnterWorld` |

## 日志位置

| 日志 | 路径 | 用途 |
|---|---|---|
| 服务控制台输出 | `bin/logs/<服务名>.log` | 启动信息、端口监听 |
| 全量业务日志 | `bin/log4_<服务名>.log` | 带时间戳和级别的业务日志 |
| 纯错误日志 | `bin/log4_<服务名>_error.log` | 快速排障（无正常流程刷屏） |
| 机器人工具 | `bin/logs/robots.log` | 压测进度与汇总 |
| PHP 验证服务 | `bin/logs/php-fpm.log` | php-fpm 运行日志 |

## 目录结构

```
├── make-all.sh              # 一键编译（Debug/Release/clean）
├── run-all.sh               # 一键启动分布式服务 [release]
├── run-allinone.sh          # 一键启动一体化进程 [release]
├── stop-all.sh              # 停止所有服务
├── bin/                     # 可执行文件与日志
├── deploy/                  # nginx / php-fpm 配置（账号验证服务）
├── web/                     # 第三方账号验证 PHP 脚本
├── res/
│   ├── engine.yaml          # 服务进程配置
│   ├── log4/                # 各服务日志配置
│   └── resource/            # 策划配置（world.csv 等）
└── src/
    ├── libs/                # 框架库：libserver / libplayer / libresource
    ├── apps/                # 服务进程：appmgr / dbmgr / login / game / space / allinone
    └── tools/               # 工具：robots（压测）
```

## 关键设计

- **多进程 + 多线程**：每个服务独立进程，进程内按 Logic/Listen/Connect/Mysql 线程分工，消息跨线程广播分发
- **对象池**：Entity/Component/Packet/ConnectObj 全部走对象池，SN 全局唯一
- **连接代次（Epoch）**：socket fd 复用后仍能区分新旧连接，杜绝串包
- **第三方验证连接池**：login 预建 8 条到 PHP 的 keep-alive 长连接，验证请求复用，消除短连接风暴（500 机器人压测从 30s 优化到 1.3s）
- **超时自愈**：验证请求丢失时 5 秒超时踢人+删锁+重试，保证最终一致
- **防重复登录**：redis 在线锁 + token 交接窗口，同账号重复登录会被拒绝

## 常见问题

| 问题 | 排查 |
|---|---|
| 服务起不来 / 端口没监听 | `bin/logs/<服务名>.log`（绑定失败、配置错误） |
| 登录卡在验证 | `log4_login.log` 的 `verify timeout`；检查 8080 的 nginx/php-fpm 是否存活 |
| 提示"账号在线"被拒 | 账号有残留登录锁：`redis-cli -n 1 DEL engine::online::login::<账号>` |
| dbmgr 启动失败 | MySQL 账号/密码是否与 `engine.yaml` 一致，是否有建库权限 |
| 压测结果 < 机器人数 | 看 `log4_*_error.log` + robots.log 卡住的阶段，对照上表定位 |
