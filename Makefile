NAME		  = bombkurdistan
NAME_WIN	= bombkurdistan.exe
NAME_WEB  = bombkurdistan.html

ZIP_NAME     = bombkurdistan_linux.zip
ZIP_NAME_WIN = bombkurdistan_windows.zip

INC_DIR		= inc
LIB_DIR		= lib
SRC_DIR		= src
BUILD_DIR	= build

RAYLIB_SRC	= $(LIB_DIR)/raylib/src
RAYLIB		  = $(RAYLIB_SRC)/libraylib.a
RAYLIB_WIN  = $(RAYLIB_SRC)/libraylib.win.a
RAYLIB_WEB    = $(RAYLIB_SRC)/libraylib.web.a

DISCORDRPC_SRC = $(LIB_DIR)/discordrpc
DISCORDRPC	  = $(DISCORDRPC_SRC)/libdiscordrpc.a

EMSCRIPTEN    = emcc
CXX			  = clang++
CXXFLAGS	= -Wall -Wextra -g3 -ggdb3 -std=c++23 -MMD -MP \
			-I$(INC_DIR) -I$(RAYLIB_SRC)
LDFLAGS		= -L$(RAYLIB_SRC)
RM			  = rm -rf


ifeq ($(MAKECMDGOALS), windows)
	CXX = x86_64-w64-mingw32-g++
	CXXFLAGS = -O3
	LDFLAGS += -lraylib.win -static -lgdi32 -lwinmm
	BUILD_SUBDIR = $(BUILD_DIR)/win
else ifeq ($(MAKECMDGOALS), web)
	CXX         = $(EMSCRIPTEN)
	LDFLAGS    += -lraylib.web --shell-file assets/shell.html
	BUILD_SUBDIR = $(BUILD_DIR)/web
	CXXFLAGS   += -Os --preload-file assets -s USE_GLFW=3 -s EXPORTED_RUNTIME_METHODS=ccall -DPLATFORM_WEB
else
# include discordrpc if it's on linux
	LDFLAGS += -lraylib -L$(DISCORDRPC_SRC)/build -ldiscordrpc
	CXXFLAGS += -I$(DISCORDRPC_SRC)/inc -O3
	BUILD_SUBDIR = $(BUILD_DIR)/linux
endif

SRCS		= $(wildcard $(SRC_DIR)/*.cpp)
OBJS		= $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_SUBDIR)/%.o, $(SRCS))
DEPS		= $(OBJS:.o=.d)

all: $(NAME)
windows: $(NAME_WIN)
web: $(NAME_WEB)

$(RAYLIB):
	@$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_DESKTOP \
		CUSTOMIZE_BUILD=ON \
		SUPPORT_GIF_RECORDING=0 SUPPORT_SCREEN_CAPTURE=0 \
		SUPPORT_CAMERA_SYSTEM=0 SUPPORT_GESTURES_SYSTEM=0 \
		SUPPORT_MOUSE_GESTURES=0 SUPPORT_VR_SIMULATOR=0 \
		SUPPORT_DISTORTION_SHADER=0 SUPPORT_IMAGE_GENERATION=0 \

$(RAYLIB_WEB):
	@$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_WEB -B

$(DISCORDRPC):
	@echo "Building discordrpc..."
	@$(MAKE) -C $(DISCORDRPC_SRC) lib

$(BUILD_SUBDIR):
	@mkdir -p $@

$(BUILD_SUBDIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_SUBDIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJS) $(RAYLIB) $(DISCORDRPC)
	@$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $@

$(NAME_WIN): $(OBJS) $(RAYLIB_WIN)
	@$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $@

$(NAME_WEB): $(OBJS) $(RAYLIB_WEB)
	@$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $(BUILD_SUBDIR)/$@

$(RAYLIB_WIN):
	@echo "Downloading raylib..."
	@$(RM) -rf raylib.zip raylib-5.5_win64_mingw-w64 $(RAYLIB_WIN)
	@wget -q --show-progress https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip -O raylib.zip
	@unzip -q raylib.zip
	@mv raylib-5.5_win64_mingw-w64/lib/libraylib.a $(RAYLIB_WIN)
	@$(RM) -r raylib.zip raylib-5.5_win64_mingw-w64
	@echo "Raylib downloaded and moved"

zip:
	@if [ ! -f "$(NAME)" ] && [ ! -f "$(NAME_WIN)" ]; then \
		echo "Error: Neither Linux nor Windows executable found!"; \
		exit 1; \
	fi
	@if [ -f "$(NAME)" ]; then \
		@$(RM) -f $(ZIP_NAME); \
		echo "Creating $(ZIP_NAME)..."; \
		zip -r $(ZIP_NAME) assets $(NAME) 2>/dev/null || true; \
		echo "Zip archive created: $(ZIP_NAME)"; \
	fi
	@if [ -f "$(NAME_WIN)" ]; then \
		$(RM) -f $(ZIP_NAME_WIN) \
		echo "Creating $(ZIP_NAME_WIN)..."; \
		zip -r $(ZIP_NAME_WIN) assets $(NAME_WIN) 2>/dev/null || true; \
		echo "Zip archive created: $(ZIP_NAME_WIN)"; \
	fi

clean:
	@$(RM) $(BUILD_DIR)

fclean: clean
	@$(MAKE) -C $(RAYLIB_SRC) clean
	@$(MAKE) -C $(DISCORDRPC_SRC) clean
	@$(RM) $(NAME) $(NAME_WIN) $(NAME_WEB) $(RAYLIB) $(RAYLIB_WIN) $(ZIP_NAME) $(ZIP_NAME_WIN)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re windows zip
