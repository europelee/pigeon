LOCAL_PATH := $(call my-dir)

$(warning  ****LOCAL_PATH**** )  
$(warning  $(LOCAL_PATH))

PAHO_C_PATH = /root/base/org.eclipse.paho.mqtt.c

$(warning  $(PAHO_C_PATH))  

include $(CLEAR_VARS)  
   
LOCAL_MODULE := npaho-mqtt3as
LOCAL_SRC_FILES := $(PAHO_C_PATH)/libs/armeabi/libpaho-mqtt3as.so 

include $(PREBUILT_SHARED_LIBRARY)

PAHO_CPP_PATH = /root/base/org.eclipse.paho.mqtt.cpp

$(warning  $(PAHO_CPP_PATH))  

include $(CLEAR_VARS)  
   
LOCAL_MODULE := mqttpp
LOCAL_SRC_FILES := $(PAHO_CPP_PATH)/libs/armeabi/libmqttpp.so 

include $(PREBUILT_SHARED_LIBRARY)
  
include $(CLEAR_VARS)  
 
LOCAL_MODULE    := gwagent
define walk
 $(wildcard $(1)) $(foreach e, $(wildcard $(1)/*), $(call walk, $(e)))
endef

GWAGENTFILES = $(call walk, $(LOCAL_PATH))
GWAGENT_FILE_LIST := $(filter %.cpp, $(GWAGENTFILES)) 

LOCAL_SRC_FILES := $(filter-out test/testhttp/%.cpp, $(GWAGENT_FILE_LIST:$(LOCAL_PATH)/%=%)) 

$(warning  $(LOCAL_SRC_FILES))

ISCFILES = $(call walk, $(LOCAL_PATH)/../cpp_lib/isc)
ISC_FILE_LIST := $(filter %.cpp, $(ISCFILES)) 
#ISC_FILE_LIST := $(wildcard $(LOCAL_PATH)/../cpp_lib/isc/*.cpp)
LOCAL_SRC_FILES +=  $(ISC_FILE_LIST:$(LOCAL_PATH)/%=%)

$(warning  $(LOCAL_SRC_FILES))

MMFILES = $(call walk, $(LOCAL_PATH)/../cpp_lib/mqtt_module)
MM_FILE_LIST := $(filter %.cpp, $(MMFILES)) 
LOCAL_SRC_FILES +=  $(MM_FILE_LIST:$(LOCAL_PATH)/%=%)

$(warning  $(LOCAL_SRC_FILES))

UTILSFILES = $(call walk, $(LOCAL_PATH)/../cpp_lib/utils)
UTILS_FILE_LIST := $(filter %.c, $(UTILSFILES)) 
LOCAL_SRC_FILES +=  $(UTILS_FILE_LIST:$(LOCAL_PATH)/%=%)

$(warning  $(LOCAL_SRC_FILES))


LOCAL_SHARED_LIBRARIES := mqttpp paho-mqtt3a

LOCAL_C_INCLUDES += \
$(LOCAL_PATH) \
$(LOCAL_PATH)/../cpp_lib/third_party/rapidjson/include \
$(LOCAL_PATH)/../cpp_lib/isc \
$(LOCAL_PATH)/../cpp_lib/mqtt_module \
$(LOCAL_PATH)/../cpp_lib/utils \
$(PAHO_C_PATH)/src \
$(PAHO_CPP_PATH)/src \
$(JNI_H_INCLUDE) \

include $(BUILD_EXECUTABLE)
