C++ = g++
CC = gcc
DFLAGS = -DDEBUG
OFLAGS =
LFLAGS = -lboost_system -lboost_date_time -lboost_program_options
CFLAGS =

CFLAGS += $(OFLAGS) $(DFLAGS)

OBJS = client.o clientlist.o config.o game.o gamelist.o main.o moreobs.o protocol.o util.o control.o
PROGS = ./moreobs

all: $(OBJS) $(PROGS)

./moreobs: $(OBJS)
	$(C++) -o ./moreobs $(OBJS) $(LFLAGS)

clean:
	rm -f $(OBJS) $(PROGS)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

./moreobs: $(OBJS)
all: $(PROGS)
client.o: includes.h util.h client.h moreobs.h protocol.h game.h gamelist.h
clientlist.o: includes.h client.h clientlist.h
config.o: includes.h config.h
game.o: includes.h client.h game.h util.h
gamelist.o: includes.h gamelist.h game.h
main.o: includes.h config.h moreobs.h
moreobs.o: includes.h config.h moreobs.h client.h protocol.h gamelist.h
protocol.o: includes.h protocol.h util.h game.h gamelist.h client.h
util.o: includes.h util.h
control.o: includes.h control.h util.h game.h gamelist.h moreobs.h
