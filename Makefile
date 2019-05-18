#hugh boy
#march 25
#unreasonably complicated makefile just for fun

CXX := g++
CXX_FLAGS := -std=c++11 -Wall -g
INCLUDE_DIR := src
CPP_DIR := src
BUILD_DIR := build
NCURSES_FLAGS := -lncurses
ASIO_FLAGS := -lpthread -DASIO_STANDALONE 
CLIENT_MAIN := src/client.cpp
SERVER_MAIN := src/server.cpp

MAINS_OBJ := $(patsubst $(CPP_DIR)%.cpp,$(BUILD_DIR)%.o,$(CLIENT_MAIN) $(SERVER_MAIN))
HEADERS := $(wildcard $(INCLUDE_DIR)/*.h)
SOURCES := $(wildcard $(CPP_DIR)/*.cpp)
OBJS := $(filter-out $(MAINS_OBJ),$(patsubst $(CPP_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES)))

build/%.o: src/%.cpp $(HEADERS)
	$(CXX) $(CXX_FLAGS) -c -o $@ $< $(NCURSES_FLAGS) $(ASIO_FLAGS)

all: $(BUILD_DIR)/client $(BUILD_DIR)/server

$(BUILD_DIR)/client: $(OBJS) $(HEADERS) $(CLIENT_MAIN)
	$(CXX) $(CXX_FLAGS) -o $@ $(CLIENT_MAIN) $(OBJS) $(NCURSES_FLAGS) $(ASIO_FLAGS)

$(BUILD_DIR)/server: $(OBJS) $(HEADERS) $(SERVER_MAIN)
	$(CXX) $(CXX_FLAGS) -o $@ $(SERVER_MAIN) $(OBJS) $(NCURSES_FLAGS) $(ASIO_FLAGS)

clean:
	rm build/*

debug:
	@echo "MAINSOBJ   $(MAINS_OBJ)"
	@echo "HEADERS    $(HEADERS)"
	@echo "SOURCES    $(SOURCES)"
	@echo "OBJS       $(OBJS)"
