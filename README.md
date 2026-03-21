# Nuitka Studio

Nuitka Studio 是一个帮助您使用python nuitka打包工具的软件，使用C++ Qt框架编写，目前仅支持Windows8.1+

本项目使用 **Apache 2.0** 协议授权，请遵守开源协议

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

自定义命令参数：顾名思义，可以自定义nuitka打包时的命令参数，注意单个参数（比如--onefile）不能有空格，多个参数之间使用空格分割

*例：`--onefile --standalone --output-dir=/output`*



#### 数据文件

项目所需要的数据文件，例如图片，音频等，按需添加

##### 图标设置

软件的图标，如果不是.ico格式需要在项目的环境中安装imageio库



### 导入/导出项目配置文件

在软件界面的导出选项卡中，可以导出项目的参数，以便于下一次使用

在导出界面，可以预览项目设置或修改

确认完毕项目设置后点击"导出"按钮即可导出为.npf文件



## 构建项目

1. 从Github上克隆仓库到本地  `git clone https://github.com/Redrch/NuitkaStudio.git`
2. 确保设备安装有 mingw64,  cmake,  vcpkg,  python3.11+(推荐3.13)，并且cmake, vcpkg, python 配有环境变量，如果没有请安装
3. 打开cmd，将工作目录移动到项目根目录
4. 修改config.toml，将文件中的路径改为您设备上的路径
5. 执行`python build.py`，执行完成后如果是debug模式构建结果在`cmake-build-debug`中，release模式在`releases/NuitkaStudio{版本号}·`目录下



## 设置项

### 通用设置

1. 控制台输入编码：在执行nuitka命令时向控制台输入的编码，一般为`utf-8`，不建议修改
2. 控制台输出编码：控制台输出所用的编码，一般为`utf-8`，不建议修改
3. 语言：默认为简体中文，可切换为English
4. 打包计时器触发间隔：指的是打包时状态栏计时器的更新间隔，指越小计时器越精确，性能消耗越大，建议为100ms
5. 显示退出操作页面：在退出时显示 *退出/在托盘栏* 的选项
6. 退出时最小化到托盘：需要将`显示退出操作页面`项设置为`false`指才可使用，为`true`后可以自动退出时最小化到托盘
7. 缓存路径：NuitkaStudio缓存文件的路径，默认是`%temp%/NuitkaStudio`文件夹

​	**NPF文件设置**

1. 最大日志文件数量：指`pack_log`文件夹中能储存的最多日志数量，不建议设置在100以上，

	如果你想要将日志文件保存到npf文件中

2. 是否保存打包日志：是否在保存时将打包日志保存在npf文件中，默认为否，

	如果为是，那么将打包日志保存至npf文件；

	如果为否，那么只会将打包日志文件存储在`软件根目录/pack_log/npf文件名`目录下

	**警告：如果你将最大日志文件数量的值设置的很大，开启此选项会将npf文件变得巨大无比，这样加载npf文件的速度会变慢**



## 特别鸣谢

ElaWidgetTools: https://github.com/Liniyous/ElaWidgetTools

Quazip: https://github.com/stachenov/quazip 

spdlog: https://github.com/gabime/spdlog 

fmt: https://github.com/fmtlib/fmt

<br>

<br>

------------------------



## 附件

### 格式声明

1. 此标题下的两位版本号指的是这个版本号下的所有版本

	例如：NuitkaStudio1.2指NuitkaStudio1.2.0.0和NuitkaStudio1.2.1.0两个版本（因为1.2就这两个版本）

2. 版本号+指这个版本及以上的所有版本

	例如：NuitkaStudio1.2+指NuitkaStudio1.2.0.0及以上的所有版本（包含NuitkaStudio1.2.0.0）



### NPF文件格式

此文件中的NPF文件仅代表NuitkaStudio所使用的Nuitka Project File(NPF)文件

此处仅讨论NuitkaStudio1.2.0.0以上的版本中使用的NPF文件，不讨论NuitkaStudio1.2.0.0以下版本所使用的NPF文件



#### NPF Version 1

**注意：此版本已经不再维护，请使用NPF Version 2！**

这个版本的npf文件只在NuitkaStudio1.2版本使用

这个版本的npf文件，本质上是一个ZIP压缩包，格式如下所示

```
- /
--- data.json            // 存储npf文件的基本信息，例如NPF格式版本号，以及使用nuitka打包时的参数
--- pack_log/            // nuitka打包日志
------  yyyy-mm-dd_hh-MM-ss.log           // 具体文件
------  ...
```

**data.json 中的格式如下所示**

```json
{
    "npf_version": 1,
    "project": {
        "company": "company",
        "dataList": [
            ""
        ],
        "fileDescription": "description",
        "fileVersion": "version",
        "iconPath": "logo path",
        "legalCopyright": "legal copyright",
        "legalTrademarks": "legal trademarks",
        "ltoMode": 0,
        "mainFilePath": "main path",
        "onefile": false,
        "outputFilename": "output name",
        "outputPath": "output path",
        "productName": "name",
        "productVersion": "version",
        "projectName": "name",
        "projectPath": "path",
        "pythonPath": "python path",
        "removeOutput": false,
        "standalone": false
    }
}

```

`ltoMode`值中，0代表`Auto`，1代表`Yes`，2代表`No`

`npf_version`值目前为1



**pack_log/note.json文件中的内容如下所示**

```json
{
    "log file name1": "note1",
    "log file name2": "note2",
    "log file name3": "note3",
    "log file name4": "note4",
    "log file name5": "note5"
}
```

`log file name`是log文件的文件名

可以继续增加下去，这里只是举了一些例子



#### NPF Version 2

这是在NuitkaStudio1.3+的版本使用的格式，本质上是一个json文件，格式如下：

```json
{
    "npf_version": "2",
    "project": {
        "company": "company",
        "dataList": [
            ""
        ],
        "fileDescription": "description",
        "fileVersion": "version",
        "iconPath": "logo path",
        "legalCopyright": "legal copyright",
        "legalTrademarks": "legal trademarks",
        "ltoMode": 0,
        "mainFilePath": "main path",
        "onefile": false,
        "outputFilename": "output name",
        "outputPath": "output path",
        "productName": "name",
        "productVersion": "version",
        "projectName": "name",
        "projectPath": "path",
        "pythonPath": "python path",
        "removeOutput": false,
        "standalone": false
    },
    "pack_log": {
        "log name1": ["log1", "note"],
    	"log name2": ["log2", "note"]
    }
}
```

`pack_log`字段中的内容可以继续增加

如果没有勾选包含打包日志，那么npf文件中不会含有`pack_log`字段

**注意，此版本及以后版本中的`npf_version`字段是字符串**



### note.json文件格式

**note.json文件指的是打包日志备注文件，文件名必须为`note.json`**

本文件通常位于`软件根目录/pack_log/npf文件名/note.json`，

在打开npf文件后如果没有此文件并且npf文件中含有`pack_log`字段，那么会自动生成此文件

``````json
{
    "log name1": "note1",
    "log name2": "note2"
}
``````

`log name`字段指的是日志文件的名称，`note`字段指的是其对应的备注内容

<br>
<br>
<br>



------------------------------------------------------------------------

NuitkaStudio      版权所有 © 2025-2026 Redrch   Email: redrch327@gmail.com
