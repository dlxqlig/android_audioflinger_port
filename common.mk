HOST=i686-cm-linux
CC=gcc
CXX=g++
AR=ar
AR_RC=ar rc
AS=as
LD=ld
M4=m4
NM=nm
RANLIB=ranlib
BISON=bison
STRIP=strip
OBJCOPY=objcopy
OBJDUMP=objdump
STRINGS=strings
CXXCPP=$(CXX) -E

CPP_DEFS =-DHAVE_CONFIG_H
CPP_OPTS =-Wall -O0 -msse3 -mfpmath=sse -mtune=atom -march=atom -ffast-math -funroll-loops -DHAVE_PTHREADS -DHAVE_SYS_UIO_H -DHAVE_ENDIAN_H

CFLAGS = -I$(BASE_DIR)/include -I$(BUILD_ROOT)/addpack/usr/include -I$(BUILD_ROOT)/fsroot/include -I$(BUILD_ROOT)/fsroot/usr/include -I$(BUILD_DEST)/include -I$(BUILD_DEST)/include/linux_user -I$(BUILD_DEST)/usr/include
CFLAGS += ${CPP_OPTS} ${CPP_DEFS}

LDFLAGS = -L$(BASE_DIR)
CPPFLAGS = $(CFLAGS) 
INSTALL=/usr/bin/install -c



