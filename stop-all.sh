#!/bin/bash
# ============================================
# 停止 Arise 所有服务（Debug 和 Release 版都会停止）
# 优雅关闭：SIGINT，服务端捕获后保存数据退出
# ============================================
cd "$(dirname "$0")/bin" || exit 1

# Debug 版二进制带 d 后缀，Release 版不带
for sfx in d ""; do
    for base in game space login dbmgr appmgr robots allinone; do
        name="${base}${sfx}"
        if pgrep -f "(^|/)$name( |$)" > /dev/null; then
            pkill -INT -f "(^|/)$name( |$)"
            echo "[stop] $name 已发送停止信号"
        fi
    done
done

# nginx + php-fpm 验证服务（只停本项目的实例，不动系统自带的）
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
if [ -f "$ROOT/deploy/nginx.pid" ]; then
    nginx -c "$ROOT/deploy/nginx.conf" -s quit 2>/dev/null || pkill -f "$ROOT/deploy/nginx.conf"
    echo "[stop] nginx (项目实例)"
fi
pkill -f "$ROOT/deploy/php-fpm.conf" 2>/dev/null && echo "[stop] php-fpm (项目实例)" || true
rm -f "$ROOT/deploy/php-fpm.sock"

sleep 15

# 清理 stdin 占位进程（各服务控制台 fifo 的 tail）
pkill -f 'tail -f /dev/null.*stdin.fifo' 2>/dev/null

# 清理残留进程
for sfx in d ""; do
    for base in game space login dbmgr appmgr robots allinone; do
        name="${base}${sfx}"
        if pgrep -f "(^|/)$name( |$)" > /dev/null; then
            pkill -9 -f "(^|/)$name( |$)"
            echo "[kill] $name 强制结束"
        fi
    done
done
echo "已全部停止。"
