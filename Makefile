CC=gcc -std=c2x
CFLAGS=-fPIC -shared -Wall -Wextra -Wpedantic
CFLAGS+=-Iinclude
LDFLAGS=
NAME=libscieppend.so

SRCS=$(wildcard src/core/*.c)
OBJS=$(SRCS:.c=.o)
DEPS=$(SRCS:.c=.d)

.PHONY: clean default fullclean debug release

default: release

debug: CFLAGS+=-g -DDEBUG
debug: $(NAME)

release: CFLAGS+=-Ofast
release: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

%.o: %.c
	$(CC) -MMD $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

fullclean: clean
	@rm -f $(NAME)

-include $(DEPS)
