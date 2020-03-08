VERBOSE = FALSE
PROJDIR := $(realpath $(CURDIR))
SOURCEDIR := $(PROJDIR)/src/
BUILDDIR := $(PROJDIR)/build/
TARGET = server.out
INCLUDES = $(PROJDIR)/include/
SOURCES = $(wildcard $(SOURCEDIR)*.c)
VPATH = $(SOURCES)
OBJS := $(subst $(SOURCEDIR),$(BUILDDIR),$(SOURCES:.c=.o))
DEPS = $(OBJS:.o=.d)
CC = gcc
LINKLIB = -lpthread
ECHO = @echo
ifeq ($(OS),Windows_NT)
	RM = del /F /Q
	RMDIR = -RMDIR /S /Q
	MKDIR = -mkdir
	ERRIGNORE = 2>NUL || true
else
	RM = rm -f
	RMDIR = rm -rf
	MKDIR = mkdir -p
	ERRIGNORE = 2>/dev/null
endif

# PSEP = $(strip $(SEP))

ifeq ($(VERBOSE),TRUE)
	HIDE =
else
	HIDE = @
	MAKEFLAGS += --no-print-directory
endif

.PHONY: all clean directories re rebuild build link

all: run
run:build link
	./$(TARGET)
build: directories $(OBJS)
link: $(TARGET)
re: rebuild
rebuild:
	@ $(MAKE) clean
	@ $(MAKE) all
$(TARGET): $(OBJS)
	@echo Linking $@
	$(HIDE)$(CC) $(OBJS) $(LINKLIB) -o $(TARGET)

$(BUILDDIR)%.o: $(SOURCEDIR)%.c directories
	$(ECHO) BUILDING $@
	$(HIDE)$(CC) -c -g -Wall -Wextra $(LINKLIB) -I$(INCLUDES) -o $@ $< -MMD
# Include dependencies
-include $(DEPS)

$(basename $(TARGET)): $(TARGET)

%.o: $(SOURCEDIR)%.c directories
	@echo Building $(BUILDDIR)$@
	$(HIDE)$(CC) -c -g -Wall -Wextra $(LINKLIB) -I$(INCLUDES) -o $(BUILDDIR)$@ $< -MMD

directories:
	$(HIDE)$(MKDIR) $(BUILDDIR) $(ERRIGNORE)

# Remove all objects, dependencies and executable files generated during the build
clean:
	$(HIDE)$(RMDIR) $(BUILDDIR) $(ERRIGNORE)
	$(HIDE)$(RM) $(TARGET) $(ERRIGNORE)
	@echo Cleaning done
