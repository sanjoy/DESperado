BUILD_DIR=build/

ifeq ($(mode),release)
	CXXFLAGS = -Wall -Werror -O3 -g -I./src/
else
	mode = debug
	CXXFLAGS = -g3 -Wall -I./src/
endif

CXX = g++

SRC :=  cipher-modes.hpp	\
	cipher-modes-inl.hpp	\
	des.cpp			\
	des.hpp			\
	desl-inl.hpp		\
	main.cpp		\
	utils.hpp               \
	files.hpp               \
	files.cpp

OBJECTS := $(addprefix $(BUILD_DIR), $(patsubst %.cpp, %.o, $(filter %.cpp, $(SRC))))
HEADERS := $(filter %.hpp, $(SRC))

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

desperado: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS)

$(BUILD_DIR)%.o: src/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@


.PHONY:clean
clean:
	find . -name "*.o" | xargs rm -vf
	rm -vf fooexe
