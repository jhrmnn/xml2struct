MEXEXT:=`mexext`

all : xml2struct.$(MEXEXT)

xml2struct.$(MEXEXT) : xml2struct.cc
	mex CC=c++ CXX=c++ LD=c++ -I./rapidxml-1.13 xml2struct.cc

rebuild : 
	touch xml2struct.cc
	make

clean :
	-rm xml2struct.$(MEXEXT)

