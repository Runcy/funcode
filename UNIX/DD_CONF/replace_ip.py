#!/usr/bin/python
# -*- coding:utf-8 -*-

import sys, os, hashlib, datetime, platform, time, pickle,shutil


ini_files = []
xml_files = []
def find_config_files_rec(dirname):
    files = os.listdir(dirname)
    for f in files:
        subpath = os.path.join(dirname, f)
        #print subpath
        if os.path.isfile(subpath):
            filename = os.path.basename(subpath)
            if ".ini" in filename:
                ini_files.append(os.path.abspath(subpath))
            elif ".xml" in filename:
                xml_files.append(os.path.abspath(subpath))
            else:
                pass
        else:
            find_config_files_rec(subpath)

def replace_ini_file(ini):
    lines = open(ini, 'r').readlines()
    section = ""
    flen = len(lines)
    for i in range(flen):
        line = lines[i]
        if line[0] == '#':
            continue
        if line[0] == '[':
            section = line[1:len(line)-2]
            section = section.upper()
            continue
            #print section
        if "REDIS" in section: # replace redis ip
            if "=" in line:
                lines[i] = lines[i].replace("10.25.74.110", "116.31.115.98")
        else:                  # replace server ip
            if "=" in line:
                lines[i] = lines[i].replace("10.25.74.110", "183.60.111.136")
    # write to ini file now
    open(ini, 'w').writelines(lines)

def replace_xml_file(xml):
    lines = open(xml, 'r').readlines()
    flen = len(lines)
    for i in range(flen):
        lines[i] = lines[i].replace("10.25.74.110", "183.60.111.136")
        lines[i] = lines[i].replace("10.25.74.95", "116.31.115.98")
    # write to xml file now
    #open(xml, 'w').writelines(lines)

if __name__ == "__main__":
    find_config_files_rec("./")
    for r in ini_files:
        replace_ini_file(r)
    for r in xml_files:
        replace_xml_file(r)