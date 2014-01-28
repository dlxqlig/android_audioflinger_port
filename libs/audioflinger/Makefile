include $(BASE_DIR)/common.mk

SRCS := \
    ISchedulingPolicyService.cpp \
    SchedulingPolicyService.cpp \
    AudioFlinger.cpp            \
    Threads.cpp                 \
    Tracks.cpp                  \
    Effects.cpp                 \
    AudioMixer.cpp              \
    AudioResampler.cpp          \
    AudioPolicyService.cpp      \
    ServiceUtilities.cpp        \
    AudioResamplerCubic.cpp     \
    AudioResamplerSinc.cpp     \
    StateQueue.cpp             \
    FastMixer.cpp              \
    FastMixerState.cpp         \
    AudioOutputManager.cpp


OBJS += $(patsubst %.cpp,%.o, $(SRCS))

TARGET = $(BASE_DIR)/libaudioflinger.so

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
	#$(CXX) $(CFLAGS) $(LDFLAGS) -shared -fpic -o $(TARGET) $(OBJS) -lcutils -lutils -laudio_utils \
	#    -lcommon_time_client -lmedia -lnbaio -lstagefright_foundation -lhardware -lbinder -lpthread \
	#    -lspeexresampler -leffect -lrt -ldl
	@echo "Successfully built $@"

print:
	@echo "****************************************************"
	@echo "*** Start building binder                        ***"
	@echo "****************************************************"

clean:
	@echo Cleaning up previous object files
	@rm -f $(TARGET) $(OBJS)

