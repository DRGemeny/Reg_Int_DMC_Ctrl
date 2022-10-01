CC=g++

CFLAGS=-c -Wall
#CFLAGS=-lrt -ldl
LDFLAGS=-ldl

SOURCES=example.cpp \
	Log.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=TestLog

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
