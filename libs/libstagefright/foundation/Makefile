include $(BASE_DIR)/common.mk

SRCS := \
    AAtomizer.cpp                 \
    ABitReader.cpp                \
    ABuffer.cpp                   \
    AHandler.cpp                  \
    AHierarchicalStateMachine.cpp \
    ALooper.cpp                   \
    ALooperRoster.cpp             \
    AMessage.cpp                  \
    AString.cpp                   \
    base64.cpp                    \
    hexdump.cpp

OBJS += $(patsubst %.cpp,%.o, $(SRCS))

TARGET = $(BASE_DIR)/libstagefright_foundation.so

CFLAGS += -I$(BASE_DIR)/include/stagefright/foundation

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

