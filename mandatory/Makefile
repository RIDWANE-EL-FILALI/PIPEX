NAME = pipex.a
CC = gcc
CFLAGS = -Wall -Wextra -Werror
RM = rm -f
MSRC = children.c error.c free.c ft_split.c ft_strdup.c ft_strjoin.c ft_strncmp.c pipex.c ft_strlen.c 

MOBJ = ${MSRC:.c=.o}

all : $(NAME)

$(NAME) : $(MOBJ) pipex.h
	@ar rc $(NAME) $(MOBJ)
	@${CC} ${CFLAGS} pipex.c ${NAME} -o pipex
clean :
	@${RM} ${MOBJ}
fclean : clean
	@${RM} $(NAME) pipex
re : fclean all