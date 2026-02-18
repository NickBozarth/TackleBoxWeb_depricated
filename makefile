ifeq ($(OS), Windows_NT)
	RM = del /Q /S
	MKDIR = if not exist $(TARGETDIR) mkdir
	FIX_PATH = $(subst /,\,$1)
	TARGET_EXEC = $(TARGET).exe
	LDFLAGS = -lws2_32
else
	RM = rm -rf
	MKDIR = mkdir -p $(TARGETDIR)
	FIX_PATH = $1
	TARGET_EXEC = $(TARGET)
LDFLAGS = 
endif


CC = gcc
CFLAGS = -Wall -Iheaders -MMD -MP
TARGETDIR = target
TARGET = Tacklebox
SRCS_ALL = $(wildcard src/*.c)
BLACKLIST_FILES = src/http.c 
SRCS = $(filter-out $(BLACKLIST_FILES), $(SRCS_ALL))
OBJS = $(SRCS:src/%.c=$(TARGETDIR)/%.o)
DEPS = $(OBJS:.o=.d)

-include $(DEPS)

all : $(TARGET_EXEC)

run : all
	./$(TARGETDIR)/$(TARGET)


$(TARGET_EXEC) : $(OBJS)
	$(CC) $(OBJS) -o $(TARGETDIR)/$@ $(LDFLAGS)

$(TARGETDIR)/%.o : src/%.c
	@$(MKDIR)
	$(CC) $(CFLAGS) -c $< -o $@

bear:
	bear -- make clean all

clean:
	$(RM) $(call FIX_PATH,$(TARGETDIR)/*)

.PHONY : all bear run clean
