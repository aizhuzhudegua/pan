# 开发日志

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
   ...> name varchar(32),
   ...> pwd varchar(32));
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

### 2.6 服务端开发

#### 2024.7.24 服务器实现流程

![image-20240724132249510](images/image-20240724132249510.png)

- 让自定义类支持信号槽

![image-20240724132651610](images/image-20240724132651610.png)

- 单例模式

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

### 2.7 柔性数组

int d[] 不占空间

![image-20240724140635773](images/image-20240724140635773.png)

![image-20240724141348628](images/image-20240724141348628.png)