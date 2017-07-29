1、先安装python fabric
2、cp fabric.ini.tmpl fabric.ini
打开fabric.ini, 设置自己的用户名密码
3、修改~/.bashrc,添加alias fab='fab -f /absolute/path/fabfile.py'，后面为自己的fabfile的绝对路径，然后source ~/.bashrc

配置fabfile.py，将路径替换成自己本地的工作路径
home_path="/mnt/windows"
supervisor_path="%s/DD_GAMES/DD_CONF/supervisor" % home_path
game_path="%s/DD_GAMES/Output" % home_path
comm_path="%s/DD_COMM/Output" % home_path
remote_path="/home/Output"
4、默认fabfile只提供两个公共方法，rcp,redist
5、执行fab -R test-game rcp:local_file,remote_path，可以拷贝文件到test-game，可以指定机器role
6、执行fab redist:game_folder_path,[alloc|game|robot|hundredgame|hundredrobot|all]，可以拷贝并部署到test-game,自动重启服务
注意此方法只适用于新框架下的游戏
game_folder_path为游戏文件夹名称
[alloc|game|robot|hundredgame|hundredrobot|all]可以任意指定一个，alloc会更新Alloc模块，其它类推，All更新所有模块

7、test5上采用supervisor监控启动程序，如果没有采用它控制的游戏，请自行添加到supervisor，方法参照test5上的/etc/supervisor/fish.conf
拷贝文件修改后，
将原有程序killall
执行
sudo supervisorctl reread
sudo supervisorctl update
关于supervisor的问题可以自行百度

8、关于supervisor启动组和组成员
例如
重启fishgame全组：sudo supervisorctl restart fishgame:
重启fishgame-1：sudo supervisorctl restart fishgame:fishgame-1

如果没有组，sudo supervisorctl restart program

9、在线服务器的程序上传相当慢，因此fabric增加了ftp命令，实现本地上传到ftp服务器,目标机器下载到本地的功能
example：
在DD_Servers目录，执行fab -R test-game ftp:Output/LandGame/GameServer/bin/LandGame,~/

10、各游戏的supervisor配置文件，必须写好并放到DD_CONF/supervisor目录下，实际部署的时候将它copy到/etc/supervisor/目录