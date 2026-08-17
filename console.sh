#!/bin/bash
# ============================================
# 向运行中的服务发送控制台命令
# 用法: bash console.sh <服务名> <命令...>
# 例:
#   bash console.sh space 'world -all'
#   bash console.sh login 'thread -pool'
#   bash console.sh appmgr 'create -all'
# 输出: bin/logs/<服务名>.log
# ============================================
ROOT="$(cd "$(dirname "$0")" && pwd)"
NAME="$1"; shift
CMD="$*"

if [ -z "$NAME" ] || [ -z "$CMD" ]; then
    echo "用法: bash console.sh <服务名> <命令>"
    echo "例: bash console.sh space 'world -all'"
    exit 1
fi

FIFO="$ROOT/bin/logs/$NAME.stdin.fifo"
if [ ! -p "$FIFO" ]; then
    echo "[error] $NAME 未运行，或不是用 run-all.sh 启动的（缺少 $FIFO）"
    exit 1
fi

echo "$CMD" > "$FIFO"
echo "[send] $NAME <= $CMD"
echo "[查看输出] tail -f $ROOT/bin/logs/$NAME.log"
