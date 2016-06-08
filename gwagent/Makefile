
PAHO_C_LIB ?= /usr/local/lib
PAHO_CPP_PATH ?= /root/base/org.eclipse.paho.mqtt.cpp
PIGEON_CPPLIB_PATH ?= ../cpp_lib
RAPIDJSON_PATH ?= $(PIGEON_CPPLIB_PATH)/third_party/rapidjson
I_DEVICE_PATH ?= ../dev_party
CXXFLAGS += -Wall -std=c++11
CPPFLAGS += -I$(PIGEON_CPPLIB_PATH)/third_party/easyloggingpp -I./ -I$(RAPIDJSON_PATH)/include -I$(PIGEON_CPPLIB_PATH)/isc -I$(PIGEON_CPPLIB_PATH)/http_module -I$(PIGEON_CPPLIB_PATH)/mqtt_module -I$(PIGEON_CPPLIB_PATH)/utils  -I$(PAHO_CPP_PATH)/src -I$(I_DEVICE_PATH)
DEBUG ?= 1
ifdef DEBUG
  CPPFLAGS += -DDEBUG -DELPP_DEFAULT_LOG_FILE='"./logs/gwagent.log"' -DELPP_THREAD_SAFE -DELPP_STACKTRACE_ON_CRASH -DELPP_FORCE_USE_STD_THREAD
  CXXFLAGS += -g -O0
else
  CPPFLAGS += -D_NDEBUG -DELPP_DEFAULT_LOG_FILE='"./logs/gwagent.log"' -DELPP_THREAD_SAFE -DELPP_STACKTRACE_ON_CRASH -DELPP_FORCE_USE_STD_THREAD
  CXXFLAGS += -O2
endif

LDLIBS += -L$(PAHO_CPP_PATH)/lib -L$(PAHO_C_LIB) -lmqttpp -lpaho-mqtt3as -lcurl -lpthread -ldl
CXX=g++

SRCS=$(wildcard ./*.cpp) $(wildcard $(PIGEON_CPPLIB_PATH)/isc/*.cpp) $(wildcard $(PIGEON_CPPLIB_PATH)/http_module/*.cpp) $(wildcard $(PIGEON_CPPLIB_PATH)/mqtt_module/*.cpp) $(wildcard $(PIGEON_CPPLIB_PATH)/utils/*.c)
#OBJ=$(addprefix ./, $(addsuffix .o, $(basename $(SRC))))
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
TARGET=gwagent

all: $(TARGET)
	
$(TARGET): $(SRCS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LDLIBS)
#$(OBJS):$(SRCS)
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(LDLIBS)
clean:
	rm -f $(TARGET) $(OBJ)
