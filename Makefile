.phony: all

############################
# BASELINE
############################
EXECUTABLE_NAME = LPPShow
EXECUTABLE_VERSION = $(shell cat version)

SOURCES_DIR = src
INCLUDE_DIR = include
THIRDPARTY_INCLUDE = thirdparty
IMGUI_DIR = $(THIRDPARTY_INCLUDE)/imgui

SOURCES_BASE = $(SOURCES_DIR)/assets.cpp $(SOURCES_DIR)/camera.cpp $(SOURCES_DIR)/LPPShow.cpp $(SOURCES_DIR)/solver.cpp $(SOURCES_DIR)/display.cpp
SOURCES_THIRDPARTY = $(THIRDPARTY_INCLUDE)/quickhull/QuickHull.cpp
SOURCES_THIRDPARTY += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES_THIRDPARTY += $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES_THIRDPARTY += $(THIRDPARTY_INCLUDE)/glad.c

SOURCES = $(SOURCES_BASE) + $(SOURCES_THIRDPARTY)

OBJS_BASE = $(addprefix objects/, $(addsuffix .o, $(basename $(notdir $(SOURCES_BASE)))))
OBJS_THIRDPARTY = $(addprefix objects/, $(addsuffix .o, $(basename $(notdir $(SOURCES_THIRDPARTY)))))

OBJS = $(OBJS_BASE) $(OBJS_THIRDPARTY)

SHADERS = $(wildcard assets/*.vert) $(wildcard assets/*.frag)

CXXFLAGS = -I$(INCLUDE_DIR) -I$(THIRDPARTY_INCLUDE)
CXXFLAGS += -I$(IMGUI_DIR) -DUSE_CDDLIB
LIBS = 

############################
# PLATFORM-SPECIFIC
############################
PLATFORM := $(shell uname -s)
ifeq ($(PLATFORM),Linux)
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3` `pkg-config --static --libs cddlib`
	CXXFLAGS += `pkg-config --cflags glfw3` `pkg-config --cflags cddlib`
endif

ifeq ($(PLATFORM),Darwin) # Borrowed from ImGui's example script
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib -L/opt/homebrew/lib
	LIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS),Windows_NT)
	LIBS += -lglfw3 -lopengl32 -lgdi32
	CXXFLAGS += -static -Llibraries
endif

CFLAGS = $(CXXFLAGS) # I think after?

############################
# CONFIGURATIONS
############################
configuration:=debug
ifeq ($(configuration),debug)
	CXXFLAGS += -DDEBUG -DUSE_OBJ_LOADER -g
endif

ifeq ($(configuration),optimized)
	CXXFLAGS += -DUSE_BAKED_SHADERS -O3
endif

ifeq ($(configuration),cum)
	CXXFLAGS += -g -D_GLIBCXX_DEBUG
endif

############################
# TARGETS
############################
objects/%.o:src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
objects/%.o:tests/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
objects/%.o:thirdparty/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
objects/%.o:thirdparty/quickhull/%.cpp
	$(CXX) -I$(THIRDPARTY_INCLUDE) -c -o $@ $<
objects/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
objects/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

include/bake%.h: $(SHADERS)
	python preconfigure/bake_shaders.py $^ $@

all: object-folder $(EXECUTABLE_NAME)
	@echo "Build done for $(EXECUTABLE_NAME) v$(EXECUTABLE_VERSION)"

run-tests: objects/tests.o objects/solver.o
	$(CXX) -o $@ $^ -g -D_GLIBCXX_DEBUG $(LIBS)

bake: include/baked_shaders.h

object-folder:
	@mkdir -p objects

obj: $(OBJS)

$(EXECUTABLE_NAME): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean-all: clean
	-rm $(OBJS_THIRDPARTY)

clean:
	-rm $(OBJS_BASE)
	-rm $(EXECUTABLE_NAME)
	@echo "All clean!"