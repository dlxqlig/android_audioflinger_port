include $(BASE_DIR)/common.mk

SRCS := \
    OMX.cpp                       \
    OMXMaster.cpp                 \
    OMXNodeInstance.cpp           \
    SimpleSoftOMXComponent.cpp    \
    SoftOMXComponent.cpp          \
    SoftOMXPlugin.cpp


OBJS += $(patsubst %.cpp,%.o, $(SRCS))

TARGET = $(BASE_DIR)/libstagefright_omx.so

CFLAGS += -I$(BASE_DIR)/include/stagefright/foundation
CFLAGS += -I$(BASE_DIR)/include/media/hardware
CFLAGS += -I$(BASE_DIR)/include/media/openmax

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

