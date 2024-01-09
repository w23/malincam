.SUFFIXES:
.DEFAULT:
.EXTRA_PREREQS:= $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFLAGS += -r --no-print-directory

BUILDDIR ?= build
CC ?= cc
CFLAGS += -std=gnu99 -Wall -Wextra -Werror -pedantic

ifeq ($(DEBUG), 1)
	CONFIG = debug
	CFLAGS += -O0 -ggdb3
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
	main.c 

OBJS = $(SOURCES:%=$(OBJDIR)/%.o)
DEPS = $(OBJS:%=%.d)
-include $(DEPS)

$(OBJDIR)/malincam: $(OBJS)
	$(CC) $^ -o $@

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
