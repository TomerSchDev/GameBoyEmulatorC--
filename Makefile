CXX = g++
CC = gcc
CXXFLAGS = -Wall -std=c++17
CFLAGS = -Wall
DEBUG_CXXFLAGS = $(CXXFLAGS) -g
DEBUG_CFLAGS = $(CFLAGS) -g
#all the include directories
SDL_INCLUDE = -I./SDL/include
EMU_INCLUDE = -I./emulator/include
TTF_INCLUDE = -I./SDL_TTF/include
CPU_INST_INCLUDE = -I./emulator/include/cpu_instructions/
INCLUDES = $(SDL_INCLUDE) $(EMU_INCLUDE) $(TTF_INCLUDE) $(CPU_INST_INCLUDE)

# Add the path to the SDL2_ttf library
# Note: You may need to adjust the path to the SDL2_ttf library
SDL_TTF_DIR = -L./SDL_TTF/lib -lSDL3_ttf
SDL_LIB = -L./SDL/lib -lSDL3
LIBS = $(SDL_LIB) $(SDL_TTF_DIR)



# Windows/MSYS2 specific
UNAME := $(shell uname)
ifeq ($(UNAME),)
	# Native Windows
	TARGET = game.exe
	DEBUG_TARGET = game_debug.exe
	RM = del /Q
	MKDIR = if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	CP = copy /Y
	SEP = \\
else
	# MSYS2/MinGW environment
	TARGET = game.exe
	DEBUG_TARGET = game_debug.exe
	RM = rm -f
	MKDIR = mkdir -p
	CP = cp -f
	SEP = /
	CC = gcc
	CXX = g++
endif

MAIN_SRC = main.cpp
EMU_SRCS_CPP = $(wildcard emulator/src/*.cpp)
EMU_SRCS_C = $(wildcard emulator/src/*.c)
CPU_INST_SRCS = $(wildcard emulator/src/cpu_instructions/*.cpp)

SRCS_CPP = $(MAIN_SRC) $(EMU_SRCS_CPP) $(CPU_INST_SRCS)
SRCS_C = $(EMU_SRCS_C)
OBJS_CPP = $(SRCS_CPP:.cpp=.o)
OBJS_C = $(SRCS_C:.c=.o)
OBJS = $(OBJS_CPP) $(OBJS_C)
BIN_DIR = bin



$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(BIN_DIR)$(SEP)$(TARGET) $(LIBS)
	-$(CP) SDL$(SEP)bin$(SEP)SDL3.dll $(BIN_DIR)$(SEP)
	-$(CP) SDL_TTF$(SEP)bin$(SEP)SDL3_ttf.dll $(BIN_DIR)$(SEP)

debug: CXXFLAGS := $(DEBUG_CXXFLAGS)
debug: CFLAGS := $(DEBUG_CFLAGS)
debug: $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) -o $(BIN_DIR)$(SEP)$(DEBUG_TARGET) $(LIBS)
	-$(CP) SDL$(SEP)bin$(SEP)SDL3.dll $(BIN_DIR)$(SEP)
	-$(CP) SDL_TTF$(SEP)bin$(SEP)SDL3_ttf.dll $(BIN_DIR)$(SEP)

$(BIN_DIR):
	-$(MKDIR) $(BIN_DIR)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	-$(RM) $(OBJS)
	-$(RM) $(BIN_DIR)$(SEP)$(TARGET)
	-$(RM) $(BIN_DIR)$(SEP)$(DEBUG_TARGET)

cleanobj:
	-$(RM) $(OBJS)

run: $(TARGET) cleanobj
	$(BIN_DIR)$(SEP)$(TARGET)

debug: debug cleanobj
	

runclean: $(TARGET)
	$(BIN_DIR)$(SEP)$(TARGET)
	$(MAKE) clean

.PHONY: clean cleanobj run runclean debug run_debug