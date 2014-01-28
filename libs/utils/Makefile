include $(BASE_DIR)/common.mk

SRCS := \
	CallStack.cpp \
	Debug.cpp \
	Flattenable.cpp \
	misc.cpp \
	RefBase.cpp \
	SharedBuffer.cpp \
	String16.cpp \
	String8.cpp \
	SystemClock.cpp \
	TextOutput.cpp \
	Timers.cpp \
	Unicode.cpp \
	VectorImpl.cpp \
	BufferedTextOutput.cpp \
	Threads.cpp \
	Static.cpp \
	Log.cpp \
    LinearTransform.cpp \
    StringArray.cpp \
    Trace.cpp

OBJS = $(patsubst %.cpp,%.o, $(SRCS))

TARGET = $(BASE_DIR)/libutils.so

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
	@echo "*** Start building utils                         ***"
	@echo "****************************************************"

clean:
	@echo Cleaning up previous object files
	@rm -f $(TARGET) $(OBJS)

