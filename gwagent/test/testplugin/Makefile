I_DEV_PATH ?= ../../../dev_party
CFLAGS = -shared -fPIC -Wall -g -D _DEBUG  -I./ -I$(I_DEV_PATH) -Wl,-Bdynamic -lpthread
CC=g++
SRC=$(wildcard ./*.cpp)
OBJ=$(addprefix ./, $(addsuffix .o, $(basename $(SRC))))
TARGET=test_plugin.so

all: $(TARGET)
		
$(TARGET): $(SRC)
		$(CC)  -o $@  $^ $(CFLAGS)

clean:
		rm -f $(TARGET) $(OBJ)
