NAME        = webserv

INCLUDES	= includes/

BUILD_DIR	= .build

CC          = c++
FLAGS		= -I $(INCLUDES)
CFLAGS      = -Wall -Wextra -Werror -std=c++98

SRC         =	srcs/main.cpp \
				srcs/exceptions/exceptions.cpp \
				srcs/Server/RequestGestion/Request.cpp \
				srcs/Server/RequestGestion/Response.cpp \
				srcs/Server/Upload/UploadHandler.cpp \
				srcs/Server/Connections/ConnectionManager.cpp \
				srcs/Server/Connections/Connection.cpp \
				srcs/Server/RequestGestion/Session.cpp \
				srcs/Server/RequestGestion/SessionManager.cpp \
				srcs/Server/WebServer.cpp \
				srcs/Server/Config/Redir.cpp \
				srcs/Server/Config/Location.cpp \
				srcs/Server/Config/Server.cpp \
				srcs/Server/Config/ConfigParser.cpp \
				srcs/Server/Helpers/Time/TimeoutTracker.cpp \
				srcs/utils/format.cpp \
				srcs/Server/CGI/Cgi.cpp \
				srcs/utils/wrapper.cpp \
				srcs/utils/safemath.cpp \
				srcs/signal/signal.cpp

OBJ         = $(SRC:%.cpp=$(BUILD_DIR)/%.o)

# === Colors ===
GREEN       = $(shell printf '\033[0;32m')
PURPLE      = $(shell printf '\033[0;34m')
GREY        = $(shell printf '\033[3;30m')
TITLE       = $(shell printf '\033[1;33m')
END         = $(shell printf '\033[0m')

# === Rules ===

all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(TITLE)🔘 make $(NAME)" "$(END)"
	@$(CC) $(CFLAGS) $(FLAGS) $(OBJ) -o $(NAME) && \
	echo "   $(GREEN)⤷ $(END)Executable: $(GREEN)$(NAME)$(END)"

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(FLAGS) -c $< -o $@ && \
	printf "   $(GREEN)⤷ $(GREY)%-40s $(PURPLE)→ $(GREY)%s$(END)\n" "$<" "$@"

c clean:
	@echo "$(GREEN) $(TITLE)🔘 make clean" "$(END)"
	@rm -f $(OBJ) && echo "   $(GREEN)⤷ $(END)Removed object files"
	@rm -rf $(BUILD_DIR)

fc fclean: clean
	@echo "$(GREEN) $(TITLE)🔘 make fclean" "$(END)"
	@if [ -f $(NAME) ]; then rm -f $(NAME) && echo "   $(GREEN)⤷ $(END)Removed binary $(NAME)"; fi

re: fclean all

.PHONY: all clean fclean re c fc