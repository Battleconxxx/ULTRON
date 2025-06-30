################################################################################
#
# run - AI agent binary with model files
#
################################################################################

RUN_VERSION = 1.0
RUN_SITE = $(TOPDIR)/package/run
RUN_SITE_METHOD = local

define RUN_BUILD_CMDS
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(TARGET_DIR)/usr/bin/run $(RUN_SITE)/run.c -lm
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(TARGET_DIR)/usr/bin/ai_shell $(RUN_SITE)/ai-shell.c 
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(TARGET_DIR)/usr/bin/ai $(RUN_SITE)/ai.c 
endef

define RUN_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/run
	cp $(RUN_SITE)/stories15M.bin $(TARGET_DIR)/usr/share/run/
	cp $(RUN_SITE)/tokenizer.bin $(TARGET_DIR)/usr/share/run/
	cp $(RUN_SITE)/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf $(TARGET_DIR)/usr/share/run/
	cp $(RUN_SITE)/Shell_command.bin $(TARGET_DIR)/usr/share/run/

	# Copy the static llama-run executable
	cp $(RUN_SITE)/llama-run $(TARGET_DIR)/usr/share/run/
endef

$(eval $(generic-package))
