CC=gcc -std=c2x
CFLAGS=-Wall -Wextra -Wpedantic
CFLAGS+=-Iinclude
LDFLAGS=-lm
NAME=libscieppend.so

SRC_DIRS=src/core src/core/concurrent
SRCS=$(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
OBJS=$(SRCS:.c=.o)
DEPS=$(SRCS:.c=.d)

TEST_SRCS=$(shell find src/test/ -type f -name *.c)
TEST_OBJS=$(TEST_SRCS:.c=.o)
TEST_DEPS=$(TEST_SRCS:.c=.d)

.PHONY: default clean fullclean debug release test test-debug

default: release

debug: CFLAGS+=-g -DDEBUG
debug: $(NAME)

release: CFLAGS+=-Ofast
release: $(NAME)

test-debug: CFLAGS+=-g -DDEBUG
test-debug: test

test: NAME=scieppend-test
test: $(TEST_OBJS) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(TEST_OBJS) -o $(NAME) $(LDFLAGS)

$(NAME): CFLAGS+=-fPIC -shared
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

%.o: %.c
	$(CC) -MMD $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)
	@rm -f $(TEST_OBJS)
	@rm -f $(TEST_DEPS)

fullclean: clean
	@rm -f $(NAME)
	@rm -f scieppend-test

-include $(DEPS)
