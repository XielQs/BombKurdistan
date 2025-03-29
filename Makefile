CXX = clang++
CXXFLAGS = -Wall -Wextra -O0 -g3 -ggdb3 -std=c++23 -MMD -MP
INCLUDES = -Iexternal/raylib/src
LDFLAGS = -Lexternal/raylib/build -lraylib

SRCDIR = .
BUILDDIR = build
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))
DEP = $(OBJ:.o=.d)
EXEC = $(BUILDDIR)/bombkurdistan

all: raylib $(EXEC)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(EXEC): $(OBJ) external/raylib/build/raylib/libraylib.a
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

-include $(DEP)

external/raylib/build/raylib/libraylib.a:
	mkdir -p external/raylib/build
	cd external/raylib/build && \
	cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release && \
	make

raylib: external/raylib/build/raylib/libraylib.a

clean:
	rm -rf $(BUILDDIR) external/raylib/build

.PHONY: all clean raylib
