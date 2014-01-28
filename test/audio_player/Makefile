include $(BASE_DIR)/common.mk

PCM_SRCS := \
        audio_player_pcm.cpp

DECODER_SRCS := \
        audio_player_decoder.cpp

PCM_OBJS = $(patsubst %.cpp,%.o, $(PCM_SRCS))
DECODER_OBJS = $(patsubst %.cpp,%.o, $(DECODER_SRCS))

PCM_TARGET = $(BASE_DIR)/audio_player_pcm
DECODER_TARGET = $(BASE_DIR)/audio_player_decoder

.PHONY: all clean $(PCM_TARGET) $(DECODER_TARGET)

all: print $(PCM_TARGET) $(DECODER_TARGET)

%.o : %.cpp
	@echo Compiling $<
	@$(CXX) $(CFLAGS) -c -o $@ $<

%.o : %.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(PCM_TARGET): $(PCM_OBJS) 
	@echo Building executable files for $(PCM_TARGET)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $(PCM_TARGET) $(PCM_OBJS) -lpthread -ldl -lrt -laudioplayer -lmedia -lbinder -lutils -lnbaio -lstagefright_foundation -lcommon_time_client -lcutils -lhardware -laudio_utils -lspeexresampler
	@echo "Successfully built $@"

$(DECODER_TARGET): $(DECODER_OBJS) 
	@echo Building executable files for $(DECODER_TARGET)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $(DECODER_TARGET) $(DECODER_OBJS) -laudUtils -laudDec -laudioplayer -lmedia -lbinder -lutils -lnbaio -lstagefright_foundation -lcommon_time_client -lcutils -lhardware -laudio_utils -lspeexresampler
	@echo "Successfully built $@"

print:
	@echo "****************************************************"
	@echo "*** Start building audio player test             ***"
	@echo "****************************************************"

clean:
	@echo Cleaning up previous object files
	@rm -f $(PCM_OBJS) $(PCM_TARGET) $(DECODER_OBJS) $(DECODER_TARGET)

