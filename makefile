ifeq ($(OS), Windows_NT)
	RM = del /Q /S
	MKDIR = if not exist $(TARGETDIR) mkdir
	FIX_PATH = $(subst /,\,$1)
	TARGET_EXEC = $(TARGET).exe
	LDFLAGS = -lws2_32
endif


CC = gcc
CFLAGS = -Wall -Werror -Iheaders -MMD -MP
TARGETDIR = target
TARGET = Tacklebox
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=$(TARGETDIR)/%.o)
DEPS = $(OBJS:.o=.d)

all : $(TARGET_EXEC)

-include $(DEPS)

run : all
	./$(TARGETDIR)/$(TARGET)

$(TARGET_EXEC) : $(OBJS)
	$(CC) $(OBJS) -o $(TARGETDIR)/$@ $(LDFLAGS)

$(TARGETDIR)/%.o : src/%.c
	@$(MKDIR)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	$(RM) $(call FIX_PATH,$(TARGETDIR)/*)

.PHONY : all run clean