# -*- coding:utf-8 -*-
from httptest import *
from dduser import *
import socket

uids=set()
def php_get_uid(user):
    response = request_guest_login_php(user.uuid, user.device_token)
    if response is not None:
        if response["flag"] == 200:
            data = response["data"]
            user.uid = data["mid"]
            print user.uid,data["money"]
            uids.add(user.uid)
    # print "mid count:",len(uids)
    # print uids

def do_common_request(user, pack, noresp=False):
    try:
        if user.sock is None:
            user.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            user.sock.connect((user.host, user.port))
        user.sock.send(pack.getpackbuff())
        if noresp:
            return 0, "", None
        data = user.sock.recv(4)
        packlen, = struct.unpack("!i", data)
        data = user.sock.recv(packlen)
    except:
        traceback.print_exc()
        user.sock.close()
        user.sock = None
    outpack = Packet()
    outpack.unpack(data, False)
    retno = outpack.readshort()
    retmsg = outpack.readstring()
    return retno, retmsg, outpack

def print_result(outpack):
    print "uid",outpack.readint()
    print "money",outpack.readint64()
    print "safemoney",outpack.readint64()
    print "wintimes",outpack.readint()
    print "losetimes",outpack.readint()
    print "runawaytimes",outpack.readint()
    print "maxwin",outpack.readint()

def update_money(user):
    pack = Packet()
    pack.main_cmd = 0x0462
    pack.type = game_id
    pack.uid = user.uid
    pack.writeint(pack.uid)
    pack.writebyte(1)
    pack.writeint64(1000000L)
    pack.end(False)
    retno, retmsg, outpack = do_common_request(user, pack)
    print retno, retmsg
    if retno == 0:
        print_result(outpack)

def get_money(user):
    pack = Packet()
    pack.main_cmd = 0x0461
    pack.type = game_id
    pack.uid = user.uid
    pack.writeint(pack.uid)
    pack.writeint(20301)
    pack.end(False)
    retno, retmsg, outpack = do_common_request(user, pack)
    print retno, retmsg
    print_result(outpack)

def get_money_260(user):
    pack = Packet()
    pack.main_cmd = 0x0260
    pack.type = game_id
    pack.uid = user.uid
    pack.writeint(pack.uid)
    pack.writeshort(1)
    pack.end(False)
    retno, retmsg, outpack = do_common_request(user, pack)
    print retno, retmsg
    print outpack.readint()
    print outpack.readint64()
    print outpack.readshort()
    # print_result(outpack)

def sync_money(user):
    pack = Packet()
    pack.main_cmd = 0x0150
    pack.type = 3
    pack.uid = user.uid
    pack.writeint64(1000)
    pack.writeint64(1000)
    pack.writeint(0)
    pack.writeint(0)
    pack.writeint(0)
    pack.writeint(0)
    pack.end(False)
    retno, retmsg, outpack = do_common_request(user, pack, True)
    print retno, retmsg

def notify_update_money(user):
    pack = Packet()
    pack.main_cmd = 0x0090
    pack.uid = user.uid
    pack.type = user.gameid
    pack.writeint(user.uid)
    pack.writestring("10")
    pack.end(False)
    retno, retmsg, outpack = do_common_request(user, pack)
    print "update money", retno, retmsg

def update_game_round(user):
    pack = Packet()
    pack.main_cmd = 0x0640
    pack.uid = user.uid
    pack.type = user.gameid
    pack.writestring('{"uid":[6871420], "test":1}')
    pack.end(False)
    retno, retmsg, outpack = do_common_request(user, pack)
    print "update game round", retno, retmsg

def logchip_round(user):
    pack = Packet()
    pack.main_cmd = 0x0120
    pack.uid = user.uid
    pack.type = user.gameid
    pack.writeint(user.uid)
    pack.writeint64(100)
    pack.writeint(1)
    pack.writeint(1)
    pack.writeint(0)
    pack.writeint(10)
    pack.writeint(2)
    pack.writeint(20700)
    pack.writeint(1)
    pack.writeint(1)
    pack.writestring("")
    pack.writeint(0)
    pack.writeint64(6000)
    pack.writeint(time.time())
    pack.writeint(time.time())
    pack.writeint(1)
    pack.end(False)
    retno, retmsg, outpack = do_common_request(user, pack, True)
    print "logchip", retno, retmsg

def bench_money(user):
    php_get_uid(user)
    while True:
        try:
            # get_money(user)
            break
            # update_money(user)
            # logchip_round(user)
            # sync_money(user)
        except:
            pass
