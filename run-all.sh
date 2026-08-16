#!/bin/bash
# ============================================
# Arise 分布式服务一键启动
# 顺序: PHP账号验证 -> appmgr -> dbmgr -> login -> space -> game
# 日志: bin/logs/
# ============================================
ROOT="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$ROOT/bin/logs"
cd "$ROOT/bin" || exit 1

start() {
    local name=$1; shift
    if pgrep -f "(^|/)$name( |$)" > /dev/null; then
        echo "[skip] $name 已在运行"
        return
    fi
    # tail 保持 stdin 不 EOF（服务器控制台线程读 stdin，EOF 会空转）
    ( tail -f /dev/null 2>/dev/null | ./$name "$@" > logs/$name.log 2>&1 & echo $! > logs/$name.pid )
    echo "[start] $name $*  (pid=$(cat logs/$name.pid), log: bin/logs/$name.log)"
}

# 1. 第三方账号验证 HTTP 服务 (web/member_login_t.php)
if ! pgrep -f "php -S 127.0.0.1:8080 -t $ROOT/web" > /dev/null; then
    nohup php -S 127.0.0.1:8080 -t "$ROOT/web" > logs/php_login.log 2>&1 &
    echo "[start] php 账号验证服务 127.0.0.1:8080 (pid=$!)"
else
    echo "[skip] php 账号验证服务已在运行"
fi

# 2. 服务进程（按依赖顺序启动，连接器会自动重连所以顺序不严格）
start appmgrd                 # 服务管理：注册中心 + 世界分配
sleep 1
start dbmgrd                  # 数据库服务：MySQL + Redis
sleep 1
start logind -sid=101         # 登录服务
start spaced -sid=301         # 世界服务（地图）
start gamed -sid=201          # 游戏逻辑服务

sleep 2
echo ""
echo "===== 监听端口检查 ====="
ss -tln 2>/dev/null | grep -E ':(5500|5800|5401|5601|5701|7071|8080)[[:space:]]' || echo "!! 未发现监听端口，请查看 bin/logs/ 下日志"
echo ""
echo "启动完成。停止: bash stop-all.sh   查看日志: tail -f bin/logs/<服务名>.log"
