CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -pedantic -Iinclude
LDFLAGS :=

TARGET := search_engine
BUILD_DIR := build

SOURCES := \
	src/main.cpp \
	src/core/posting_list.cpp \
	src/core/search_engine.cpp \
	src/core/trie.cpp \
	src/parsing/tokenizer.cpp \
	src/ranking/bm25.cpp

OBJECTS := $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJECTS:.o=.d)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	./$(TARGET) -i datasets/smallDataset.txt -k 5

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)
