# -*- coding:utf-8 -*-

import random
import struct
import time
import json
import socket
import traceback
from utils import *
from config import *
from httptest import *

encrypt_map = (0x51,0xA1,0x9E,0xD7,0x1E,0x83,0x1C,0x2D,0xE9,0x77,0x3D,0x13,0x93,0x10,0x45,0xFF,
	0x6D,0xC9,0x20,0x2F,0x1B,0x82,0x1A,0x7D,0xF5,0xCF,0x52,0xA8,0xD2,0xA4,0xB4,0x0B,
	0x31,0x97,0x57,0x19,0x34,0xDF,0x5B,0x41,0x58,0x49,0xAA,0x5F,0x0A,0xEF,0x88,0x01,
	0xDC,0x95,0xD4,0xAF,0x7B,0xE3,0x11,0x8E,0x9D,0x16,0x61,0x8C,0x84,0x3C,0x1F,0x5A)

class Packet:
    def __init__(self):
        self.packlen = 0 #int
        self.version = 0 #byte
        self.main_cmd = 0 #short
        self.sub_cmd = 0 #byte
        self.encode = 0 #byte
        self.seq = 0 #short
        self.source = 0 #byte
        self.type = 0 #byte
        self.uid = 0 #int
        self.code = 0 #byte
        self.head = bytearray()
        self.body = bytearray()
        self.pos = 0

    def getheadlen(self):
        return struct.calcsize("!iBhBBhBBiB")

    def packhead(self):
        return struct.pack("!iBhBBhBBiB", self.packlen, self.version, self.main_cmd, self.sub_cmd, self.encode, self.seq,
                    self.source, self.type, self.uid, self.code)

    def writebyte(self, val):
        self.body += struct.pack("B", val)

    def writeshort(self, val):
        self.body += struct.pack("!h", val)

    def writeint(self, val):
        self.body += struct.pack("!i", val)

    def writeint64(self, val):
        self.body += struct.pack("!q", val)

    def writestring(self, val):
        self.body += struct.pack("!i", len(val))
        self.body += val

    def encrypt(self):
        self.encode = random.randint(0, len(encrypt_map)-1)
        code = encrypt_map[self.encode]
        for i in xrange(0,len(self.body)):
            self.code += self.body[i]
            if self.code > 256:
                self.code -= 256
            self.body[i] ^= code
        self.code = 256 - self.code

    def decrypt(self):
        checkcode = 0
        code = encrypt_map[self.encode]
        for i in range(0,len(self.body)):
            self.body[i] ^= code
            checkcode += self.body[i]
            if checkcode > 256:
                checkcode -= 256
        # print "checkcode",checkcode,self.code


    def end(self, encrypted=True):
        self.packlen = len(self.body) + self.getheadlen() - 4
        if encrypted:
            self.encrypt()
        self.head = self.packhead()

    def getpackbuff(self):
        return self.head + self.body

    def unpack(self, data, decypted=True):
        self.version, self.main_cmd, self.sub_cmd, self.encode, self.seq, self.source, self.type, self.uid, self.code = struct.unpack("!BhBBhBBiB", data[:self.getheadlen()-4])
        self.body = bytearray(data[self.getheadlen()-4:])
        if decypted:
            self.decrypt()

    def readbyte(self):
        val, = struct.unpack("B", str(self.body[self.pos:self.pos+1]))
        self.pos += 1
        return val

    def readshort(self):
        val, = struct.unpack("!h", str(self.body[self.pos:self.pos+2]))
        self.pos += 2
        return val

    def readint(self):
        val, = struct.unpack("!i", str(self.body[self.pos:self.pos+4]))
        self.pos += 4
        return val

    def readint64(self):
        val, = struct.unpack("!q", str(self.body[self.pos:self.pos+8]))
        self.pos += 8
        return val

    def readstring(self):
        len, = struct.unpack("!i", str(self.body[self.pos:self.pos+4]))
        self.pos += 4
        val = str(self.body[self.pos:self.pos+len])
        self.pos += len
        return val

state_none,state_login_php,state_login_hall,state_alloc,state_enter,state_leave, = range(0, 6)
class MonitorUser:
    def __init__(self):
        self.uid = 0
        self.name = ""
        self.coin = 0L
        self.uuid=get_uuid()
        self.device_token=get_uuid()
        self.host= ""
        self.port=26000
        self.mtkey = 0
        self.sock = None
        self.state = 0
        self.gameid = game_id
        self.level = test_level
        self.tid = 0
        self.seqno = 1

    def get_seqno(self):
        self.seqno += 1
        return self.seqno

    def run_loop(self):
        try:
            self.state = state_login_php
            while True:
                if self.state == state_login_php:
                    if not self.login_php():
                        print "guest login failed"
                    else:
                        self.state = state_login_hall
                if self.state == state_login_hall:
                    print "start login hall",self.host,self.port
                    try:
                        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        self.sock.connect((self.host, self.port))
                    except:
                        self.sock.close()
                        # traceback.print_exc()
                        print "connect failed"
                        time.sleep(10)
                        continue
                    self.login_hall()
                elif self.state == state_alloc:
                    self.alloc_table()
                elif self.state == state_enter:
                    self.enter_game()
                elif self.state == state_leave:
                    self.leave_game()
        except Exception,e:
            traceback.print_exc()

    def do_request(self, pack):
        try:
            self.sock.send(pack.getpackbuff())
            data = self.sock.recv(4)
            if len(data) != 4:
                return -1, "error", None
            packlen, = struct.unpack("!i", data)
            data = self.sock.recv(packlen)
            if len(data) != packlen:
                return -1, "error", None
        except:
            self.sock.close()
            return -1, "except", None
        outpack = Packet()
        outpack.unpack(data)
        retno = outpack.readshort()
        retmsg = outpack.readstring()
        return retno, retmsg, outpack

    def login_php(self):
        response = None
        try:
            response = request_guest_login_php(self.uuid, self.device_token)
        except:
            traceback.print_exc()
        if response is not None:
            if response["flag"] == 200:
                data = response["data"]
                self.uid = data["mid"]
                self.coin = data["money"]
                self.mtkey = data["mtkey"].encode("utf8")
                self.name = data["mnick"].encode("utf8")
                server_info = data["serverInfo"]
                self.host = server_info["hallAddr"]["ip"]
                self.port = server_info["hallAddr"]["port"]
            return True
        return False

    def login_hall(self):
        pack = Packet()
        self.seqno = self.get_seqno()
        pack.main_cmd = 0x0001
        pack.uid = self.uid
        pack.type = self.gameid
        pack.writeint(self.uid)
        pack.writestring(self.name)
        pack.writeint64(self.coin)
        prop = {
            "cid":0,
            "sid":ACCOUNT_SID,
            "pid":product_id
        }
        pack.writestring(json.dumps(prop))
        pack.writestring(self.mtkey)
        pack.end()
        retno, retmsg, outpack = self.do_request(pack)
        print "login hall", retno, retmsg
        if retno >= 0:
            self.state = state_alloc
        elif not outpack:
            self.state = state_login_hall
            time.sleep(5)
        else:
            time.sleep(3)

    def alloc_table(self):
        pack = Packet()
        self.seqno = self.get_seqno()
        pack.main_cmd = 0x0003
        pack.uid = self.uid
        pack.type = self.gameid
        pack.writeint(self.uid)
        pack.writeshort(self.level)
        pack.writeint64(self.coin)
        pack.end()
        retno, retmsg, outpack = self.do_request(pack)
        print "alloc table", retno, retmsg
        if not outpack:
            self.state = state_login_hall
            return
        if retno >= 0:
            self.state = state_enter
            try:
                outpack.readint()
                self.tid = outpack.readint()
                level = outpack.readshort()
                print "level:", level, "tid:", self.tid
            except:
                self.state = state_login_hall
                return
        else:
            time.sleep(5)

    def enter_game(self):
        pack = Packet()
        self.seqno = self.get_seqno()
        pack.main_cmd = 0x0200
        pack.uid = self.uid
        pack.type = self.gameid
        pack.writeint(self.uid)
        pack.writestring(self.name)
        pack.writeint(self.tid)
        pack.writeint64(self.coin)
        pack.writeshort(self.level)
        prop = {
            "cid":0,
            "sid":ACCOUNT_SID,
            "pid":product_id,
            "sex":1
        }
        pack.writestring(json.dumps(prop))
        pack.end()
        retno, retmsg, outpack = self.do_request(pack)
        print "enter game", retno, retmsg
        if not outpack:
            self.state = state_login_hall
            return
        if retno >= 0:
            self.state = state_leave
        else:
            time.sleep(5)
            self.state = state_alloc

    def leave_game(self):
        pack = Packet()
        pack.main_cmd = 0x0250
        self.seqno = self.get_seqno()
        pack.uid = self.uid
        pack.type = self.gameid
        pack.writeint(self.uid)
        pack.writeint(self.tid)
        pack.end()
        retno, retmsg, outpack = self.do_request(pack)
        print "leave game", retno, retmsg
        if not outpack:
            self.state = state_login_hall
            return
        time.sleep(10)
        self.state = state_alloc
