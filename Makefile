.SUFFIXES:
.DEFAULT:
.EXTRA_PREREQS:= $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFLAGS += -r --no-print-directory

BUILDDIR ?= build
CC ?= cc
CFLAGS += -std=gnu99 -Wall -Wextra -Werror -pedantic
LDFLAGS +=

ifeq ($(DEBUG), 1)
	CONFIG = debug
	CFLAGS += -O0 -ggdb3
else ifeq ($(ASAN), 1)
	CONFIG = asan
	CFLAGS += -O0 -ggdb3 -fsanitize=address
	LDFLAGS += -fsanitize=address
else
	CONFIG = release
	CFLAGS += -O3
endif

DEPFLAGS = -MMD -MP
COMPILE.c = $(CC) $(CFLAGS) $(DEPFLAGS) -MT $@ -MF $@.d
OBJDIR ?= $(BUILDDIR)/$(CONFIG)

all: $(OBJDIR)/malincam

$(OBJDIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(COMPILE.c) -c $< -o $@

SOURCES += \
	src/device.c \
	src/main.c \
	src/pollinator.c \
	src/pump.c \
	src/queue.c \
	src/subdev.c \
	src/v4l2-print.c \

OBJS = $(SOURCES:%=$(OBJDIR)/%.o)
DEPS = $(OBJS:%=%.d)
-include $(DEPS)

$(OBJDIR)/malincam: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
