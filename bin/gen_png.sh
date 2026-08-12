#!/bin/bash
#
#

n=$1
[ -z $n ] && n=50

# 把 Valgrind/Callgrind 性能分析文件转换成 PNG 调用关系图
for name in callgrind.out.*; do
    var=${name#callgrind.out.}
    echo "gprof2dot -f callgrind -n${n} -s ${name} | dot -Grankdir=LR -Tpng -o valgrind-${var}-n${n}.png"
    gprof2dot -f callgrind -n${n} -s ${name} | dot -Grankdir=LR -Tpng -o valgrind-${var}-n${n}.png
done