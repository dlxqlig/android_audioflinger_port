include $(BASE_DIR)/common.mk

SRCS := \
	atomic.c \
	native_handle.c \
	process_name.c \
	threads.c \
    config_utils.c \
    ashmem-host.c \
    load_file.c \
    misc.c \
    str_parms.c \
    memory.c \
    hashmap.c \
    sched_policy.c

OBJS = $(patsubst %.c, %.o, $(SRCS))

TARGET = $(BASE_DIR)/libcutils.so

.PHONY: all clean $(TARGET)

all: print $(TARGET)

%.o : %.cpp
	@echo Compiling $<
	@$(CXX) $(CFLAGS) -c -o $@ $<

%.o : %.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS) 
	@echo Building executable files for $(TARGET)
	$(CXX) $(CFLAGS) $(LDFLAGS) -shared -fpic -o $(TARGET) $(OBJS)
	@echo "Successfully built $@"

print:
	@echo "****************************************************"
	@echo "*** Start building cutils                        ***"
	@echo "****************************************************"

clean:
	@echo Cleaning up previous object files
	@rm -f $(TARGET) $(OBJS)

