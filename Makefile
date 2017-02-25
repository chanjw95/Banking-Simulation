# CISC 2200: Data Structures
# Makefile for Project 4
#
# Author: Arthur G. Werschulz
# Date:   4 April 2016

CXXFLAGS=-Wall -std=c++14 -g
# change this to
# CXX=gfilt
# if you want STL error msgs filtered
CXX=gfilt

######################################################################
#
# Assuming that you're using the STL's priority_queue, there is no
# need for you to touch anything beyond this point.  If you decide to
# use another implementation, you're on your own here.
#
######################################################################

proj4: proj4.o 
	$(CXX) $(CXXFLAGS) -o proj4 proj4.o
clean:
	rm -f *.o proj4

veryclean:
	rm -f *.o proj4 \#* *~ core*
