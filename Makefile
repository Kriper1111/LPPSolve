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

SOURCES = $(SOURCES_DIR)/assets.cpp $(SOURCES_DIR)/camera.cpp $(SOURCES_DIR)/LPPShow.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(THIRDPARTY_INCLUDE)/glad.c

OBJS = $(addprefix objects/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))

CXXFLAGS = -I$(INCLUDE_DIR) -I$(THIRDPARTY_INCLUDE)
CXXFLAGS += -I$(IMGUI_DIR) -DDEBUG
LIBS = -lfmt

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
ifeq ($(configuration),debug)
	SOURCES_DEBUG = 
	OBJS += $(addprefix objects/, $(addsuffix .o, $(basename $(notdir $(SOURCES_DEBUG)))))
	
	CXXFLAGS += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
	CXXFLAGS += -DDEBUG -DOBJL_CONSOLE_OUTPUT
	CXXFLAGS += -g -v
endif

############################
# TARGETS
############################
objects/%.o:src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
objects/%.o:thirdparty/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
objects/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
objects/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: object-folder $(EXECUTABLE_NAME)
	@echo "Build done for $(EXECUTABLE_NAME) v$(EXECUTABLE_VERSION)"

object-folder:
	@mkdir -p objects

obj: $(OBJS)

$(EXECUTABLE_NAME): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	-rm $(OBJS)
	-rm $(EXECUTABLE_NAME)
	@echo "All clean!"