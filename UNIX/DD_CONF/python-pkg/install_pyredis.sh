#!/bin/bash

tar zxf setuptools-28.0.0.tar.gz
cd setuptools-28.0.0
sudo python setup.py install
cd ..
rm -rf setuptools-28.0.0

tar zxf redis-2.10.5.tar.gz
cd redis-2.10.5
sudo python setup.py install
cd ..
rm -rf redis-2.10.5