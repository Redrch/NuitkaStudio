# Nuitka Studio

Nuitka Studio 是一个帮助您使用python nuitka打包工具的软件，使用C++ Qt框架编写，目前仅支持Windows8.1+

## 使用

### 基础

至Release界面下载发行版，解压后双击`NuitkaStudio.exe`即可运行

日志文件存放在 `app.log` 文件中

配置文件是 `config.ini`，一般于软件中修改

#### 基础设置

python解释器路径：项目所使用的python解释器的路径，需含有nuitka

主文件路径：项目入口文件（主文件）的路径，常为main.py

输出路径：项目输出的路径

输出文件名：输出exe文件的名字

#### 构建设置

独立文件：使输出的exe无需python.dll即可运行，一般情况下打开

单文件输出：将输出内容放在一个exe文件中

删除编译临时文件：输出编译中产生的临时文件，建议打开

链接时优化(LTO)：让Nuitka在链接时进行优化，会加长打包时间，如果不是对打包时间有要求或者电脑配置较低，建议启用

#### 数据文件

项目所需要的数据文件，例如图片，音频等，按需要添加

##### 图标设置

软件的图标，如果不是.ico格式需要在项目的环境中安装imageio库



### 导入/导出项目配置文件

在软件界面的导出选项卡中，可以导出项目的参数，以便于下一次使用

在导出界面，可以预览项目设置或修改

确认完毕项目设置后点击"导出"按钮即可导出为.npf文件



## 构建项目

1. 从Github上克隆仓库到本地  `git clone https://github.com/Redrch/NuitkaStudio.git`
2. 确保设备安装有 mingw64  cmake  vcpkg  python3.5+(推荐3.13)，并且cmake  vcpkg  python 配有环境变量，如果没有请安装
3. 打开cmd，将工作目录移动到项目根目录
4. 修改config.toml，将文件中的路径改为您设备上的路径
5. 执行`python build.py`，执行完成后如果是debug模式构建结果在`cmake-build-debug`中，release模式在`releases/NuitkaStudio{版本号}·`目录下



## 特别鸣谢

[Quazip]: https://github.com/stachenov/quazip
[spdlog]: https://github.com/gabime/spdlog
[fmt]: https://github.com/fmtlib/fmt



------------------------

## 附件

### NPF文件格式

**此文件中的NPF文件仅代表NuitkaStudio所使用的Nuitka Project File(NPF)文件**

**此处仅讨论NuitkaStudio1.2.0.0以上的版本中使用的NPF文件，不讨论NuitkaStudio1.2.0.0以下版本所使用的NPF文件**

本质上，NPF文件是一个ZIP压缩包，格式如下所示

```
- /
--- data.json            // 存储npf文件的基本信息，例如NPF格式版本号，以及使用nuitka打包时的参数
--- pack_log/            // nuitka打包日志
------  yyyy-mm-dd_hh-MM-ss.log           // 具体文件
------  ...
```

