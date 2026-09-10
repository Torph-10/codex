NAME        = codexion
CC          = cc
CFLAGS      = -Wall -Wextra -Werror -pthread

SRCS = main.c \
       parsing.c \
       init.c \
       creation.c \
       coder_routine.c \
       monitor.c \
       monitor_utils.c \
       utils.c \
       dongles.c \
       dongles_utils.c \
       heap.c \
       cleanup.c

OBJS        = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re