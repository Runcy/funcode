#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := ReleaseDDCrypto

#Toolchain
CC := gcc
CXX := g++
LD := $(CXX)
AR := ar
OBJCOPY := objcopy

#Additional flags
PREPROCESSOR_MACROS := NDEBUG=1 RELEASE=1 __DDCRYPTO__
INCLUDE_DIRS := /usr/include/mysql/ comm comm/Util ./ third/HttSvrComm/include third/HttSvrComm/include/Log third/HttSvrComm/include/hiredis /third/curl/include
LIBRARY_DIRS := /usr/lib64/mysql third/HttSvrComm/lib /third/curl/lib
LIBRARY_NAMES := dl event mysqlclient pthread stdc++ m HttpSvrComm curl
ADDITIONAL_LINKER_INPUTS := 
MACOS_FRAMEWORKS := 
LINUX_PACKAGES := 

CFLAGS := -ggdb -ffunction-sections -O3
CXXFLAGS := -ggdb -ffunction-sections -O3 -std=c++0x -D__STDC_FORMAT_MACROS
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections
COMMONFLAGS := 
LINKER_SCRIPT := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group


ifeq ($(__DDCRYPTO__),)
INCLUDE_DIRS += third/openssl/include third/DDCrypt/include
LIBRARY_DIRS += third/openssl/linux_lib third/DDCrypt/lib
LIBRARY_NAMES += crypto DDCrypt
endif

#Additional options detected from testing the toolchain
IS_LINUX_PROJECT := 1
