CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -pedantic -Iinclude
LDFLAGS :=

CLI_TARGET := search_engine
API_TARGET := search_engine_api
BUILD_DIR := build

CORE_SOURCES := \
	src/core/analytics.cpp \
	src/core/posting_list.cpp \
	src/core/search_engine.cpp \
	src/core/trie.cpp \
	src/parsing/query_expander.cpp \
	src/parsing/tokenizer.cpp \
	src/ranking/bm25.cpp \
	src/utils/fuzzy.cpp

CLI_OBJECTS := $(BUILD_DIR)/src/main.o $(CORE_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
API_OBJECTS := $(BUILD_DIR)/src/api_runner.o $(CORE_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
OBJECTS := $(CLI_OBJECTS) $(API_OBJECTS)
DEPS := $(OBJECTS:.o=.d)

.PHONY: all clean run

all: $(CLI_TARGET) $(API_TARGET)

$(CLI_TARGET): $(CLI_OBJECTS)
	$(CXX) $(CXXFLAGS) $(CLI_OBJECTS) $(LDFLAGS) -o $@

$(API_TARGET): $(API_OBJECTS)
	$(CXX) $(CXXFLAGS) $(API_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(CLI_TARGET)
	./$(CLI_TARGET) -i datasets/smallDataset.txt -k 5

clean:
	rm -rf $(BUILD_DIR) $(CLI_TARGET) $(API_TARGET)

docker-build:
	docker build -t intellisearch:latest .

docker-run:
	docker run -it --rm -p 8000:8000 --name intellisearch-app intellisearch:latest

docker-clean:
	docker rmi intellisearch:latest

-include $(DEPS)
