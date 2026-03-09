NAME        = webserv

INCLUDES	= includes/

BUILD_DIR	= .build

CC          = c++
FLAGS		= -I $(INCLUDES)
CFLAGS      = -Wall -Wextra -Werror -std=c++98

SRC         =	srcs/main.cpp \
				srcs/exceptions/exceptions.cpp \
				srcs/Server/Request_gestion/Request.cpp \
				srcs/Server/Request_gestion/Response.cpp \
				srcs/Server/Request_gestion/Session.cpp \
				srcs/Server/WebServer.cpp \
				srcs/Server/config/Redir.cpp \
				srcs/Server/config/Location.cpp \
				srcs/Server/config/Server.cpp \
				srcs/Server/config/ConfigParser.cpp \
				srcs/utils/format.cpp \
				srcs/Server/CGI/Cgi.cpp \
				srcs/utils/wrapper.cpp \
				srcs/utils/safemath.cpp \
				srcs/signal/signal.cpp 

OBJ         = $(SRC:%.cpp=$(BUILD_DIR)/%.o)

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
	@$(CC) $(CFLAGS) $(FLAGS) $(OBJ) -o $(NAME) && \
	echo "   $(GREEN)⤷ $(END)Executable: $(GREEN)$(NAME)$(END)"

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(FLAGS) -c $< -o $@ && \
	printf "   $(GREEN)⤷ $(BROWN)%-25s $(BLUE)→ $(BROWN)%s$(END)\n" "$<" "$@"

c clean:
	@echo "$(GREEN) $(TITLE)🔘 make clean" "$(END)"
	@rm -f $(OBJ) && echo "   $(GREEN)⤷ $(END)Removed object files"

fc fclean: clean
	@echo "$(GREEN) $(TITLE)🔘 make fclean" "$(END)"
	@if [ -f $(NAME) ]; then rm -f $(NAME) && echo "   $(GREEN)⤷ $(END)Removed binary $(NAME)"; fi

re: fclean all

.PHONY: all clean fclean re c fc