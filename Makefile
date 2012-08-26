BUILD_DIR = build
SRC_DIR = src

ifeq ($(mode),release)
	CXXFLAGS = -Wall -Werror -O4 -g -I./$(SRC_DIR)/
else
	mode = debug
	CXXFLAGS = -g3 -Wall -I./$(SRC_DIR)/
endif

CXX=g++

HEADERS :=				\
	src/cipher-modes.hpp		\
	src/cipher-modes-inl.hpp	\
	src/des.hpp			\
	src/des-inl.hpp			\
	src/utils.hpp			\
	src/files.hpp

.PHONY:all
all: information desperado

information:
ifneq ($(mode),release)
ifneq ($(mode),debug)
	@echo "Invalid build mode." 
	@echo "Please use 'make mode=release' or 'make mode=debug'"
	@exit 1
endif
endif
	@echo "Building on "$(mode)" mode"
	@echo ".........................."

desperado: $(BUILD_DIR)/des.o $(BUILD_DIR)/main.o $(BUILD_DIR)/files.o
	$(CXX) -o $@ $(BUILD_DIR)/des.o $(BUILD_DIR)/main.o $(BUILD_DIR)/files.o

$(BUILD_DIR)/des.o : $(SRC_DIR)/des.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $(SRC_DIR)/des.cpp -o $@

$(BUILD_DIR)/main.o : $(SRC_DIR)/main.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $(SRC_DIR)/main.cpp -o $@

$(BUILD_DIR)/files.o : $(SRC_DIR)/files.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $(SRC_DIR)/files.cpp -o $@


.PHONY:clean
clean:
	find $(BUILD_DIR) -name "*.o" | xargs rm -vf	
	rm -vf desperado
