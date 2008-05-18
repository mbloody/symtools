LIBS = -lz
INCL = -I.
CXXFLAGS = $(INCL)
LDFLAGS = $(LIBS)

BIN = $(DESTDIR)/usr/bin

OBJS = main.o

unback: $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o unback

install: unback
	install -d $(BIN)
	install -m755 ./unback $(BIN)

uninstall:
	rm -rf $(BIN)/unback
        
clean:
	rm -rf $(OBJS) unback

