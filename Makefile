CC = clang
CFLAGS = -Wall -Wextra -Werror=return-type
LIBS = -lm
READLINE ?= auto # auto | 1 | 0

ifeq ($(strip $(READLINE)),auto)
	ifeq ($(OS),Windows_NT)
	else
		ifeq ($(shell uname -s),Linux)
			CFLAGS += `pkg-config --cflags readline` -DUSE_READLINE
			LIBS += `pkg-config --libs readline`
		endif
	endif
else ifeq ($(strip $(READLINE)),1)
	CFLAGS += `pkg-config --cflags readline` -DUSE_READLINE
	LIBS += `pkg-config --libs readline`
endif

ifeq ($(r),1)
	CFLAGS += -O3 -DNDEBUG
else
	CFLAGS += -O0 -ggdb
endif

ifeq ($(san),1)
	CFLAGS += -fsanitize=undefined,address
endif

main: main.c
	$(CC) $(CFLAGS) main.c -o main $(LIBS)
