#!/bin/bash
# ============================================
# Arise 分布式服务一键启动
# 用法: bash run-all.sh [release]
#   默认启动 Debug 版（二进制带 d 后缀）
#   release 参数启动 Release 版（二进制不带后缀）
# 顺序: PHP账号验证 -> appmgr -> dbmgr -> login -> space -> game
# 日志: bin/logs/
# ============================================
ROOT="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$ROOT/bin/logs"
cd "$ROOT/bin" || exit 1

# 构建类型: Debug(默认) / Release
if [ "${1:-}" = "release" ]; then
    BUILD_TYPE="Release"
    SFX=""
else
    BUILD_TYPE="Debug"
    SFX="d"
fi

echo "===== 启动 ${BUILD_TYPE} 版 ====="

# 检测另一版本的进程是否在运行（端口会冲突）
if [ -z "$SFX" ]; then OTHER_SFX="d"; else OTHER_SFX=""; fi
for base in appmgr dbmgr login space game allinone; do
    if pgrep -f "(^|/)$base$OTHER_SFX( |$)" > /dev/null; then
        echo "[warn] 检测到 $base$OTHER_SFX 正在运行（另一版本），端口可能冲突，建议先执行 bash stop-all.sh"
        break
    fi
done

start() {
    local name=$1; shift
    if [ ! -x "./$name" ]; then
        local hint="bash make-all.sh"
        [ -z "$SFX" ] && hint="bash make-all.sh release"
        echo "[error] bin/$name 不存在，请先编译: $hint"
        return
    fi
    if pgrep -f "(^|/)$name( |$)" > /dev/null; then
        echo "[skip] $name 已在运行"
        return
    fi
    # tail 保持 stdin 不 EOF（服务器控制台线程读 stdin，EOF 会空转）
    ( tail -f /dev/null 2>/dev/null | ./$name "$@" > logs/$name.log 2>&1 & echo $! > logs/$name.pid )
    echo "[start] $name $*  (pid=$(cat logs/$name.pid), log: bin/logs/$name.log)"
}

# 1. 第三方账号验证 HTTP 服务 (web/member_login_t.php)
# PHP_CLI_SERVER_WORKERS: 内置服务器多进程并行，批量压测时避免验证请求排队
if ! pgrep -f "php -S 127.0.0.1:8080 -t $ROOT/web" > /dev/null; then
    nohup env PHP_CLI_SERVER_WORKERS=4 php -S 127.0.0.1:8080 -t "$ROOT/web" > logs/php_login.log 2>&1 &
    echo "[start] php 账号验证服务 127.0.0.1:8080 (pid=$!)"
else
    echo "[skip] php 账号验证服务已在运行"
fi

# 2. 服务进程（按依赖顺序启动，连接器会自动重连所以顺序不严格）
start appmgr$SFX              # 服务管理：注册中心 + 世界分配
sleep 1
start dbmgr$SFX               # 数据库服务：MySQL + Redis
sleep 1
start login$SFX -sid=101      # 登录服务
start space$SFX -sid=301      # 世界服务（地图）
start game$SFX -sid=201       # 游戏逻辑服务

sleep 2
echo ""
echo "===== 监听端口检查 ====="
ss -tln 2>/dev/null | grep -E ':(5500|5800|5401|5601|5701|7071|8080)[[:space:]]' || echo "!! 未发现监听端口，请查看 bin/logs/ 下日志"
echo ""
echo "启动完成。停止: bash stop-all.sh   查看日志: tail -f bin/logs/<服务名>.log"
