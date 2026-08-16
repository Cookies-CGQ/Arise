#!/bin/bash
# ============================================
# 停止 Arise 所有服务（优雅关闭：SIGINT，服务端捕获后保存数据退出）
# ============================================
cd "$(dirname "$0")/bin" || exit 1

for name in gamed spaced logind dbmgrd appmgrd robotsd; do
    if pgrep -f "(^|/)$name( |$)" > /dev/null; then
        pkill -INT -f "(^|/)$name( |$)"
        echo "[stop] $name 已发送停止信号"
    fi
done

# php 验证服务
if pgrep -f "php -S 127.0.0.1:8080" > /dev/null; then
    pkill -f "php -S 127.0.0.1:8080"
    echo "[stop] php 账号验证服务"
fi

sleep 15

# 清理残留进程
for name in gamed spaced logind dbmgrd appmgrd robotsd; do
    if pgrep -f "(^|/)$name( |$)" > /dev/null; then
        pkill -9 -f "(^|/)$name( |$)"
        echo "[kill] $name 强制结束"
    fi
done
echo "已全部停止。"
