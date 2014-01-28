
export BASE_DIR:=$(shell pwd)

#DIRS= $(BASE_DIR)/libs/audioflinger
DIRS=	\
    	$(BASE_DIR)/libs/libstagefright \
		$(BASE_DIR)/libs/cutils \
		$(BASE_DIR)/libs/utils \
		$(BASE_DIR)/libs/binder \
    	$(BASE_DIR)/libs/tinyalsa \
    	$(BASE_DIR)/libs/audioflinger \
		$(BASE_DIR)/servicemanager \
		$(BASE_DIR)/driver/binder \
		$(BASE_DIR)/driver/ashmem \
		$(BASE_DIR)/libs/speex \
    	$(BASE_DIR)/libs/common_time \
    	$(BASE_DIR)/libs/libhardware \
    	$(BASE_DIR)/libs/libnbaio \
    	$(BASE_DIR)/libs/audio_utils \
    	$(BASE_DIR)/libs/libstagefright/foundation \
    	$(BASE_DIR)/libs/libmedia \
    	$(BASE_DIR)/libs/libeffects \
    	$(BASE_DIR)/mediaserver \
    	$(BASE_DIR)/libs/aud-dec \
    	

TEST_DIRS=$(BASE_DIR)/test/binder_test \
           $(BASE_DIR)/test/player_test \
           $(BASE_DIR)/test/audio_player 

define LOOP_CALL
	@for dir in $(2) ; do \
		if test -d $$dir ; then \
			echo $$dir ; \
			if (cd $$dir; $(MAKE) $(1)) ; then \
				true; \
			else \
				exit 1; \
			fi; \
		fi \
	done
endef

.PHONY: all clean

all:
	$(call LOOP_CALL, all, $(DIRS))
	$(call LOOP_CALL, all, $(TEST_DIRS))

clean:
	$(call LOOP_CALL, clean, $(DIRS))
	$(call LOOP_CALL, clean, $(TEST_DIRS))

