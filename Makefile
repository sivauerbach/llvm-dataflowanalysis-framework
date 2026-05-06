CXX          = clang++

SOURCEDIR    = src
FRAMEWORKDIR = $(SOURCEDIR)/Framework
ANALYSISDIR  = $(SOURCEDIR)/AnalysisTypes
PASSESDIR    = $(SOURCEDIR)/Passes

BUILDDIR     = build
DEPDIR       = $(BUILDDIR)/.deps
TEST_DIR     = $(BUILDDIR)/tests

CXXFLAGS     = -rdynamic $(shell llvm-config --cxxflags) \
               -fPIC -g -std=c++20 -I$(SOURCEDIR)

LDFLAGS      = $(shell llvm-config --ldflags | tr '\n' ' ') \
               -Wl,--exclude-libs,ALL

DEPFLAGS     = -MMD -MP -MF $(@:.o=.d) -MT $@

OPTIMIZER_SOURCES = src/unifiedpass.cpp

FRAMEWORK_SOURCES := $(shell find $(FRAMEWORKDIR) -name '*.cpp' -type f)
ANALYSIS_SOURCES  := $(shell find $(ANALYSISDIR)  -name '*.cpp' -type f)
PASSES_SOURCES    := $(shell find $(PASSESDIR)    -name '*.cpp' -type f)

FRAMEWORK_OBJS := $(FRAMEWORK_SOURCES:%.cpp=$(BUILDDIR)/%.o)
ANALYSIS_OBJS  := $(ANALYSIS_SOURCES:%.cpp=$(BUILDDIR)/%.o)
PASSES_OBJS    := $(PASSES_SOURCES:%.cpp=$(BUILDDIR)/%.o)
OPTIMIZER_OBJS := $(OPTIMIZER_SOURCES:%.cpp=$(BUILDDIR)/%.o)

OPTIMIZER_LIBS = $(BUILDDIR)/unifiedpass.so

# -------------------------------------------------------------------
# TEST DISCOVERY
# -------------------------------------------------------------------

TEST_SOURCES := $(shell find tests -name '*.c' -type f)

# stem = e.g. dominators/dominators (subdir/filename without .c)
TEST_STEMS := $(TEST_SOURCES:tests/%.c=%)

# For each stem, the pass name is the top-level directory component
# e.g. dominators/dominators -> dominators
test_pass = $(word 1,$(subst /, ,$(1)))
# The filename (leaf) of the stem
# e.g. dominators/dominators -> dominators
test_name = $(notdir $(1))
# Output directory for a stem
test_outdir = $(TEST_DIR)/$(1)

# Collect final targets
TEST_M2R_LL := $(foreach s,$(TEST_STEMS),$(call test_outdir,$(s))/$(call test_name,$(s)).ll)
TEST_OPT_LL := $(foreach s,$(TEST_STEMS),$(call test_outdir,$(s))/$(call test_name,$(s))-opt.ll)

# -------------------------------------------------------------------
# PHONY TARGETS
# -------------------------------------------------------------------

.PHONY: all clean tests opt

.SECONDARY:

all: $(OPTIMIZER_LIBS)

tests: $(TEST_M2R_LL)

opt: $(TEST_OPT_LL)

clean:
	rm -rf $(BUILDDIR)

# -------------------------------------------------------------------
# FRAMEWORK / PASS OBJECTS
# -------------------------------------------------------------------

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(@:.o=.d))
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@

# -------------------------------------------------------------------
# SHARED LIBRARY
# -------------------------------------------------------------------

$(BUILDDIR)/unifiedpass.so: \
	$(OPTIMIZER_OBJS) \
	$(ANALYSIS_OBJS) \
	$(PASSES_OBJS) \
	$(FRAMEWORK_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) -shared -o $@ $(LDFLAGS) $^

# -------------------------------------------------------------------
# TEST PIPELINE (generated per-stem)
# -------------------------------------------------------------------

define TEST_RULES
# s     = e.g. dominators/dominators
# name  = e.g. dominators
# pass  = e.g. dominators
# src   = e.g. tests/dominators/dominators.c
# dir   = e.g. build/tests/dominators/dominators

$(eval _s    := $(1))
$(eval _name := $(call test_name,$(1)))
$(eval _pass := $(call test_pass,$(1)))
$(eval _src  := tests/$(1).c)
$(eval _dir  := $(call test_outdir,$(1)))

# STEP 1: .c -> .bc
$(_dir)/$(_name).bc: $(_src)
	@mkdir -p $$(@D)
	clang -MMD -MP -MF $$(@:.bc=.d) -MT $$@ \
		-fno-discard-value-names \
		-Xclang -disable-O0-optnone \
		-O0 \
		-emit-llvm \
		-c $$< \
		-o $$@

# STEP 2: mem2reg
$(_dir)/$(_name)-m2r.bc: $(_dir)/$(_name).bc
	@mkdir -p $$(@D)
	opt -bugpoint-enable-legacy-pm=1 -mem2reg $$< -o $$@

# STEP 3: disassemble IR
$(_dir)/$(_name).ll: $(_dir)/$(_name)-m2r.bc
	@mkdir -p $$(@D)
	llvm-dis $$< -o $$@

# STEP 4: run optimizer pass
$(_dir)/$(_name)-opt.bc: $(_dir)/$(_name)-m2r.bc $(OPTIMIZER_LIBS)
	@mkdir -p $$(@D)
	opt \
		-bugpoint-enable-legacy-pm=1 \
		$(OPTIMIZER_LIBS:%=-load-pass-plugin=%) \
		-passes='$(_pass)' \
		$$< \
		-o $$@ \
		> $(_dir)/$(_name).out

# STEP 5: final IR
$(_dir)/$(_name)-opt.ll: $(_dir)/$(_name)-opt.bc
	@mkdir -p $$(@D)
	llvm-dis $$< -o $$@

endef

$(foreach s,$(TEST_STEMS),$(eval $(call TEST_RULES,$(s))))

# -------------------------------------------------------------------
# DEPENDENCIES
# -------------------------------------------------------------------

DEPFILES := $(shell find $(DEPDIR) -name '*.d' 2>/dev/null)
-include $(DEPFILES)