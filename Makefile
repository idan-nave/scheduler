# Idan Nave, HRD32
# A Generic makefile to manage the project's build process
PROJ_NAME = $(notdir $(shell pwd))

###############################################################################
####################### Project Dirs, Files, Dependencies #####################
###############################################################################

# Proj dependencies lists (PROJ_DEPS is read from a file)
PROJ_DEPS 	:= $(shell cat ./lib/lib_dep)
DEP_DIRS 	= $(foreach Proj,$(PROJ_DEPS),../$(Proj))
DEP_MAKES	= $(foreach Dir,$(DEP_DIRS), $(Dir)/Makefile)
DEP_LIBS	= $(foreach Dir,$(DEP_DIRS), $(wildcard $(Dir)/lib/*.so))
DEP_INCS 	= $(foreach Dir,$(DEP_DIRS), $(Dir)/include)

# Proj Directories
TREE = $(INC_DIRS) $(SRC_DIRS) $(OBJ_DIR) $(LIB_DIR) $(TST_DIR) $(DBG_DIR) $(REL_DIR)
INC_DIRS 	=  $(API_INC_DIR) $(DEP_INCS)
API_INC_DIR = ./include
API_SRC_DIR = ./src
TST_SRC_DIR = ./test
SRC_DIRS 	=  $(API_SRC_DIR) $(TST_SRC_DIR)
OBJ_DIR 	= ./obj
LIB_DIR 	= ./lib
BIN_DIR		= ./bin
DBG_DIR 	= $(BIN_DIR)/debug
REL_DIR 	= $(BIN_DIR)/release

# Proj Files (auto listing using wildcards)
API_SRCS 	= $(foreach C,$(API_SRC_DIR),$(wildcard $(C)/*.c))
TST_SRCS 	= $(foreach C,$(TST_SRC_DIR),$(wildcard $(C)/*.c))
SRCS 		= $(API_SRCS) $(TST_SRCS)
INCS 		= $(foreach Dir,$(INC_DIRS),$(wildcard $(Dir)/*.h))

# Pattern substitution on a list of SRCS
API_OBJS 	= $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(API_SRCS)))
TST_OBJS 	= $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(TST_SRCS)))
OBJS 		= $(API_OBJS) $(TST_OBJS)
LIB 		= $(LIB_DIR)/lib$(PROJ_NAME).so
BIN 		= $(REL_DIR)/$(PROJ_NAME).out
TST_BIN 	= $(DBG_DIR)/$(PROJ_NAME)_dbg.out

# Transformation applied to the $(OBJS) lists. It means "replace the .o extension with .d for each item in the $(OBJS) lists."
DEPS 		= $(API_OBJS:.o=.d) $(TST_OBJS:.o=.d) 

###############################################################################
########################## Project Building Parameters ########################
###############################################################################

# compilation params
CC 				= gcc
ERR_FLAGS 		= -pedantic-errors -Wall -Wextra -ansi
#enable debug info
DBG_COMP_FLAGS 	= -g
#disable debugging info & asserts
REL_COMP_FLAGS 	= -DNDEBUG
# encode make rules for the .h dependencies with -MMD	
INC_FLAGS 		= $(foreach Dir,$(INC_DIRS),-I$(Dir)) -MMD

# linking params
DEP_LDFLAGS 	= $(foreach Lib,$(DEP_LIBS),-L$(dir $(Lib)) -l$(patsubst lib%.so,%,$(notdir $(Lib))) -Wl,-rpath,$(dir $(Lib)))
API_LDFLAGS 	= -L$(dir $(LIB)) -l$(PROJ_NAME) -Wl,-rpath,$(dir $(LIB))

# Final Build Flags
DBG_CFLAGS 		= $(DBG_COMP_FLAGS) $(ERR_FLAGS) $(INC_FLAGS)
TST_CFLAGS 		= $(DBG_COMP_FLAGS) $(ERR_FLAGS) -I$(API_INC_DIR) -MMD
REL_CFLAGS 		= $(REL_COMP_FLAGS) $(ERR_FLAGS) $(INC_FLAGS)
#no optimization, non-PIE executable
DBG_LDFLAGS 	= -no-pie -O0
#Max optimization for release
REL_LDFLAGS 	= -O3

###############################################################################
############################ Buildig Process Targets ##########################
###############################################################################
all: clean build_deps validate_deps build_objs build_shared_lib build_bin #test distribute #enable_ASLR
############################## Dependencies Targets ###########################

# Run make for each dependency Makefile
build_deps: $(DEP_MAKES)
	@echo "inside $(DEP_DIRS) Building $(words $(DEP_DIRS)) Dependency directories: $(DEP_DIRS)"
	@for Makefile in $(DEP_MAKES); do \
		echo "Building $$Makefile"; \
		$(MAKE) -C $$(dirname $$Makefile); \
	done

# Validate that all dependencies are built
validate_deps: build_deps
	@for lib in $(DEP_LIBS); do \
		if [ ! -e "$$lib" ]; then \
			echo "Error: $$lib not found. No Dependencies built!"; \
			exit 1; \
		fi; \
		done
-include $(DEPS)

################################# Project Targets #############################

build_objs: $(OBJS)
./obj/$(PROJ_NAME).o: ./src/$(PROJ_NAME).c
	@echo "Building: $@"
	$(CC) -c -fpic $(DBG_CFLAGS) -o $@ $< $(DEP_LDFLAGS)
./obj/$(PROJ_NAME)_test.o: ./test/$(PROJ_NAME)_test.c
	@echo "Building: $@"
	$(CC) -c -fpic $(DBG_CFLAGS) -o $@ $< $(DEP_LDFLAGS)
# ./obj/dll.o: ./src/dll.c
# ./obj/dll_test.o: ./test/dll_test.c
# $(OBJ_DIR)/%.o: $(API_SRC_DIR)/%.c
# $(OBJ_DIR)/%.o: $(TST_SRC_DIR)/%.c

build_shared_lib: $(LIB)
$(LIB): $(API_OBJS)
	@echo "Building: $@"
	$(CC) -shared -o $@ $^ $(DEP_LDFLAGS)

build_bin: $(TST_BIN) $(BIN) 
$(TST_BIN): $(TST_OBJS) $(LIB)
	@echo "Building: $@"
	$(CC) $(DBG_CFLAGS) $(DBG_LDFLAGS) -o $@ $< $(API_LDFLAGS)
$(BIN): $(TST_OBJS) $(LIB)
	@echo "Building: $@"
	$(CC) $(REL_CFLAGS) $(REL_LDFLAGS) -o $@ $< $(API_LDFLAGS)

test: $(TST_BIN)
	@echo "Running: $@"
	$(TST_BIN)

# non-targetfile rules
.PHONY: all clean distribute diff

# perform valgrind analysis
valg:
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes $(TST_BIN)

# Clean up makable files
clean: tree
	rm -rf $(OBJ_DIR)/*.o
# .d files will be kept for optimized future build	
	rm -rf $(REL_DIR)/*.out
	rm -rf $(DBG_DIR)/*.out
	rm -rf $(LIB_DIR)/*.a
	rm -rf $(LIB_DIR)/*.so

# Gen. directories tree
tree:
	@mkdir -p $(TREE)
# -p flag ensures that directories are created as needed, without causing an error if they already exist.

# View code files
view:
	@for File in $(SRCS) $(INCS); do \
	    echo "Opening $$File"; \
	    open "$$File"; \
	done 
# $$File tells Make to pass the value of the File variable to the shell without interpreting it as a Makefile variable

disable_ASLR:
	echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
enable_ASLR:
	echo 2 | sudo tee /proc/sys/kernel/randomize_va_space


# prepare an archive of source code
distribute:
	rm -rf ./*.tgz
	tar zcvf $(PROJ_NAME)_dist.tgz $(API_INC_DIR) $(API_SRC_DIR) $(TST_SRC_DIR) $(LIB_DIR) $(BIN_DIR)

# prints git's changelog (@ silences printing if needed)
changelog:
	$(info The status of the repository, and the volume of per-file changes:)
	@git status
	git diff --stat




###############################################################################
############################## Pattern Substitution ###########################
###############################################################################

# OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SRCS)))

# OBJS: This is a variable that you are defining. It will store a list of object files that need to be created.
# $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SRCS))): This is the value assigned to OBJS, and it's created using several Makefile functions.
# patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SRCS)) is a series of Makefile functions applied to $(SRCS).
# $(notdir $(SRCS)): This function takes the list of source files $(SRCS) and removes the directory portion, leaving only the filenames. For example, if $(SRCS) contains ./src/file1.c ./test/file2.c, $(notdir $(SRCS)) becomes file1.c file2.c.
# $(patsubst %.c,$(OBJ_DIR)/%.o,...): This function performs a pattern substitution on each of the filenames obtained from $(notdir $(SRCS)). Specifically:
# %.c: This is a pattern that matches any string ending with .c, which represents a C source file.
# $(OBJ_DIR)/%.o: This is the replacement pattern. It says that for every string matching %.c, replace it with $(OBJ_DIR)/%.o, where $(OBJ_DIR) is the directory where object files should be stored. This effectively changes the file extension from .c to .o and places it in the OBJ_DIR.

# So, the final result of $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SRCS))) is a list of object file names with their respective paths, ready to be used as prerequisites or targets in your Makefile rules.


###############################################################################
################################ Dependency flags #############################
###############################################################################

#Dependency flags, such as -MM and -MD, are used in Makefiles to automatically generate dependency rules for your source files. These flags are especially useful in large projects with many source files and header files. Here's why you might need them:
#Automatic Dependency Tracking: When you compile a C or C++ source file, it often depends on header files. If a header file changes, the corresponding source file should be recompiled. Manually tracking these dependencies can be error-prone and tedious. Dependency flags automate this process.
#Avoiding Unnecessary Recompilation: Without dependency tracking, changing a header file can require recompiling many source files even if they didn't directly include the modified header. Dependency flags ensure that only the affected source files are recompiled, saving compilation time.
#Maintainability: In a large project, managing dependencies manually is challenging. Dependency flags generate and maintain the dependency information for you, making it easier to keep your build system up to date.
#Parallel Builds: Dependency information can also help make parallel builds more efficient because it allows the build system to determine which files can be compiled concurrently without causing conflicts.

# -M: Generates a list of dependencies (header files) for the source file. This list is suitable for Makefile inclusion.
# -MM : tells the compiler to generate dependency information but without including system header files as dependencies. It only generates dependencies for user header files and source files. This is useful when you want to generate Makefile dependencies for your project's code without including dependencies on system or standard library headers, which are unlikely to change.
# -MT : allows you to specify the target (output) for the generated dependency file. It lets you control where the generated .d file will be placed and what its name will be. This is useful when you want to place the generated dependency files in a specific directory or give them a particular naming convention.
# -MP : generates phony targets for each prerequisite (dependency) in a Makefile rule. This is useful to prevent issues when a header file is deleted or renamed, as it avoids Makefile errors related to missing dependencies.
# -MD : Generates a Makefile rule for each dependency, including both source and header files. It creates a .d file for each source file, listing the source file's dependencies (header files, for example).
# -MMD: Similar to -MD, but excludes system header files.



###############################################################################
################################ Dependency files #############################
###############################################################################

# -include $(DEPS): includes the dependency files ($(DEPS)) in the makefile. Dependency files are typically generated by the compiler when using the -MD flag and contain information about which headers each source file depends on. This helps Make understand the dependencies between files and rebuild them if necessary.

# The .d files generated by the line DEPS = $(OBJS:.o=.d) will be saved in the same directory as the object files specified in the $(OBJS) variable. In this case, if $(OBJS) contains object files located in the $(OBJ_DIR) directory, the corresponding .d files will also be saved in the $(OBJ_DIR) directory.

# If your #include directives for header files are not consistently or correctly specified in your source files, using the -MMD flag alone may not be sufficient to accurately track header file dependencies. -MMD relies on the #include directives to determine dependencies, so if they are inconsistent or incorrect, it may miss some dependencies.

# In such cases, you can still use your INC_FLAGS to specify the include paths explicitly. This ensures that the compiler knows where to look for header files, regardless of how the #include directives are written in your source files. Combining -MMD with explicit -I flags in INC_FLAGS is a good practice to ensure accurate dependency tracking, especially when dealing with inconsistent or incorrect #include directives.

# So, you can continue using both -MMD and INC_FLAGS to cover all bases and ensure that header file dependencies are correctly tracked.