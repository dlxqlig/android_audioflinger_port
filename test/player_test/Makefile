include $(BASE_DIR)/common.mk

SRCS := \
        player_test.cpp

OBJS = $(patsubst %.cpp,%.o, $(SRCS))

TARGET = $(BASE_DIR)/player_test


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
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) -lpthread -lbinder -lutils -lcutils -lmedia -laudio_utils -lspeexresampler -lstagefright_foundation
	@echo "Successfully built $@"

print:
	@echo "****************************************************"
	@echo "*** Start building player test                   ***"
	@echo "****************************************************"

clean:
	@echo Cleaning up previous object files
	@rm -f $(OBJS) $(TARGET)

