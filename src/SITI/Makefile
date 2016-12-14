CC = clang++
CFLAGS = -W -Wall -g \
			 -I/usr/include/boost \
			 $(shell pkg-config --cflags opencv)

LDFLAGS = -L/usr/local/opt/boost/lib \
		  -lboost_program_options \
		  $(shell pkg-config --libs opencv)

SRC = main.cpp
OBJS = $(SRC:.cpp=.o)
AOUT = SITI

all : $(AOUT)

SITI : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
%.o : %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<
clean :
	$(RM) *.o $(AOUT)
