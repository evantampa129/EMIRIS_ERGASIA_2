# set compiler and flags
# flags are similar to those used in slide 35 of "Φ1-Οργάνωση&Σχεδίαση"
# opencv headers are passed as system headers so their own warnings are not shown
# -MMD -MP generate .d files so objects are rebuilt when a header changes
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -MMD -MP -Iinclude $(shell pkg-config --cflags-only-I opencv4 | sed 's/-I/-isystem /g')

# pkg-config handles openCV linking
LDFLAGS = `pkg-config --libs opencv4`

# set directories and target
SRC_DIR = src
BUILD_DIR = build
TARGET = search

# define source and object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

.PHONY: all clean
clean:
	rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d $(TARGET)
