#-----------------------------------------------------------------------------
# Makefile
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Choose a compiler and its options
#--------------------------------------------------------------------------
CC   = gcc -std=gnu99	
OPTS = -Og -Wall -Werror -Wno-error=unused-variable -Wno-error=unused-function -D_FORTIFY_SOURCE=2 -pedantic
DEBUG = -g						# -g for GDB debugging

#--------------------------------------------------------------------
# Build Environment
#--------------------------------------------------------------------
SRCDIR=./src
OBJDIR=./obj
INCDIR=./inc
LIBDIR=./lib
BINDIR=./bin
METRICSDIR=./metrics

#--------------------------------------------------------------------
# Build Files
#--------------------------------------------------------------------
SRCS=$(wildcard $(SRCDIR)/*.c)
HDRS=$(wildcard $(INCDOR)/*.h)

#--------------------------------------------------------------------
# Compiler Options
#--------------------------------------------------------------------
INCLUDE=$(addprefix -I,$(INCDIR))
LIBRARY=$(addprefix -L,$(OBJDIR))
SRCOBJS=${SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o}
OBJS=$(OBJDIR)/croquette.o 
CFLAGS=$(OPTS) $(INCLUDE) $(LIBRARY) $(DEBUG)

#--------------------------------------------------------------------
# Build Recipies for the Executables (binary)
#--------------------------------------------------------------------
TARGET = $(OBJS)

all: $(TARGET) 
test: $(BINDIR)/croquette_test

lib: $(OBJS)
	ar rcs $(LIBDIR)/libcroquette.a $^

# Links the object files to create the target binary
$(BINDIR)/croquette_test: $(OBJDIR)/croquette_test.o $(OBJS) $(HDRS) $(INCDIR)
	$(CC) ${CFLAGS} -o $@ $(SRCOBJS) 

$(OBJDIR)/%.o: $(SRCDIR)/%.c 
	$(CC) $(CFLAGS) -c -o $@ $<

# Runs Croquette Self-Test
run_ht: 
	@echo "Initializing test environment."
	@rm -f $(OBJS)/*.o
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/croquette.o $(SRCDIR)/croquette.c 
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/croquette_test.o $(SRCDIR)/croquette_test.c
	$(CC) $(CFLAGS) -o $(BINDIR)/croquette_test $(OBJDIR)/croquette_test.o $(OBJDIR)/croquette.o 
	$(BINDIR)/croquette_test
	@rm -f $(BINDIR)/* $(OBJDIR)/* $(METRICSDIR)/*
	@echo "Cleaning up test environment."

# Runs a Croquette Memory Self-Test
run_htm: 
	@echo "Initializing test environment."
	@rm -f $(OBJS)/*.o
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/croquette.o $(SRCDIR)/croquette.c 
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/croquette_test.o $(SRCDIR)/croquette_test.c
	$(CC) $(CFLAGS) -o $(BINDIR)/croquette_test $(OBJDIR)/croquette_test.o $(OBJDIR)/croquette.o 
	@valgrind -s --leak-check=full --show-leak-kinds=all $(BINDIR)/croquette_test
	@rm -f $(BINDIR)/* $(OBJDIR)/* $(METRICSDIR)/*
	@echo "Cleaning up test environment."

# Runs a Croquette Self-Test with Coverage Checks
run_htc: 
	@echo "Initializing test environment."
	@rm -f $(OBJS)/*.o
	$(CC) $(CFLAGS) --coverage -c -o $(OBJDIR)/croquette.o $(SRCDIR)/croquette.c
	$(CC) $(CFLAGS) --coverage -c -o $(OBJDIR)/croquette_test.o $(SRCDIR)/croquette_test.c
	$(CC) $(CFLAGS) --coverage -o $(BINDIR)/croquette_test $(OBJDIR)/croquette_test.o $(OBJDIR)/croquette.o 
	@$(BINDIR)/croquette_test
	@gcov $(OBJDIR)/croquette.o > $(METRICSDIR)/cov.out; vim $(METRICSDIR)/cov.out
	@mv *.gcov $(METRICSDIR)/.
	@vim $(METRICSDIR)/croquette.c.gcov
	@rm -f $(BINDIR)/* $(OBJDIR)/* $(METRICSDIR)/*
	@echo "Cleaning up test environment."

# Runs a Croquette Self-Test with Coverage Checks
run_htp: 
	@echo "Initializing test environment."
	@rm -f $(OBJS)/*.o
	$(CC) $(CFLAGS) -pg -c -o $(OBJDIR)/croquette.o $(SRCDIR)/croquette.c 
	$(CC) $(CFLAGS) -pg -c -o $(OBJDIR)/croquette_test.o $(SRCDIR)/croquette_test.c
	$(CC) $(CFLAGS) -pg -o $(BINDIR)/croquette_test $(OBJDIR)/croquette_test.o $(OBJDIR)/croquette.o 
	@$(BINDIR)/croquette_test
	@gprof $(BINDIR)/croquette_test > $(METRICSDIR)/croquette_test.prof
	@mv gmon.out $(METRICSDIR)/.
	@vim $(METRICSDIR)/croquette_test.prof
	@rm -f $(BINDIR)/* $(OBJDIR)/* $(METRICSDIR)/*
	@echo "Cleaning up test environment."

#--------------------------------------------------------------------
# Cleans the binaries
#--------------------------------------------------------------------
clean:
	@rm -f $(OBJS) $(SRCOBJS) $(TARGET) $(HELPER_TARGETS)
	@rm -f $(BINDIR)/* $(OBJDIR)/* $(METRICSDIR)/* $(LIBDIR)/*.a
	@echo "Environment Clean"
