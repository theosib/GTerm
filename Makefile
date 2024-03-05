#
# See Makefile.config for configuration options
#

all: gterm

include Makefile.config

LIBOBJS = actions.o states.o utils.o gterm.o pseudo.o vt52_states.o
GOBJS = xintfc.o xkeys.o

ifeq ($(PTY_CODE),openpty)
CPPFLAGS += -DUSE_OPENPTY
LDFLAGS += -lutil
endif

ifeq ($(PTY_CODE),unix98)
CPPFLAGS += -DUSE_UNIX98
endif

ifeq ($(PTY_CODE),classic)
CPPFLAGS += -DUSE_CLASSIC_PTY
endif

.PHONY: all clean install

libgterm.a: $(LIBOBJS)
	rm -f $@ || exit 0
	ar csq $@ $(LIBOBJS)

gterm: $(GOBJS) libgterm.a
	$(CXX) $(CXXFLAGS) -o $@ $(GOBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

clean:
	-rm -f *.o *.i libgterm.a gterm

install: all
	mkdir -p $(DESTDIR)$(bindir)
	install -m $(BIN_MODE) -o $(BIN_OWNER) -g $(BIN_GROUP) gterm $(DESTDIR)$(bindir)/gterm

# dependency tracing for header files, used for pseudo.hpp
%.hpp:
	touch $@

# dependencies
include Makefile.depend
