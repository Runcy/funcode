# -*- coding:utf-8 -*-

import threading
from utils import *
from dduser import *
from config import *
from httptest import *
from moneytest import *

# from gevent import monkey
# import gevent
# monkey.patch_all()

def worker(index):
    user = MonitorUser()
    # user.host, user.port = ("192.168.115.70", 20701)
    # user.host, user.port = ("192.168.115.153", 20700)
    user.uuid, user.device_token = uuid_set[index]
    # user.run_loop()
    bench_money(user)

def bench_test():
    for i in xrange(0, thread_num):
        t = threading.Thread(target=worker,args=(i,))
        t.start()

# def bench_gevent_test():
#     Greenlets=[]
#     for i in xrange(0, thread_num):
#         t = gevent.Greenlet(worker)
#         Greenlets.append(t)
#         t.start()
#     gevent.joinall(Greenlets)

if __name__ == "__main__":
    read_uuid_file()
    user = MonitorUser()
    user.uuid, user.device_token = get_uuid(), get_uuid()
    # user.uid = 6871420#5227706 ##6877622#
    user.uid = 5228231#5228228
    user.host, user.port = ("192.168.115.70", 20600)
    # user.host, user.port = ("192.168.115.153", 20600)
    # user.host, user.port = ("192.126.114.225", 20601)
    # user.host, user.port = ("127.0.0.1", 20700)
    # notify_update_money(user)
    # request_api_url()
    # request_guest_login_php(get_uuid(), get_uuid())
    # php_get_uid(user)
    # request_get_version()
    # request_room_config()
    # bench_test()
    # get_money(user)
    update_money(user)
    # update_game_round(user)
    # logchip_round(user)
    # sync_money(user)
    # get_money_260(user)
