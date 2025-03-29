NAME		  = bombkurdistan

INC_DIR		= inc
LIB_DIR		= lib
SRC_DIR		= src
BUILD_DIR	= build

RAYLIB_SRC	= $(LIB_DIR)/raylib/src
RAYLIB		  = $(RAYLIB_SRC)/libraylib.a

CXX			  = clang++
CXXFLAGS	= -Wall -Wextra -O0 -g3 -ggdb3 -std=c++23 -MMD -MP \
			-I$(INC_DIR) -I$(RAYLIB_SRC)
LDFLAGS		= -L$(RAYLIB_SRC) -lraylib
RM			  = rm -rf

SRCS		= $(wildcard $(SRC_DIR)/*.cpp)
OBJS		= $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))
DEPS		= $(OBJS:.o=.d)

all: $(NAME)

$(RAYLIB):
	@$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_DESKTOP

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJS) $(RAYLIB)
	@$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $@

clean:
	@$(RM) $(BUILD_DIR)

fclean: clean
	@$(MAKE) -C $(RAYLIB_SRC) clean
	@$(RM) $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re
