MAINSOURCE := Main.cpp base/tests/LoggingTest.cpp tests/HTTPClient.cpp
SOURCE  := $(wildcard *.cpp base/*.cpp tests/*.cpp MemoryPool/*.cpp)
override SOURCE := $(filter-out $(MAINSOURCE),$(SOURCE))
OBJS    := $(patsubst %.cpp,%.o,$(SOURCE))

TARGET  := WebServer
CC      := g++
LIBS    := -lpthread
INCLUDE:= -I./usr/local/lib
CFLAGS  := -std=c++11 -g -Wall -O3 -D_PTHREADS
CXXFLAGS:= $(CFLAGS)

# Test object
SUBTARGET1 := LoggingTest
SUBTARGET2 := HTTPClient

.PHONY : objs clean veryclean rebuild all tests debug
all : $(TARGET) $(SUBTARGET1) $(SUBTARGET2)
objs : $(OBJS)
rebuild: veryclean all

tests : $(SUBTARGET1) $(SUBTARGET2)
clean :
	find . -name '*.o' | xargs rm -f
veryclean :
	find . -name '*.o' | xargs rm -f
	find . -name $(TARGET) | xargs rm -f
	find . -name $(SUBTARGET1) | xargs rm -f
	find . -name $(SUBTARGET2) | xargs rm -f
debug:
	@echo $(SOURCE)

$(TARGET) : $(OBJS) Main.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
# $@代表目标，这里是$(TARGET)

$(SUBTARGET1) : $(OBJS) base/tests/LoggingTest.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

$(SUBTARGET2) : $(OBJS) tests/HTTPClient.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
