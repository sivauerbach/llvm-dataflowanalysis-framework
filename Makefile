CXX          = clang++
SOURCEDIR	 = src
FRAMEWORKDIR = $(SOURCEDIR)/Framework
ANALYSISDIR  = $(SOURCEDIR)/AnalysisTypes
PASSESDIR  	 = $(SOURCEDIR)/Passes
CXXFLAGS     = -rdynamic $(shell llvm-config --cxxflags) -fPIC -g -std=c++20 -I$(SOURCEDIR)
LDFLAGS      = $(shell llvm-config --ldflags | tr '\n' ' ') -Wl,--exclude-libs,ALL
BUILDDIR     = build
DEPDIR       = $(BUILDDIR)/.deps
DEPFLAGS 	 = -MT $@ -MMD -MP -MF $(DEPDIR)/$(patsubst $(BUILDDIR)/%.o,%.d,$@)

TESTS             = dominators faint
OPTIMIZER_SOURCES = src/unifiedpass.cpp 

FRAMEWORK_SOURCES = $(wildcard $(FRAMEWORKDIR)/*.cpp)
ANALYSIS_SOURCES  = $(wildcard $(ANALYSISDIR)/*.cpp)
PASSES_SOURCES	  = $(wildcard $(PASSESDIR)/*.cpp)

FRAMEWORK_OBJS 	  = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(FRAMEWORK_SOURCES))
ANALYSIS_OBJS     = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(ANALYSIS_SOURCES))
PASSES_OBJS  	  = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(PASSES_SOURCES))
OPTIMIZER_OBJS    = $(OPTIMIZER_SOURCES:%.cpp=$(BUILDDIR)/%.o)

OPTIMIZER_LIBS    = $(BUILDDIR)/unifiedpass.so
TESTS_PRE         = $(TESTS:%=$(BUILDDIR)/tests/%-m2r.ll)
OPT_OUT           = $(TESTS:%=$(BUILDDIR)/tests/%-opt.ll)
DEPFILES 		  = $(patsubst %.cpp,$(DEPDIR)/%.d,$(FRAMEWORK_SOURCES) $(ANALYSIS_SOURCES) $(PASSES_SOURCES) $(OPTIMIZER_SOURCES))

.PHONY: all clean tests
.SECONDARY:

all: $(OPTIMIZER_LIBS)
tests: $(TESTS_PRE)
opt: $(OPT_OUT)

clean:
	rm -rf $(BUILDDIR)

$(BUILDDIR)/$(FRAMEWORKDIR)/%.o: $(FRAMEWORKDIR)/%.cpp | $(DEPDIR)/$(FRAMEWORKDIR) $(BUILDDIR)/$(FRAMEWORKDIR)
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/$(ANALYSISDIR)/%.o: $(ANALYSISDIR)/%.cpp | $(DEPDIR)/$(ANALYSISDIR) $(BUILDDIR)/$(ANALYSISDIR)
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/$(PASSESDIR)/%.o: $(PASSESDIR)/%.cpp | $(DEPDIR)/$(PASSESDIR) $(BUILDDIR)/$(PASSESDIR)
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/$(SOURCEDIR)/%.o: $(SOURCEDIR)/%.cpp | $(DEPDIR)/$(SOURCEDIR) $(BUILDDIR)/$(SOURCEDIR)
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/unifiedpass.so: $(OPTIMIZER_OBJS) $(ANALYSIS_OBJS) $(PASSES_OBJS) $(FRAMEWORK_OBJS)
	$(CXX) -shared -o $@ $(LDFLAGS) $(OPTIMIZER_OBJS) $(ANALYSIS_OBJS) $(PASSES_OBJS) $(FRAMEWORK_OBJS)

# Compile .c tests into bytecode:
$(BUILDDIR)/tests/%.bc: tests/%.c | $(BUILDDIR)/tests
	clang -fno-discard-value-names -Xclang -disable-O0-optnone -O0 -emit-llvm -c $< -o $@

#Convert compiled .bc to M2R file *-m2r.bc:
$(BUILDDIR)/tests/%-m2r.bc:  $(BUILDDIR)/tests/%.bc | $(BUILDDIR)/tests
	opt -bugpoint-enable-legacy-pm=1 -mem2reg $< -o $@

#Convert compiled M2R bytecode inlo IR (.LL) file:
$(BUILDDIR)/tests/%-m2r.ll: $(BUILDDIR)/tests/%-m2r.bc | $(BUILDDIR)/tests
	llvm-dis $< -o $@

$(BUILDDIR)/tests/%-opt.bc: $(BUILDDIR)/tests/%-m2r.bc $(OPTIMIZER_LIBS) | $(BUILDDIR)/tests
	opt -bugpoint-enable-legacy-pm=1 $(OPTIMIZER_LIBS:%=-load-pass-plugin=%) -passes='$*' $< -o $@ >  $(BUILDDIR)/tests/$*.out 

$(BUILDDIR)/tests/%-opt.ll: $(BUILDDIR)/tests/%-opt.bc
	llvm-dis $< -o $@

$(DEPDIR) $(DEPDIR)/$(SOURCEDIR) $(DEPDIR)/$(FRAMEWORKDIR) $(DEPDIR)/$(ANALYSISDIR) $(DEPDIR)/$(PASSESDIR) \
$(BUILDDIR) $(BUILDDIR)/$(SOURCEDIR) $(BUILDDIR)/tests $(BUILDDIR)/$(FRAMEWORKDIR) $(BUILDDIR)/$(ANALYSISDIR) $(BUILDDIR)/$(PASSESDIR):
	@mkdir -p $@

-include $(wildcard $(DEPFILES))
