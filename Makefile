CC=gcc -std=c2x
CFLAGS=-fPIC -shared -Wall -Wextra -Wpedantic
CFLAGS+=-Iinclude
LDFLAGS=
NAME=libscieppend.so

SRCS=$(wildcard src/core/*.c)
OBJS=$(SRCS:.c=.o)
DEPS=$(SRCS:.c=.d)

.PHONY: clean default fullclean debug release

default:
	@echo "Specify a target. Options: debug, release"

debug: CFLAGS += -g -DDEBUG
debug: release

release: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

%.o: %.cpp
	$(CC) -MMD $(CPPFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

fullclean: clean
	@rm -f $(NAME)

-include $(DEPS)
