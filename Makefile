CC = g++
CFLAGS = -Wall -O3
OUTDIR = bin

SRCS = precomputed_bitmask.cpp precomputed_bitmask_custom.cpp precomputed_bitmask_full_custom.cpp precomputed_bitmask_full_custom_length_sinks.cpp precomputed_bitmask_unroll.cpp precomputed_bitmask_unroll_custom.cpp sharpsat.cpp

EXECS = $(SRCS:%.cpp=$(OUTDIR)/%.exe)

all: $(EXECS)
	@echo Compiled all executables

$(OUTDIR):
	mkdir $(OUTDIR)

$(OUTDIR)/%.exe: $(OUTDIR) $(SRCS)
	$(CC) $(CFLAGS) $(MAINSRC) $(@:$(OUTDIR)/%.exe=%.cpp) -o $@
	@echo Compiled $@
