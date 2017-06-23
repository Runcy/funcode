import hashlib
import uuid as uuidlib

uuid_set = []

def get_uuid():
    return str(uuidlib.uuid1()).replace("-", "")

def gen_uuid_file():
    f = open("uuid.txt", "w")
    for i in xrange(0, 10000):
        f.write(get_uuid() + " " + get_uuid() + "\n")
    f.close()

def read_uuid_file():
    with open("uuid.txt") as f:
        for line in f.readlines():
            uuid_set.append(line.strip().split(" "))

def genmd5(data):
    m5 = hashlib.md5()
    m5.update(data)
    return m5.hexdigest()
