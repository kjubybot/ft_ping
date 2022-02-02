NAME = ft_ping

CC = gcc
CFLAGS = -Iinclude/ -MMD

RM = rm -fr

SRCDIR = src/
OBJDIR = obj/

OBJ = \
	  main.o\
	  recv.o\
	  opts.o\
	  ip.o

OBJ := $(addprefix $(OBJDIR),$(OBJ))

NB = $(words $(OBJ))
DONE = 0

all: $(NAME)

$(NAME): $(OBJDIR) $(OBJ)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJ)
	@printf "\r\033[38;5;46mDONE\033[0m\033[K\n"

$(OBJDIR)%.o: $(SRCDIR)%.c
	@$(CC) $(CFLAGS) -c -o $@ $<
	@$(eval DONE=$(shell expr $(DONE) + 1))
	@printf "\r\033[38;5;178mCOMPILING %d/%d\033[0m" $(DONE) $(NB)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	@$(RM) $(OBJDIR)
	@printf "\033[38;5;124mCLEAN\033[0m\n"

fclean: clean
	@$(RM) $(NAME)
	@printf "\033[38;5;88mFCLEAN\033[0m\n"

re: fclean all

.PHONY: all clean fclean re

-include obj/*.d
