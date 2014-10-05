CC = gcc # compiler
CFLAGS = -Wall -g # compile flags
LIBS = # libs

SRCS = cpusched.c # source files
OBJS = $(SRCS:.c=.o) # object files

TARG = cpusched # target

all: $(TARG)
# generates the target executable
$(TARG): $(OBJS)
	$(CC) -o $(TARG) $(OBJS) $(LIBS)

%.o: %.c # generates the object files
	$(CC) $(CFLAGS) -c $*.c

# cleans stuff
clean:
	rm -f $(OBJS) $(TARG) *~