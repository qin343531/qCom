#!/bin/bash

# 获取传入的可执行文件路径和依赖库目录路径
executable="$1"
lib_dir="$2"

# 检查可执行文件是否存在
if [[ ! -f "$executable" ]]; then
  echo "错误: 找不到可执行文件 $executable"
  exit 1
fi

# 检查依赖库目录是否存在
if [[ ! -d "$lib_dir" ]]; then
  echo "错误: 找不到库目录 $lib_dir"
  exit 1
fi

# 获取所有依赖库路径
libs=$(ldd "$executable" | awk '{print $3}' | grep -v '^$')

# 复制依赖库到目标目录
for lib in $libs; do
  if [[ -f "$lib" ]]; then
    echo "复制库 $lib 到 $lib_dir"
    cp "$lib" "$lib_dir"
  else
    echo "警告: 找不到依赖库 $lib，跳过复制"
  fi
done

echo "所有依赖库已复制到 $lib_dir"

