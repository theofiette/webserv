NAME        = webserv

CC          = c++
CFLAGS      = -Wall -Wextra -Werror -std=c++98

SRC         = srcs/main.cpp
OBJ         = $(SRC:.cpp=.o)

# === Colors ===
GREEN       = \033[0;32m
BLUE        = \033[0;34m
BROWN       = \033[0;33m
END         = \033[0m
TITLE       = \033[1;36m

# === Rules ===

all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(GREEN) $(TITLE)🔘 make $(NAME)" "$(END)"
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME) > /dev/null && \
	echo "   $(GREEN)⤷ $(END)Executable: $(GREEN)$(NAME)$(END)"

%.o: %.cpp
	@$(CC) $(CFLAGS) -c $< -o $@ > /dev/null && \
	printf "   $(GREEN)⤷ $(BROWN)%-25s $(BLUE)→ $(BROWN)%s$(END)\n" "$<" "$@"

c clean:
	@echo "$(GREEN) $(TITLE)🔘 make clean" "$(END)"
	@rm -f $(OBJ) && echo "   $(GREEN)⤷ $(END)Removed object files"

fc fclean: clean
	@echo "$(GREEN) $(TITLE)🔘 make fclean" "$(END)"
	@if [ -f $(NAME) ]; then rm -f $(NAME) && echo "   $(GREEN)⤷ $(END)Removed binary $(NAME)"; fi

re: fclean all

.PHONY: all clean fclean re c fc