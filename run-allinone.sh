#!/bin/bash
# ============================================
# Arise 一体化进程启动（所有服务合并在一个进程，本地测试最方便）
# 用法: bash run-allinone.sh [release]
#   默认启动 Debug 版（allinoned），release 参数启动 Release 版（allinone）
# 注意: 与 run-all.sh 二选一，不要同时运行（端口冲突）
# ============================================
ROOT="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$ROOT/bin/logs"
cd "$ROOT/bin" || exit 1

# 构建类型: Debug(默认) / Release
if [ "${1:-}" = "release" ]; then
    BUILD_TYPE="Release"
    NAME="allinone"
else
    BUILD_TYPE="Debug"
    NAME="allinoned"
fi

echo "===== 启动 ${BUILD_TYPE} 版 ====="

if [ ! -x "./$NAME" ]; then
    local_hint="bash make-all.sh"
    [ "$BUILD_TYPE" = "Release" ] && local_hint="bash make-all.sh release"
    echo "[error] bin/$NAME 不存在，请先编译: $local_hint"
    exit 1
fi

# 第三方账号验证 HTTP 服务（nginx + php-fpm，支持 keep-alive 长连接）
# 用 socket/配置文件精确判断本项目的实例，避免与系统自带的 nginx/fpm 混淆
if [ ! -S "$ROOT/deploy/php-fpm.sock" ]; then
    nohup php-fpm8.4 -y "$ROOT/deploy/php-fpm.conf" > logs/php-fpm.log 2>&1 &
    echo "[start] php-fpm (pid=$!)"
else
    echo "[skip] php-fpm 已在运行"
fi
sleep 1
if ! pgrep -f "$ROOT/deploy/nginx.conf" > /dev/null; then
    nginx -c "$ROOT/deploy/nginx.conf"
    echo "[start] nginx 127.0.0.1:8080 (php-fpm 代理)"
else
    echo "[skip] nginx 已在运行"
fi

if pgrep -f "(^|/)$NAME( |$)" > /dev/null; then
    echo "[skip] $NAME 已在运行"
else
    ( tail -f /dev/null 2>/dev/null | ./$NAME > logs/$NAME.log 2>&1 & echo $! > logs/$NAME.pid )
    echo "[start] $NAME (pid=$(cat logs/$NAME.pid), log: bin/logs/$NAME.log)"
fi

sleep 2
ss -tln 2>/dev/null | grep -E ':(5401|5500|5601|5701|5800|7071|8080)[[:space:]]'
echo "停止: bash stop-all.sh"
