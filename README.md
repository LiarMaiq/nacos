# Nacos

#### 介绍
A simple and very easy to use Alibaba Nacos C++ client.

#### 安装教程

1.  sudo apt install libcurl
2.  mkdir build
3.  cd build
4.  cmake ..
5.  make

使用 Virtual Studio Community 2019 构建：
1. 获取 vcpkg `$ git clone https://github.com/microsoft/vcpkg.git`，并安装依赖的库：
    ```
    $ vcpkg install libcurl
    ```
2. 使用 Virtual Studio Community 2019 打开文件夹，选择配置构建即可，目前有4个配置：
    - x64-Debug（Windows）
    - x64-Release（Windows）
    - WSL-GCC-Debug
    - WSL-GCC-Release

#### 使用说明

- 编辑配置文件 nacos.json；
- 只需引用头文件 Nacos.h；
- Nacos 初始化后会启动一个线程用于发送心跳和获取服务实例；
- 调用 Nacos::require 获取一个可用的服务实例；
- 首次获取指定服务实例后，该服务会被加入定时刷新队列。

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

