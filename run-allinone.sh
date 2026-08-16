#!/bin/bash
# ============================================
# Arise 一体化进程启动（所有服务合并在一个进程，本地测试最方便）
# 注意: 与 run-all.sh 二选一，不要同时运行（端口冲突）
# ============================================
ROOT="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$ROOT/bin/logs"
cd "$ROOT/bin" || exit 1

# 第三方账号验证 HTTP 服务
if ! pgrep -f "php -S 127.0.0.1:8080 -t $ROOT/web" > /dev/null; then
    nohup php -S 127.0.0.1:8080 -t "$ROOT/web" > logs/php_login.log 2>&1 &
    echo "[start] php 账号验证服务 127.0.0.1:8080 (pid=$!)"
else
    echo "[skip] php 账号验证服务已在运行"
fi

if pgrep -f "(^|/)allinoned( |$)" > /dev/null; then
    echo "[skip] allinoned 已在运行"
else
    ( tail -f /dev/null 2>/dev/null | ./allinoned > logs/allinoned.log 2>&1 & echo $! > logs/allinoned.pid )
    echo "[start] allinoned (pid=$(cat logs/allinoned.pid), log: bin/logs/allinoned.log)"
fi

sleep 2
ss -tln 2>/dev/null | grep -E ':(5401|5500|5601|5701|5800|7071|8080)[[:space:]]'
echo "停止: bash stop-all.sh"
