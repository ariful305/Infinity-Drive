# macOS: frameworks. Linux: install freeglut, then: make CC=cc LDLIBS='-lglut -lGLU -lGL -lm'
CC ?= cc
CFLAGS ?= -Wall -Wextra -O2 -std=c11

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  CFLAGS += -DGL_SILENCE_DEPRECATION=1
  # Cocoa is required for GLUT windowing on macOS (fixes arm64 undefined symbols)
  LDFLAGS ?= -framework OpenGL -framework GLUT -framework Cocoa
else
  LDFLAGS ?= -lglut -lGLU -lGL -lm
endif

OBJS = globals.o primitives.o scene1.o scene2.o scene3.o scene4.o scene5.o main_app.o
TARGET = infinite_drive

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c drive.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
