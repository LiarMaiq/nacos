# Nacos

#### 介绍
A simple and very easy to use Alibaba Nacos C++ client.

#### 安装教程

1.  sudo apt install libcurl
2.  mkdir build
3.  cd build
4.  cmake ..
5.  make

#### 使用说明

- Nacos 初始化后会启动一个线程用于发送心跳和获取服务实例；
- 调用 Nacos::require 获取一个可用的服务实例；
- 首次获取指定服务实例后，该服务会被加入定时刷新队列。

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
