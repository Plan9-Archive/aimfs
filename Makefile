BIN=aimtelnet
OFILES=\
	flap.o\
	main.o\
	snac.o\
	tlv.o\
	login.o\

HFILES=dat.h\
	fns.h\

$(BIN):	$(OFILES)
	$(LD) -o $(BIN) $(OFILES)

clean:
	rm $(BIN) $(OFILES)
