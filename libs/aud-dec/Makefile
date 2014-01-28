include $(BASE_DIR)/common.mk

CMN_SRC_DIR = $(BASE_DIR)/libs/aud-dec/aud-cmn/src
THREAD_DIR = $(CMN_SRC_DIR)/threads
UTILS_DIR = $(CMN_SRC_DIR)/utils
DEC_SRC_DIR = $(BASE_DIR)/libs/aud-dec/aud-dec/src
OMX_DEC_SRC_DIR = $(BASE_DIR)/libs/aud-dec/aud-omx-dec/src

CFLAGS += ${CPP_OPTS} ${CPP_DEFS}

THREADS_SRCS = 	$(THREAD_DIR)/Thread.cpp 

UTILS_SRCS = 	$(UTILS_DIR)/Logger.cpp \
				$(UTILS_DIR)/AudBuffer.cpp 

DEC_SRCS = 	$(DEC_SRC_DIR)/OMXAudio.cpp \
				$(DEC_SRC_DIR)/OMXCore.cpp \
				$(DEC_SRC_DIR)/OMXProcess.cpp

OMX_DEC_SRCS = 	$(OMX_DEC_SRC_DIR)/library_entry_point.c \
				$(OMX_DEC_SRC_DIR)/omx_audiodec_component.c

THREADS_OBJS = $(patsubst %.cpp, %.o, $(THREADS_SRCS))
UTILS_OBJS = $(patsubst %.cpp, %.o, $(UTILS_SRCS))

DEC_OBJS = $(patsubst %.cpp, %.o, $(DEC_SRCS))

OMX_DEC_OBJS = $(patsubst %.c, %.o, $(OMX_DEC_SRCS))

UTILS = $(BASE_DIR)/libaudUtils.so
DEC = $(BASE_DIR)/libaudDec.so
OMX_DEC = $(BASE_DIR)/libaudOmxDec.so

.PHONY: all install install_dev install_target clean print
all: print $(UTILS) $(DEC) $(OMX_DEC)

print:
	@echo "****************************************************"
	@echo "*** Start building audio utils                   ***"
	@echo "****************************************************"

%.o : %.cpp
	@echo Compiling $<
	@$(CXX) $(CFLAGS) -c -o $@ $<

%.o : %.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(UTILS): $(THREADS_OBJS) $(UTILS_OBJS) 
	@echo Building executable files for $(UTILS)
	$(CXX) $(CFLAGS) $(LDFLAGS) -shared -fpic -o $(UTILS) $(THREADS_OBJS) $(UTILS_OBJS)
	@echo "Successfully built $@"


$(DEC): $(DEC_OBJS) $(UTILS)
	@echo Building executable files for $(DEC)
	$(CXX) $(CFLAGS) $(LDFLAGS) -shared -fpic -o $(DEC) $(DEC_OBJS) -laudUtils -lavutil -lavcodec -lavformat -lswscale -lrt -lomxil-bellagio -lpthread
	@echo "Successfully built $@"

#$(DEC_TEST): $(DEC_TEST_OBJS) $(DEC)
	#@echo Building executable files for $(DEC_TEST)
	#$(CXX) $(CFLAGS) $(LDFLAGS) -o $(DEC_TEST) $(DEC_TEST_OBJS) -laudUtils -laudRender -laudDec

$(OMX_DEC): $(OMX_DEC_OBJS)
	@echo Building executable files for $(OMX_DEC)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fpic -o $(OMX_DEC) $(OMX_DEC_OBJS) -lavcodec -lavformat -lavutil -lswscale -lswresample
	@echo "Successfully built $@"

install: install_dev install_target

install_dev: 

install_target:
	@echo "****************************************************"
	@echo "*** Start installing audio utils                 ***"
	@echo "****************************************************"


	@echo "Installation done."

install_local:
	@echo "****************************************************"
	@echo "*** Start installing audio render               ***"
	@echo "****************************************************"

clean: 
	@echo Cleaning up previous object files
	@rm -f $(THREADS_OBJS) $(UTILS_OBJS) $(UTILS) $(DEC) $(DEC_OBJS) $(OMX_DEC) $(OMX_DEC_OBJS)
