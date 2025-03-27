CXX = clang++
CXXFLAGS = -Wall -Wextra -O0 -g3 -ggdb3 -std=c++23 -MMD -MP
INCLUDES = -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lraylib

SRCDIR = .
BUILDDIR = build
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))
DEP = $(OBJ:.o=.d)
EXEC = $(BUILDDIR)/bombkurdistan

all: $(EXEC)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

-include $(DEP)

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
