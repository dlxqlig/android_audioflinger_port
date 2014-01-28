include $(BASE_DIR)/common.mk

SRCS := \
    AudioBufferProviderSource.cpp   \
    AudioStreamOutSink.cpp          \
    AudioStreamInSource.cpp         \
    NBAIO.cpp                       \
    NBLog.cpp                       \
    MonoPipe.cpp                    \
    MonoPipeReader.cpp              \
    Pipe.cpp                        \
    PipeReader.cpp                  \
    roundup.cpp                     \
    SourceAudioBufferProvider.cpp

OBJS = $(patsubst %.cpp,%.o, $(SRCS))

TARGET = $(BASE_DIR)/libnbaio.so

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
	@echo "*** Start building speex resampler               ***"
	@echo "****************************************************"

clean:
	@echo Cleaning up previous object files
	@rm -f $(TARGET) $(OBJS)

