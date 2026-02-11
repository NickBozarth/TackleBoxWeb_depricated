CC = gcc
CFLAGS = -Wall -Iheaders
LDFLAGS = -lws2_32 # definitely wont work on linux
TARGETDIR = target
TARGET = Tacklebox
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=$(TARGETDIR)/%.o)

all : $(TARGET)

run : $(TARGET)
	./$(TARGETDIR)/$(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGETDIR)/$@

$(TARGETDIR)/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY : all