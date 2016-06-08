
PAHO_C_LIB ?= /usr/local/lib
PAHO_CPP_PATH ?= /root/base/org.eclipse.paho.mqtt.cpp
PIGEON_CPPLIB_PATH ?= ../cpp_lib
PIGEON_ZK_INCPATH ?= /usr/local/include/zookeeper
MONGOCXX_PATH ?= /root/db/mongo-cli/mongo-cxx-driver/build/install

RAPIDJSON_PATH ?= $(PIGEON_CPPLIB_PATH)/third_party/rapidjson
CXXFLAGS += -Wall -std=c++11
CPPFLAGS += -I$(PIGEON_CPPLIB_PATH)/third_party/easyloggingpp -I$(PIGEON_ZK_INCPATH) -I$(RAPIDJSON_PATH)/include -I$(PIGEON_CPPLIB_PATH)/third_party/hiredis-lib/inc -I$(PIGEON_CPPLIB_PATH)/isc -I$(PIGEON_CPPLIB_PATH)/zmq_module -I$(PIGEON_CPPLIB_PATH)/mqtt_module -I$(PIGEON_CPPLIB_PATH)/utils  -I$(PAHO_CPP_PATH)/src -I$(MONGOCXX_PATH)/include/bsoncxx/v_noabi -I$(MONGOCXX_PATH)/include/mongocxx/v_noabi

ifdef DEBUG
  CPPFLAGS += -DDEBUG -DELPP_THREAD_SAFE -DELPP_STACKTRACE_ON_CRASH -DELPP_FORCE_USE_STD_THREAD -DELPP_DEFAULT_LOG_FILE='"./logs/dev_controller.log"' 
  CXXFLAGS += -g -O0
else
  CPPFLAGS += -D_NDEBUG  -DELPP_THREAD_SAFE -DELPP_STACKTRACE_ON_CRASH -DELPP_FORCE_USE_STD_THREAD -DELPP_DEFAULT_LOG_FILE='"./logs/dev_controller.log"'
  CXXFLAGS += -O2
endif

LDLIBS += -L$(PAHO_CPP_PATH)/lib -L$(PAHO_C_LIB) -L$(PIGEON_CPPLIB_PATH)/third_party/hiredis-lib/lib -L$(MONGOCXX_PATH)/lib -lae -ljemalloc -lhiredis -lzmq -lmqttpp -lpaho-mqtt3as -lpthread -lzookeeper_mt -lzoolock -lmongocxx -lbsoncxx
CXX=g++

SRCS=$(wildcard ./*.cpp) $(wildcard $(PIGEON_CPPLIB_PATH)/isc/*.cpp) $(wildcard $(PIGEON_CPPLIB_PATH)/zmq_module/*.cpp) $(wildcard $(PIGEON_CPPLIB_PATH)/mqtt_module/*.cpp) $(wildcard $(PIGEON_CPPLIB_PATH)/utils/*.c)
#OBJ=$(addprefix ./, $(addsuffix .o, $(basename $(SRC))))
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
TARGET=dev_controller

all: $(TARGET)
	
$(TARGET): $(SRCS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LDLIBS)
#$(OBJS):$(SRCS)
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(LDLIBS)
clean:
	rm -f $(TARGET) $(OBJ)
