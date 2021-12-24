NAME = ft_ping

CC = gcc
CFLAGS = -Iinclude/ -g

RM = rm -fr

SRCDIR = src/
OBJDIR = obj/

OBJ = \
	  main.o\
	  recv.o

OBJ := $(addprefix $(OBJDIR),$(OBJ))

all: $(NAME)

$(NAME): $(OBJDIR) $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

$(OBJDIR)%.o: $(SRCDIR)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	$(RM) $(OBJDIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
