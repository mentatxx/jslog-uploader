CP		= gcc
CXX		= g++
CFLAGS		= -pipe -D_REENTRANT -Wall -W -g
CXXFLAGS	= -pipe -D_REENTRANT -Wall -W -g
INCPATH		= -I.
LINK		= g++
LIBS		= $(SUBLIBS) -lyaml-cpp -lcurl -ljson -lssl -lcrypto

OBJECTS_DIR = obj

HEADERS = CApplication.h \
	    CLog.h

SOURCES = CApplication.cpp \
	    CLog.cpp \
	    main.cpp

OBJECTS = obj/CApplication.o \
	    obj/CLog.o \
	    obj/main.o

TARGET	= logs_uploader

$(TARGET): $(OBJECTS)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

#Compile
obj/CApplication.o: CApplication.cpp CApplication.h CLog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/CApplication.o CApplication.cpp

obj/CLog.o: CLog.cpp CLog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/CLog.o CLog.cpp

obj/main.o: main.cpp CApplication.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/main.o main.cpp