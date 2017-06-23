1、将DD_CONF下的hosts.tmpl拷贝一份，改名为hosts，并配置所有的游戏的域名
将DD_CONF下的preconf.tmpl拷贝一份，改名为preconf，配置其中的选项，包括mysql对应数据库的用户名密码

2、DD_CONF下所有游戏配置文件夹句采用ini2redis.py中的命名规则，如[1, "ShowHand", "梭哈"],则必须命名为ShowHand，
否则会被识别成错误的游戏名称

3、每个游戏文件夹下又分为[Alloc,Game,Robot]3个子文件下，分别存放对应的配置文档，其中Alloc和Game中必须放errmsg.ini，
有的游戏将errmsg放在代码中，需导出到errmsg.ini中，Robot没有errmsg,不必存放
其它存放于本游戏相关的ini和xml配置文档，其中用于侦听的IP地址规则为
Alloc为
[AllocServer]
Host=域名或0.0.0.0
Game为
[GameServer]
Host=域名
Robot为
[RobotServer]
Host=域名或0.0.0.0
Game必须使用域名，它需要将自己的IP报告给Alloc，Alloc又将IP报告给Hall

除侦听地址，其它IP地址一律使用域名，使用ini2redis导入时，会自动替换成hosts中对应的IP
xml文件中host对应的IP也必须一律改为域名，如下
<?xml version="1.0" encoding="utf-8"?>
<servers>
        <gameserver>
                <server id="307" host="flower.dd.game" port="30700"  desc="serverid为307的游戏服务器" />
        </gameserver>
</servers>
去掉所有用于连接公共服务器的配置选项，如mysql，money，round等，及redis的连接选项，只保留与自己游戏相关的即可

4、如果Game本身分为普通和百人版，需要两种配置文件，普通的为errmsg.ini,百人的为errmsg.hundred.ini，以此命名规则为区分

5、supervisor中存放着启动每个游戏所需的supervisor配置，每个游戏负责人需自己添加

6、RobotEtc中存放着机器人所用的names.txt文件，机器人的名称

7、CommonServer中存放着所有公共服务的配置信息

8、common.ini中存放着所有公共服及Redis，mysql的连接信息

9、hosts中存放着所有的游戏级公共服对应的域名，方便大家在填写地址时使用，如部署新的机器，只需修改hosts即可

10、ini2redis使用需安装redis的python包，导入时需配置/etc/hosts，加入config.dd.redis域名，指向开启6390端口的redis机器IP
./ini2redis.py 或python ini2redis.py，此命令会导入所有配置信息
后面可以指定文件夹名，例如./ini2redis.py BullFight Flower，则只会导入BullFight Flower的配置信息

11、导入RoomConfig信息到6386，执行python ini2redis.py RoomConfig，会将所有Game目录的.txt文件导入到redis