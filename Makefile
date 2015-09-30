BIN=oscarlnet
CFLAGS=-g
OFILES=\
	flap.o\
	main.o\
	snac.o\
	tlv.o\
	login.o\
	ssi.o\
	parse.o\

HFILES=dat.h\
	fns.h\
	linux.h\

$(BIN):	$(OFILES) $(HFILES)
	$(CC) -o $(BIN) $(OFILES) -lcrypto 

clean:
	rm -f $(BIN) $(OFILES)
