</$objtype/mkfile

TARG=oscarlnet

OFILES=\
	flap.$O\
	threadmain.$O\
	snac.$O\
	tlv.$O\
	login.$O\
	ssi.$O\
	parse.$O\

HFILES=dat.h\
	fns.h\

UPDATE=\
	mkfile\
	$HFILES\
	${OFILES:%.$O=%.c}\

%.$O:	%.c
	$CC $CFLAGS $stem.c

all:V: 	$TARG imprefixer

$TARG:	$OFILES $HFILES
	$LD -o $TARG $OFILES

imprefixer: imprefixer.$O flap.$O tlv.$O snac.$O $HFILES
	$LD -o imprefixer imprefixer.$O flap.$O tlv.$O snac.$O

clean:
	rm -f $TARG *.[$OS] imprefixer
