BUILDDIR := build
OBJDIR := $(BUILDDIR)/obj
BINDIR := $(BUILDDIR)/bin

SRCS := cpu.c main.c memory.c
OBJS := $(SRCS:%.c=$(OBJDIR)/%.o)

CFLAGS := -Wall -Wextra

$(BINDIR)/nesemu: $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
