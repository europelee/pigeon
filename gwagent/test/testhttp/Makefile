
DEBUG ?= 1
HTTPCLIENT_PATH ?= /root/web/pigeon/cpp_lib/http_module
CXXFLAGS += -Wall -std=c++11
CPPFLAGS += -I$(HTTPCLIENT_PATH)

ifdef DEBUG
  CPPFLAGS += -DDEBUG
  CXXFLAGS += -g -O0
else
  CPPFLAGS += -D_NDEBUG
  CXXFLAGS += -O2
endif

LDLIBS += -lcurl
CXX=g++

SRCS=$(wildcard ./*.cpp) $(wildcard $(HTTPCLIENT_PATH)/*.cpp)
#OBJ=$(addprefix ./, $(addsuffix .o, $(basename $(SRC))))
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
TARGET=testhttpclient

all: $(TARGET)
	
$(TARGET): $(SRCS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LDLIBS)
#$(OBJS):$(SRCS)
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(LDLIBS)
clean:
	rm -f $(TARGET) $(OBJ)
