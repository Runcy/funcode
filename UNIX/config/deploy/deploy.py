#!/usr/bin/python
# -*- coding:utf-8 -*-

import os
import sys
import traceback
import tarfile
import time
import shutil
from ftplib import FTP

def show_head():
    print("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
    print("             DD Server部署脚本")
    print("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
    
    print("")
    print("(1) 检查依赖软件")
    print("(2) 检查config.dd.redis")
    print("(3) 部署服务器")
    return raw_input("请输入选择指令[1-3]:")

def check_conf_redis(conf):
    conflines = open("/etc/hosts").readlines()
    flen = len(conflines)
    redis = __import__("redis")
    have_conf = False
    for i in range(flen):
        conflines[i] = conflines[i]
        if "config.dd.redis" in conflines[i]:
            have_conf = True
            print("已配置config redis:%s" % conflines[i])
            break
    if not have_conf:
        print("未配置config.dd.redis，写入/etc/hosts, ip = %s" % conf.config_redis_ip)
        conflines.append("%s config.dd.redis" % conf.config_redis_ip)
    open("/etc/hosts", 'w').writelines(conflines)
    try:
        print("尝试连接config redis...")
        r = redis.Redis(host='config.dd.redis', port=6390, db=0)
        if r == None:
            print("please check '/etc/hosts' !")
        else:
            print("连接config redis成功，dbsize = %s" % r.dbsize())
    except:
        print("please check '/etc/hosts' !")

def get_tar_filename(filename):
    return filename  + ".tar.gz"

ftpsvr = "192.126.118.232"
ftpuser = "DDGame"
ftppwd = "9v9SxE2VOi7xbECP"
localpath = "./dep/"
def ftp_download(filename):
    ftp = FTP()
    ftp.connect(ftpsvr, 21, 10)
    ftp.login(ftpuser, ftppwd)
    #print(ftp.getwelcome())
    print("begin download %s now" % filename)
    datapath = "/"    
    localfile = localpath + get_tar_filename(filename)
    remotepath = datapath + get_tar_filename(filename)
    fp = open(localfile, 'wb')
    bufsize = 1024
    ftp.retrbinary('RETR ' + remotepath, fp.write, bufsize)
    fp.close()
    ftp.quit()
    time.sleep(2)

def extract_file(filename):
    curdir = os.getcwd()
    os.chdir(localpath)
    tar = tarfile.open(get_tar_filename(filename))
    tar.extractall(".")
    tar.close()
    os.chdir(filename)
    os.system("python setup.py install")
    os.chdir(curdir)

ddconf_local_path = "/home/Output/DD_CONF"
dep_targz_files = [
    "setuptools-28.0.0", 
    "redis-2.10.5", 
    "pycrypto-2.6.1", 
    "ecdsa-0.13", 
    "paramiko-1.17.2",
    "Fabric-1.12.0",
    "meld3-1.0.2",
    "supervisor-3.3.1"
]

def deploy_server(conf):
    # import config data to redis
    os.chdir("../DD_CONF/")
    os.system("python ini2redis.py")

    # start server now...
    for svr in conf.svr_groups:
        print("restart %s now..." % svr)
        os.system("sudo supervisorctl restart %s:" % svr)

    os.system("supervisorctl status")

def check_dep_files():
    print("check dep files now...")
    if not os.path.exists(localpath):
        os.mkdir(localpath)
        for df in dep_targz_files:
            ftp_download(df)
    
    os.system("yum -y install gcc gcc-c++ python-devel")
    for df in dep_targz_files:
        extract_file(df)

    # config supervisor
    os.system("echo_supervisord_conf > /etc/supervisord.conf")
    conflines = open("/etc/supervisord.conf").readlines()
    flen = len(conflines)
    for i in range(flen):
        conflines[i] = conflines[i]
        if ";[include]" in conflines[i]:
            conflines[i] = conflines[i][1:]
            conflines[i+1] = "files = %s/supervisor/*.conf" % ddconf_local_path
            break
    open("/etc/supervisord.conf", 'w').writelines(conflines)
    os.system("python /usr/local/bin/supervisord -c /etc/supervisord.conf")
    os.system("supervisorctl status")
    
if __name__ == "__main__":
    curpath = os.getcwd()
    if not os.path.isfile(curpath + "/conf.py"):
        shutil.copy2("conf.tmpl", curpath + "/conf.py")
        print("已生成conf.py，请先编辑该文件, 设置必要的参数然后再次运行deply.py")
        sys.exit(1)
    conf = __import__("conf")

    choise = int(show_head())
    while choise <= 3:
        if choise == 1:
            check_dep_files()
        elif choise == 2:
            check_conf_redis(conf)
        elif choise == 3:
            deploy_server(conf)
        else:
            print("invalid choise!")
        
        choise = int(show_head())
