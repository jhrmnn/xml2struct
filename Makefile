mexext := $(shell mexext)

all: xml2struct.${mexext}

%.${mexext}: %.cc
	mex CC=c++ CXX=c++ LD=c++ -I./rapidxml-1.13 -I/usr/local/Cellar/boost/1.58.0/include $<

clean:
	-rm *.${mexext}

test:
	matlab -nojvm -r "tic; disp(xml2struct('test.xml')); toc; exit"

