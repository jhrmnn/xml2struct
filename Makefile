BOOST ?= /usr/local/Cellar/boost/1.58.0/include
CC ?= c++
CXX ?= c++
LD ?= c++
mexext := $(shell mexext)

all: xml2struct.${mexext}

%.${mexext}: %.cc
	mex CC=${CC} CXX=${CXX} LD=${LD} -I./rapidxml-1.13 -I${BOOST} $<

clean:
	-rm *.${mexext}

test:
	matlab -nojvm -r "tic; disp(xml2struct('test.xml')); toc; exit"

