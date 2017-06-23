#!/bin/bash

sudo yum -y install gcc gcc-c++ python-devel

tar zxf setuptools-28.0.0.tar.gz
cd setuptools-28.0.0
sudo python setup.py install
cd ..
rm -rf setuptools-28.0.0

tar zxf pycrypto-2.6.1.tar.gz
cd pycrypto-2.6.1
sudo python setup.py install
cd ..
rm -rf pycrypto-2.6.1

tar zxf ecdsa-0.13.tar.gz
cd ecdsa-0.13
sudo python setup.py install
cd ..
rm -rf ecdsa-0.13

tar zxf paramiko-1.17.2.tar.gz
cd paramiko-1.17.2
sudo python setup.py install
cd ..
rm -rf paramiko-1.17.2

tar zxf Fabric-1.12.0.tar.gz
cd Fabric-1.12.0
sudo python setup.py install
cd ..
rm -rf Fabric-1.12.0


