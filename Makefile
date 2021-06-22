#
# Parallel Fingerprint Matching for UCC
# by Dhoto <dhoto@aoki.ecei.tohoku.ac.jp>
# 2004
#
# Dependencies : libmagick6, libtiff3g, fftw3, Lam-MPI
#
# To compile this source just type "make"
# To run this program using LAM on 4 UCC:
# > lamboot -v cluster.scheme
# > mpirun -np 4 fpoc
#
# Have a nice day
#


all:
	rm -f fpoc
	rm -f *.o
	mpicc -c *.c -Wall 
	rm -f main.o
	mpicc `Magick-config --cflags --cppflags`  main.c `Magick-config --ldflags --libs` -o fpoc *.o -lm -ltiff -lfftw3 -Wall 
	cp fpoc $(HOME)/bin/fpoc
