include $(BASE_DIR)/common.mk

SRCS_BINDER_C := \
        ITestBinderService.cpp \
        testBinder.cpp

SRCS_BINDER_S := \
        ITestBinderService.cpp \
        TestBinderService.cpp \
        testMain.cpp

OBJS_C = $(patsubst %.cpp,%.o, $(SRCS_BINDER_C))
OBJS_S = $(patsubst %.cpp,%.o, $(SRCS_BINDER_S))

BINDER_C = $(BASE_DIR)/testBinderClient
BINDER_S = $(BASE_DIR)/testBinderService

.PHONY: all clean $(BINDER_C) $(BINDER_S)

all: print $(BINDER_C) $(BINDER_S)

%.o : %.cpp
	@echo Compiling $<
	@$(CXX) $(CFLAGS) -c -o $@ $<

%.o : %.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BINDER_C): $(OBJS_C) 
	@echo Building executable files for $(BINDER_C)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $(BINDER_C) $(OBJS_C) -lpthread -lbinder -lutils -lcutils
	@echo "Successfully built $@"

$(BINDER_S): $(OBJS_S) 
	@echo Building executable files for $(BINDER_S)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $(BINDER_S) $(OBJS_S) -lpthread -lbinder -lutils -lcutils
	@echo "Successfully built $@"

print:
	@echo "****************************************************"
	@echo "*** Start building binder test                   ***"
	@echo "****************************************************"

clean:
	@echo Cleaning up previous object files
	@rm -f $(OBJS_C) $(OBJS_S) $(BINDER_C) $(BINDER_S)

