#include <u.h>
#include <libc.h>

#include "dat.h"
#include "fns.h"

void main(int argc, char **argv){
	if (argc < 3)
		exits("usage");

	aimlogin(argv[1], argv[2]);
}

