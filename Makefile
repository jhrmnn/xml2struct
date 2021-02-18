CC ?= c++
CXX ?= c++
LD ?= c++
MEX ?= mex
CFLAGS ?= -DMX_COMPAT_32
mexext := $(shell mexext)

all: xml2struct.$(mexext)

xml2struct.$(mexext): xml2struct.cc | rapidxml
	$(MEX) $(CFLAGS) CC=$(CC) CXX=$(CXX) LD=$(LD) -I./rapidxml $<

rapidxml:
	mkdir -p rapidxml && wget -O - http://iweb.dl.sourceforge.net/project/rapidxml/rapidxml/rapidxml%201.13/rapidxml-1.13.zip | tar -C rapidxml -zx --strip-components 1

clean:
	rm -f *.$(mexext)

distclean: clean
	rm -rf rapidxml

test:
	matlab -nodisplay -nojvm -r "tic; disp(xml2struct('test.xml')); toc; exit"
