#!/bin/bash

# 当前目录下的可执行文件路径
executable="./Serialport"

# 当前目录下的依赖库目录路径
lib_dir="./dependlibs"

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

# 设置LD_LIBRARY_PATH环境变量
export LD_LIBRARY_PATH="$lib_dir:$LD_LIBRARY_PATH"

# 启动可执行文件
echo "正在启动 $executable ..."
"$executable"

# 提示完成
echo "已成功设置库查找路径并运行 $executable"

