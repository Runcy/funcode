# -*- coding:utf-8 -*-
from fabric.api import *
import os,sys
import ConfigParser

ftpcmd='''ftp -n<<!
open 116.31.118.211 33321
user ddproject uxyU47741JoL
binary
cd /
lcd %s
prompt
%s %s %s
close
bye
!'''

def new_optionxform(optionstr):
    return optionstr

def get_env_conf():
    cf = ConfigParser.ConfigParser()
    cf.optionxform = new_optionxform
    cf.read(os.path.join(sys.path[0], "fabric.ini"))
    sections = cf.sections()
    evn_gateway = "%s@%s" % (cf.get("gateway", "username"), cf.get("gateway", "host"))
    env_roles = {}
    env_passwords = {}
    for section in sections:
        host = cf.get(section, "host")
        username = cf.get(section, "username")
        password = cf.get(section, "password")
        if section != "gateway":
            env_roles[section] = ["%s@%s" % (username, host)]
        env_passwords["%s@%s" % (username, host)] = password
    # print evn_gateway, env_passwords, env_roles
    return evn_gateway, env_passwords, env_roles

env.gateway, env.passwords, env.roledefs = get_env_conf()

home_path="/mnt/windows"
supervisor_path="%s/DD_GAMES/DD_CONF/supervisor" % home_path
game_path="%s/DD_GAMES/Output" % home_path
comm_path="%s/DD_COMM/Output" % home_path
remote_path="/home/Output"

GameTypes = ("Alloc", "Game", "Robot", "HundredGame", "HundredRobot", "All")
# GameTag,GameFolderName,supervisor_file
GameTags = (("Baccarat", "BaccaratGame", "baccarat.conf"),
            ("BenzBmw", "BaccaratGame", "benzbmw.conf"),
            ("BullFight", "BullFight", "bull.conf"),
            ("BullFight2017", "BullFight2017", "bull2017.conf"),
            ("CompareBull", "CompareBullGame", "compbull.conf"),
            ("Maajan", "DoubleMahjong", "jaajan.conf"),
            ("Fish", "FishGame", "fish.conf"),
            ("Flower", "FlowerGame", "flower.conf"),
            ("Flower2017", "FlowerGame2017", "flower2017.conf"),
            ("Fruit", "FruitGame", "fruit.conf"),
            ("FryBull", "FryBullGame", "frybull.conf"),
            ("GuangDongMahjong", "GuangdongMahjong", "guangdong.conf"),
            ("HebeiMahjong", "HebeiMahjongGame", "hebeimahjong.conf"),
            ("HunanMahjong", "HuNanMahjongGame", ""),
            ("Land", "LandGame", "land.conf"),
            ("Land2017", "LandGame2017", "land2017.conf"),
            ("PaiGow", "PaiGowGame", "paigow.conf"),
            ("Showhand", "ShowhandGame", "showhand.conf"),
            ("SiChuanMahjong", "SichuanMahjongGame", "sichuanmahjong.conf"),
            ("WuhanMahjong", "WuhanMahjongGame", "wuhan.conf"),
            ("ZhengzhouMahjong", "ZhengzhouMahjong", "zhengzhou.conf"),
            ("FishParty", "FishPartyGame", "fishparty.conf"),
            )

def find_tag(game_folder_name):
    for tag in GameTags:
        if game_folder_name == tag[1]:
            return tag
    return None

def check_type(_type):
    for type in GameTypes:
        if type.lower() == _type.lower():
            return type
    return None

def parse_conf(conf_name, module_name):
    file_path = os.path.join(supervisor_path, conf_name)
    groups = {}
    last_group = ""
    last_program = ""
    belong_programs = []
    with open(file_path) as f:
        for line in f.readlines():
            line = line.strip()
            if line.startswith(";"):
                continue
            elif line.startswith("[group"):
                last_group = line.strip("[]").split(":")[1].strip()
                groups[last_group] = []
            elif line.startswith("programs"):
                programs = line.split("=")[1].split(",")
                groups[last_group] = [prog.strip() for prog in programs]
            elif line.startswith("[program"):
                last_program = line.strip("[]").split(":")[1].strip()
            elif line.startswith("command"):
                if os.path.basename(line.split("=")[1].split(" ")[0]) == module_name:
                    pair = []
                    for key, val in groups.iteritems():
                        if last_program in val:
                            pair.append(key)
                    pair.append(last_program)
                    belong_programs.append(pair)
    return belong_programs

def rcp(local_file, remote_file):
    put(local_file, remote_file)

def rcpget(remote_file, local_path):
    get(remote_file, local_path)

def ftp(local_file, remote_file):
    filename = os.path.basename(local_file)
    dirname = os.path.dirname(local_file)
    rdirname = os.path.dirname(remote_file)
    local(ftpcmd % (dirname, 'put', filename, filename))
    run(ftpcmd % (rdirname, 'get', filename, filename))

def depoly(local_file, dest_path):
    filename = os.path.basename(local_file)
    # put(local_file, "~/")
    execute(ftp, local_file, "~/")
    run("chmod +x ~/%s" % filename)
    run("sudo mv ~/%s %s" % (filename, dest_path))

def deploygame(local_file):
    relative_dir = os.path.relpath(local_file, game_path)
    execute(depoly, local_file, os.path.join(remote_path, relative_dir))

@roles("test-game")
def redist(game_folder_name, type, hundred = 0):
    tag = find_tag(game_folder_name)
    type = check_type(type)
    if not tag or not type:
        print "invalid game folder name"
        return

    modules = GameTypes[:-1] if type == "All" else [type]
    for mod in modules:
        module = tag[0] + mod
        subdir = mod.replace("Hundred", "") + "Server"
        local_file = "%s/%s/%s/bin/%s" % (game_path, game_folder_name, subdir, module)
        if not os.path.exists(local_file):
            print "%s not exist" % local_file
            continue
        execute(deploygame, local_file)
        programs = parse_conf(tag[2], module)
        if not programs:
            print "can't parse %s supervisor file for %s" % tag[2], module
            continue
        for prog in programs:
            run("sudo supervisorctl restart %s" % ":".join(prog))

@roles("test-game")
def redistcommon(module):
    local_file = "%s/Common/bin/%s" % (comm_path, module)
    execute(depoly, local_file, "%s/CommonServer/bin/" % remote_path)
    ctlname = module.replace("Server", "").lower()
    run("sudo supervisorctl restart common:%s" % ctlname)

@roles("test-game")
def mysqlserver():
    origin_file = "/mnt/windows/GoServers/src/ddservers/dbserver/bin/dbserver"
    local_file = os.path.join(os.path.dirname(origin_file), "MySQLServer")
    local("mv %s %s" % origin_file, local_file)
    execute(depoly, local_file, "%s/CommonServer/bin/" % remote_path)
    run("sudo supervisorctl restart common:mysql")

@roles("test-game")
def hallserver():
    local_file = "/mnt/windows/DD_COMM/Output/Common/bin/HallServer"
    execute(depoly, local_file, "%s/CommonServer/bin/" % remote_path)
    run("sudo supervisorctl restart hallserver:")

@roles("test-php")
def httpsvr():
    local_file = "/mnt/windows/HttpSvrUCenter/build/HttpSvr"
    execute(depoly, local_file, "/home/gameserver/Output/httpsvr/bin/HttpSvr")
    run("sudo supervisorctl restart httpsvr")

@roles("test-game")
def test():
    run("sudo ifconfig")
