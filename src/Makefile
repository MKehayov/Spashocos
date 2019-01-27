NAME=spashocos

all: $(NAME)

$(NAME): $(NAME).c utils.c
		gcc -Wall -Werror -std=gnu11 -o $(NAME) $(NAME).c utils.c -lncurses -pthread
		
run:
	./$(NAME)
		
clean:
	rm -rf $(NAME) $(NAME)
