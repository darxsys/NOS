NAME=sha3
CC=g++
LD=g++

SRC_DIR=src
OBJ_DIR=obj

SRC=$(shell find $(SRC_DIR) -type f -regex ".*\.cpp")
OBJ=$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

CXX_FLAGS = -c -Wall
LD_FLAGS =


all: $(NAME)

$(NAME): $(OBJ)
	@echo [LD]
	$(LD) -o $@ $^ $(LD_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo [CC]
	@mkdir -p $(dir $@)
	$(CC) -o $@ $< $(CXX_FLAGS)

clean:
	rm -r $(NAME) $(OBJ_DIR)