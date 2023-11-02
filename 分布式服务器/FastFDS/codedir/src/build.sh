#!/bin/bash

CC=gcc
CFLAGS="-I/usr/include/fastdfs -I/usr/include/fastcommon"
LDFLAGS="-lfdfsclient"
SOURCES="*.c"
EXECUTABLE="upload_build"

$CC $CFLAGS -o $EXECUTABLE $SOURCES $LDFLAGS
