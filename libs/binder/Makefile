include $(BASE_DIR)/common.mk

SRCS := \
    Binder.cpp \
    BpBinder.cpp \
    IInterface.cpp \
    IPCThreadState.cpp \
    IPermissionController.cpp \
    IServiceManager.cpp \
    Parcel.cpp \
    PermissionCache.cpp \
    ProcessState.cpp \
    Static.cpp \
    IMemory.cpp \
    MemoryDealer.cpp \
    MemoryBase.cpp \
    MemoryHeapBase.cpp \
    MemoryHeapPmem.cpp

OBJS += $(patsubst %.cpp,%.o, $(SRCS))

TARGET = $(BASE_DIR)/libbinder.so

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
	@echo "*** Start building binder                        ***"
	@echo "****************************************************"

clean:
	@echo Cleaning up previous object files
	@rm -f $(TARGET) $(OBJS)

