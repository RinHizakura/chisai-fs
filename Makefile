CFLAGS = -Wall -Wextra -Werror
CFLAGS += -DFUSE_USE_VERSION=26 -D_FILE_OFFSET_BITS=64 -D_XOPEN_SOURCE=700 -Iinclude
LDFLAGS = -lfuse

CURDIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
OUT ?= build
CHISAI = $(OUT)/chisai
SHELL_HACK := $(shell mkdir -p $(OUT))

GIT_HOOKS := .git/hooks/applied

CSRCS = $(shell find ./src -name '*.c')
CSRCS += $(shell find ./utils -name '*.c')

_COBJ =  $(notdir $(CSRCS))
COBJ = $(_COBJ:%.c=$(OUT)/%.o)

vpath %.c $(sort $(dir $(CSRCS)))

all: $(GIT_HOOKS) $(CHISAI)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

$(OUT)/%.o: %.c
	@echo "  CC\t$@"
	@$(CC) -c $(CFLAGS) $< -o $@

$(CHISAI): $(COBJ)
	@echo "  LD\t$@"
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

check: $(CHISAI)
	./scripts/format-image.sh
	./scripts/mount_image.sh
clean:
	@$(RM) $(COBJ)
	@$(RM) $(CHISAI)
