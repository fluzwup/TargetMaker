
BINARY = MakeTarget

CXXSOURCES = main.cpp TextToPixels.cpp

OBJECTS = ${CXXSOURCES:.cpp=.o}

INCLUDES = -I . 

LOCATIONS = 

LIBRARIES = -lX11

CXXFLAGS = -ggdb -Wall
CXX = g++ 

.SUFFIXES:      .cpp .o

.cpp.o:
		@echo
		@echo Building $@		
		${CXX} ${CXXFLAGS} ${INCLUDES} -c -o $@ $<

all:            ${OBJECTS} ${BINARY} 

${BINARY}:      ${OBJECTS}
		@echo
		@echo Building ${BINARY} Executable
		${CXX} -o $@ \
		${OBJECTS}  \
		${LIBRARIES} \
		${LOCATIONS}
                         
clean:
		rm -f ${BINARY} *.o 



