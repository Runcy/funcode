# -*- coding:utf-8 -*-

import base64
import random,json,urllib2,traceback
import time
from utils import *
from config import *

def encode_data(data):
    base64data = base64.encodestring(data)
    return base64.encodestring(base64data + genmd5(base64data))

def decode_data(data):
    return base64.decodestring(base64.decodestring(data)[0:-32])

def encode_data_v1(data):
    data = bytearray(data)
    code = random.randint(0,127)
    for i in xrange(0,len(data)):
        data[i] = data[i]^code
    return base64.encodestring(chr(code)+str(data))

def decode_data_v1(data):
    rawdata = base64.decodestring(data)
    rawdata = bytearray(rawdata)
    code = rawdata[0]
    rawdata = rawdata[1:]
    for i in xrange(0, len(rawdata)):
        rawdata[i] = rawdata[i]^code

    return str(rawdata)

def gen_sig(json_data):
    allmsg = ''
    sortedkeys = sorted(json_data.keys())
    for key in sortedkeys:
        if key == 'param':
            params = json_data[key]
            pkeys = sorted(params.keys())
            for subkey in pkeys:
                allmsg += str(params[subkey])
        else:
            allmsg += str(json_data[key])
    json_data["sig"] = genmd5(php_secrect + genmd5(allmsg))
    return json_data

def send_common_req(tag, url, json_data, params):
    json_data["param"] = params
    gen_sig(json_data)
    json_data["param"] = encode_data_v1(json.dumps(params))
    post_data = "api=" + json.dumps(json_data)
    response = None
    try:
        req = urllib2.Request(url, post_data)
        response = urllib2.urlopen(req).read()
        result = json.loads(response)
        result["data"] = json.loads(decode_data_v1(result["data"]))
        result["tag"] = tag
        return result
    except:
        print response
        traceback.print_exc()

    return None

def build_common_data():
    req_body = {
        "mid":0,
        "cid":0,
        "sid":ACCOUNT_SID,
        "ctype":ctype,
        "pid":product_id,
        "mtkey":"",
        "api":ctype,
        "gameid":game_id,
        "versions":version,
        "productid":product_id,
        "phpversions": 1
    }
    return req_body

def request_api_url():
    params = {"appkey":app_key,
              "version":version,
              "time":int(time.time())}
    # data = urllib.urlencode(params)
    data = json.dumps(params)
    req = urllib2.Request(route_url, encode_data(data))
    response = urllib2.urlopen(req).read()
    result = json.loads(decode_data(response))
    print result
    if result["code"] == 200:
        api_url = result["url"]

def request_get_version():
    post_data = build_common_data()
    params = {"method":"Clientbase.getGameVersion",
              "gameid":game_id,
              "productid":product_id
              }
    result = send_common_req(params["method"], api_url, post_data, params)
    print result
    if result["flag"] == 200:
        return True
    return False

def request_user_login_php():
    post_data = build_common_data()
    params = {
        "method":"Login.account",
        "device_name":"monitor",
        "os_version":"monitor",
        "net_type":"",
        "device_no":"",
        "mnick":"monitor",
        "mobileOperator":"",
        "phoneno":"",
        "username": account,
        "password": password,
        "sitemid": "",
        "sex": 1,
        "siteurl": "",
        "deviceToken": "abcd123456",
        "uuid": "abcd123456"
    }
    result = send_common_req(params["method"], api_url, post_data, params)
    print result
    if result["flag"] == 200:
        return True
    return False

def request_guest_login_php(uuid1, uuid2):
    post_data = build_common_data()
    params = {
        "method":"Login.guest",
        "device_name": "monitor",
        "os_version": "monitor",
        "net_type": "",
        "device_no": "",
        "mnick": "monitor",
        "mobileOperator": "",
        "deviceToken": uuid1,
        "uuid": uuid2
    }
    result = send_common_req(params["method"], api_url, post_data, params)
    # print result["flag"]
    # print result
    # print json.dumps(result, indent=True)
    return result

def request_room_config():
    post_data = build_common_data()
    params = {
        "method":"Clientbase.getProductRoomConfig"
    }
    result = send_common_req(params["method"], api_url, post_data, params)
    print result
    if result["flag"] == 200:
        return True
    return False
