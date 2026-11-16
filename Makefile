CC = clang
CFLAGS = -Wall -Wextra -Werror=return-type

ifeq ($(r),1)
	CFLAGS += -O3 -DNDEBUG
else
	CFLAGS += -O0 -ggdb
endif

ifeq ($(san),1)
	CFLAGS += -fsanitize=undefined,address
endif

main: main.c
	$(CC) $(CFLAGS) main.c -o main -lm
