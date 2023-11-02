CC = gcc
CFLAGS = -I/usr/include/fastdfs -I/usr/include/fastcommon
LDFLAGS = -lfdfsclient

SOURCES = $(wildcard *.c)
EXECUTABLE = upload_makefile

all: $(SOURCES)
	    $(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCES) $(LDFLAGS)

clean:
	    rm -f $(EXECUTABLE)

