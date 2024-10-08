









# 开发日志

## 相关的C/C++基础

- `c_str()` 返回一个指向 `std::string` 内部字符数组的 C 风格字符串（以 `'\0'` 结尾）

## 1 数据库

### 1.1 技术栈

- sqlite3 version 3.37.2

### 1.2 数据库创建

```sqlite
sqlite3 ./cloud.db
```

### 1.3 数据表创建

#### 用户信息表

![image-20240723153259856](images/image-20240723153259856.png)

```sqlite
sqlite> create table usrInfo(id integer primary key autoincrement,
   ...> name varchar(32) unique,
   ...> pwd varchar(32),
   ...> online integer default 0);
```

### ![image-20240723153430756](images/image-20240723153430756.png)

插入

```sqlite
sqlite> insert into userInfo(name,pwd) values('jack','jack'),
   ...> ('rose','rose')
   ...> ('lucy','lucy');
```

![image-20240723154525507](images/image-20240723154525507.png)

#### 用户好友表

![image-20240723153316025](images/image-20240723153316025.png)

```sqlite
sqlite> create table friendInfo(id integer not null,
   ...> friendId integer not null,
   ...> primary key(id,friendId));
```

![image-20240723153850097](images/image-20240723153850097.png)

### 1.4 数据库连接

![image-20240725175628553](images/image-20240725175628553.png) 

## 2 客户端

### 2.1 技术栈

- QT5.14.2 https://mirrors.tuna.tsinghua.edu.cn/qt/official_releases/online_installers/qt-unified-windows-x64-online.exe

### 2.2 项目配置

![image-20240723205646802](images/image-20240723205646802.png)

### 2.3 创建项目

![image-20240723203912070](images/image-20240723203912070.png)

### 2.4 配置文件

- 添加

![image-20240723210251863](images/image-20240723210251863.png)****

![image-20240723210422742](images/image-20240723210422742.png)

![image-20240723210625681](images/image-20240723210625681.png)

### 2.5 客户端开发

#### 2024.7.23 配置文件加载

![image-20240723213633576](images/image-20240723213633576.png)

#### 2024.7.23 服务器连接

![image-20240723213819240](images/image-20240723213819240.png)

![image-20240723214014999](images/image-20240723214014999.png)

- 连接服务器以及绑定信号槽

![image-20240723215326064](images/image-20240723215326064.png)

#### 2024.7.25 界面信号槽

![image-20240725104429221](images/image-20240725104429221.png)

![image-20240725163819613](images/image-20240725163819613.png)

#### 2024.7.27 查看在线用户

![image-20240727183125190](images/image-20240727183125190.png)

#### 2024.7.28 搜索用户

![image-20240728121127702](images/image-20240728121127702.png)

#### 2024.7.28 添加好友

![image-20240728143751367](images/image-20240728143751367.png)

#### 2024.7.29 刷新好友

![image-20240729141528416](images/image-20240729141528416.png)

#### 2024.7.31 删除好友

![image-20240731095826689](images/image-20240731095826689.png)

#### 2024.7.31 私聊

![image-20240731143015165](images/image-20240731143015165.png)

客户端B收到聊天信息之后要设置聊天对象为发送方

![image-20240731155327428](images/image-20240731155327428.png)

#### 2024.7.31 群聊

![image-20240731202654525](images/image-20240731202654525.png)

#### 2024.8.1 文件操作

界面

![image-20240801213936500](images/image-20240801213936500.png)

创建文件夹

![image-20240801215832337](images/image-20240801215832337.png)

#### 2024.8.4 查看所有文件

![image-20240804151709251](images/image-20240804151709251.png)

#### 2024.8.4 删除文件夹

![image-20240804215044850](images/image-20240804215044850.png)

#### 2024.8.5 重命名文件

![image-20240805130833874](images/image-20240805130833874.png)

#### 2024.8.5 返回上一级

![image-20240805165707333](images/image-20240805165707333.png)

#### 2024.8.6 上传文件

![image-20240806205910075](images/image-20240806205910075.png)

#### 2024.8.8 删除常规文件

![image-20240808192821687](images/image-20240808192821687.png)

#### 2024.8.8 下载文件

![image-20240808194711280](images/image-20240808194711280.png)

#### 2024.8.11 分享文件

![image-20240811202708224](images/image-20240811202708224.png)

#### 2024.8.13 移动文件

![image-20240813202659179](images/image-20240813202659179.png)

### 2.6 服务端开发

#### 2024.7.24 服务器实现流程

![image-20240724132249510](images/image-20240724132249510.png)

#### 2024.8.5 进入文件夹

![image-20240805145629769](images/image-20240805145629769.png)

- 让自定义类支持信号槽

![image-20240724132651610](images/image-20240724132651610.png)

- 单例模式

单例模式下成员也是单例的

![image-20240724133019956](images/image-20240724133019956.png)

当static定义在局部，会导致该变量在程序的整个生命周期内只被初始化一次，并且保留其值直到程序结束

![image-20240724133112244](images/image-20240724133112244.png)

- 监听端口

![image-20240724135802213](images/image-20240724135802213.png)

- 重写虚函数

![image-20240724135849467](images/image-20240724135849467.png)

#### 2024.7.25 协议设计

![image-20240725100148752](images/image-20240725100148752.png)

![image-20240725103233143](images/image-20240725103233143.png)

![image-20240725103142187](images/image-20240725103142187.png)

#### 2024.7.25 socket收发

![image-20240725164058254](images/image-20240725164058254.png)

每个socket负责一条与服务端的信道

![image-20240725163933346](images/image-20240725163933346.png)

#### 2024.7.26 发出下线信号

socket中的定义：

![image-20240726151127963](images/image-20240726151127963.png)

发出信号

![image-20240726151246032](images/image-20240726151246032.png)

在mytcpserver类关联信号槽

![image-20240726151450792](images/image-20240726151450792.png)

![image-20240726152251475](images/image-20240726152251475.png)

槽函数处理

![image-20240726155752324](images/image-20240726155752324.png)

解决下线报错 deleteLater()

![image-20240726163124906](images/image-20240726163124906.png)

### 2.7 柔性数组

int d[] 不占空间

![image-20240724140635773](images/image-20240724140635773.png)

![image-20240724141348628](images/image-20240724141348628.png)

## TODO

- 服务器关闭时，应该设置所有online为0
- void Book::shareFile() 要改