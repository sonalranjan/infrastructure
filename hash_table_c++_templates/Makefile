
#------------------------------------------------------
# tested on linux and mac
# in case of problems: changing variables here should suffice
#------------------------------------------------------
CXX = g++ -g
CXXFLAGS = -I.
MAKEDEPEND = g++ -MM $(CXXFLAGS)
HDRS = hash.h
SRCS = hash_use.cxx
OBJS = $(addprefix objdir/, $(addsuffix .o, $(basename $(SRCS))) )
#OBJS = (objdir/hash.o)
DEPS = $(addprefix objdir/, $(addsuffix .d, $(basename $(SRCS))) )

#------------------------------------------------------
all: exe

exe: $(OBJS)
	$(CXX) $(OBJS) -o $@ 

objdir/%.o: %.cxx objdir/%.d 
	(test -d objdir) || (mkdir -p objdir)
	$(CXX) $(CXXFLAGS) -c $< -o $@

objdir/%.d: %.cxx $(HDRS)
	(test -d objdir) || (mkdir -p objdir)
	$(MAKEDEPEND) $< -o $@

cln clean:
	$(RM) -rf objdir exe

include $(DEPS)
