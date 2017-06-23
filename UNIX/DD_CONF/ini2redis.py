#!/usr/bin/python
# -*- coding:utf-8 -*-
import ConfigParser
import os
import sys
import traceback
import redis
import xml.dom.minidom

gametags = [
    [1, "ShowHand", "梭哈"],
    [2, "BullFight", "牛牛"],
    [3, "LandLord", "斗地主"],
    [4, "Fruit", "水果机"],
    [5, "DragonTiger", "龙虎斗"],
    [6, "Taxas", "德州"],
    [7, "Flower", "金花"],
    [8, "WenZhouMahjong", "温州麻将"],
    [9, "Horse", "小马快跑"],
    [10, "FanFanLe", "翻翻乐"],
    [11, "Fish", "万炮捕鱼"],
    [12, "FryBull", "炸金牛"],
    [13, "CompallBull", "通比牛牛"],
    [14, "Tiger", "虎虎生威"],
    [15, "Maajan", "双人麻将"],
    [16, "SiChuanMahjong", "四川麻将"],
    [17, "Baccarat", "百家乐"],
    [18, "Paigow", "牌九"],
    [19, "BirdAnimal", "飞禽走兽"],
    [20, "BenzBMW", "奔驰宝马"],
    [21, "HeBeiMahjong", "河北麻将"],
    [22, "GuoBiaoMahjong", "国标麻将"],
    [23, "HuNanMahjong", "湖南麻将"],
    [24, "WuHanMahjongKaikou", "武汉麻将开口番"],
    [25, "WuHanMahjongKoukou", "武汉麻将口口番"],
    [26, "GuangDongMahjong", "广东麻将"],
    [27, "ZhengZhouMahjong", "郑州麻将"],
    [28, "TigerMachine", "老虎机"],
    [29, "BlackJack", "21点"],
    [30, "ShuangKou", "双扣"],
    [31, "BullFight2017", "百人牛牛2017"],
    [32, "Flower2017", "炸金花2017"],
    [33, "LandLord2017", "斗地主2017"],
    [50, "FishParty", "捕鱼派对"],

    [10000, "HallServer", "大厅服务器"],
    [10001, "UserServer", "用户服务器"],
    [10002, "MoneyServer", "金币服务器"],
    [10003, "MySqlServer", "数据库服务器"],
    [10004, "TrumptServer", "小喇叭服务器"],
    [10005, "RoundServer", "回合服务器"]
]

hosts = dict()
preconf = dict()

def find_tag(gamename):
    for tag in gametags:
        if tag[1].lower() == gamename.lower():
            return tag
    return None

def new_optionxform(optionstr):
    return optionstr

def import_ini(filepath, prefix=""):
    cf = ConfigParser.ConfigParser()
    cf.optionxform = new_optionxform
    cf.read(filepath)
    sections = cf.sections()
    for section in sections:
        items = cf.items(section, True)
        for key, value in items:
            value = value.strip()
            global hosts
            fields = value.split(":")
            if hosts.has_key(fields[0]):
                if len(fields) == 1:
                    value = hosts[fields[0]]
                elif len(fields) == 2:
                    value = hosts[fields[0]] + ":" + fields[1]
            conn.hset(prefix+section, key, value)

def import_xml(filepath, prefix=""):
    filename = os.path.basename(filepath)
    domtree = xml.dom.minidom.parse(filepath)
    backserver = domtree.documentElement.getElementsByTagName("*")
    servers = backserver[0].getElementsByTagName("*")
    for server in servers:
        global hosts
        if server.hasAttribute("host"):
            if hosts.has_key(server.getAttribute("host")):
                server.setAttribute("host", hosts[server.getAttribute("host")])
            if server.hasAttribute("db") and preconf.has_key("mysql.%s.user" % server.getAttribute("db")):
                server.setAttribute("user", preconf["mysql.%s.user" % server.getAttribute("db")])
                server.setAttribute("passwd", preconf["mysql.%s.passwd" % server.getAttribute("db")])
        else:
            nodes = backserver[0].getElementsByTagName("*")
            for node in nodes:
                if node.hasAttribute("host") and hosts.has_key(node.getAttribute("host")):
                    node.setAttribute("host", hosts[node.getAttribute("host")])

    #print str(domtree.toxml(domtree.encoding))
    fields = filename.split(".")
    if len(fields) == 3 and fields[1] != "hundred":
        conn.hset(prefix + fields[1] + ":" + fields[0] + "." + fields[2], "content", str(domtree.toxml(domtree.encoding)))
    else:
        conn.hset(prefix + filename, "content", str(domtree.toxml(domtree.encoding)))

def import_txt(filepath, key):
    with open(filepath) as f:
        data = f.read()
        subkey = os.path.basename(filepath)
        conn.hset(key, subkey, data)

def import_robot_etc(dirname):
    print "开始导入Robot Etc"
    filenames = os.listdir(dirname)
    for filename in filenames:
        if os.path.splitext(filename)[1] == ".txt":
            import_txt(os.path.join(dirname, filename), "RobotEtc")

def import_game_path(gamename):
    tag = find_tag(gamename)
    if not tag:
        print gamename, "不是正确的游戏名称"
        return
    modulenames = os.listdir(tag[1])
    for modulename in modulenames:
        if modulename in ["Alloc", "Game", "Robot"]:
            print "开始导入" + tag[2] + " " + modulename + "..."
            filenames = os.listdir(os.path.join(tag[1], modulename))
            for filename in filenames:
                if os.path.splitext(filename)[1] == ".ini":
                    fields = filename.split(".")
                    middle = "Hundred" if len(fields) == 3 and fields[1].lower() == "hundred" else ""
                    import_ini(os.path.join(tag[1], modulename, filename), tag[1]+middle+modulename+":")
                elif os.path.splitext(filename)[1] == ".xml":
                    import_xml(os.path.join(tag[1], modulename, filename), tag[1]+modulename+":")

def import_common_path(dirname):
    servernames = os.listdir(dirname)
    for servername in servernames:
        tag = find_tag(servername)
        if not tag:
            print servername, "不是正确的公共服务器名称"
            continue
        print "开始导入" + tag[2] + " " + servername + "..."
        filenames = os.listdir(os.path.join(dirname, tag[1]))
        for filename in filenames:
            if os.path.splitext(filename)[1] == ".ini":
                import_ini(os.path.join(dirname, servername, filename), tag[1] + ":")
            elif os.path.splitext(filename)[1] == ".xml":
                import_xml(os.path.join(dirname, servername, filename), tag[1] + ":")

def import_all():
    dirnames = os.listdir("./")
    for dirname in dirnames:
        if os.path.isdir(dirname):
            import_subpath(dirname)
        elif os.path.splitext(dirname)[1] == ".ini":
            print "开始导入公共服务器配置..."
            import_ini(dirname)

def import_subpath(dirname):
    if dirname == "RobotEtc":
        import_robot_etc(dirname)
    elif dirname == "CommonServer":
        import_common_path(dirname)
    elif dirname in ["supervisor","python-pkg",".git", "fabric"]:
        pass
    else:
        import_game_path(dirname)

def import_hosts():
    with open("hosts") as f:
        for line in f.readlines():
            line = line.strip()
            if line.startswith("#") or len(line) == 0:
                continue
            fields = line.split()
            if len(fields) != 2:
                continue
            ip,domain = fields[0],fields[1]
            global hosts
            hosts[domain] = ip
        return True
    return False

def import_preconf():
    with open("preconf") as f:
        for line in f.readlines():
            line = line.strip()
            if line.startswith("#") or len(line) == 0:
                continue
            fields = line.split("=")
            if len(fields) != 2:
                continue
            key, val = fields[0].strip(),fields[1].strip()
            global preconf
            preconf[key] = val
        return True
    return False

def import_gametags():
    for tag in gametags:
        conn.hset("GameTagName", str(tag[0]), ",".join(tag[1:]))

def import_roomconfig_subpath(subpath):
    tag = find_tag(subpath)
    if not tag:
        print subpath, "不是正确的游戏名称"
        return
    if not os.path.exists(subpath):
        return
    filenames = os.listdir(subpath)
    for filename in filenames:
        if os.path.splitext(filename)[1] == ".txt":
            pipe = conn2.pipeline()
            with open(os.path.join(subpath, filename)) as f:
                for line in f.readlines():
                    line = line.strip()
                    if len(line) == 0:
                        continue
                    pipe.execute_command(*line.split())
                try:
                    print pipe.execute()
                    print "导入",tag[2],"成功"
                except:
                    print "导入",tag[2],"异常"
                    traceback.print_exc()

def import_roomconfig(subpaths):
    print "开始导入房间配置到redis-6386"
    if not subpaths:
        for tag in gametags:
            subpath = tag[1]
            import_roomconfig_subpath(subpath)
    else:
        for subpath in subpaths:
            import_roomconfig_subpath(subpath)

conn = redis.Redis(host='config.dd.redis',port=6390)
conn2 = redis.Redis(host='config.dd.redis',port=6386)

if __name__ == "__main__":
    if not import_hosts():
        print "导入hosts失败"
        exit(0)
    if not import_preconf():
        print "导入preconf失败"
        exit(0)
    import_gametags()
    if len(sys.argv) >= 2:
        if sys.argv[1].lower() == "roomconfig":
            import_roomconfig(sys.argv[2:])
        else:
            gamenames = sys.argv[1:]
            for gamename in gamenames:
                import_subpath(gamename)
    else:
        import_all()