#################################
#    2012-11-06 zhang_ming_xing
#################################
.PHONY:	clean

MKDIR = mkdir
RM = rm
RMFLAGS = -rf

CC=gcc
AR = ar
ARFLAGS = rs
SRCEXTS = .cpp .c

#################################
#          target
#################################
EXE = tcp-usbip
#LIB = 

#################################
#       direction
#################################
DIR_EXES = ./
DIR_LIBS = 
DIR_OBJS = objs
DIR_DEPS = deps
DIR_SUBS_OBJS = 
DIR_SUBS_DEPS = 

#################################
#   include / lib 
#################################
INCLUDE_DIRS = ./ /include/linux 
DIR_LINK_LIBS = 
LINK_LIBS = pthread sysfs 
DEF_LISTS = FRONVIEW DEBUG
#################################
# sub direction
#################################
SUB_DIRS=

#################################
#   clean target 
#################################
RMS = $(DIR_OBJS) $(DIR_DEPS) 

#################################
#      src
#################################
SRCS = $(wildcard $(addprefix *, $(SRCEXTS))) #root 
ifneq ("$(SUB_DIRS)", "")
SRCS += $(foreach sub_d, $(SUB_DIRS), $(wildcard $(addprefix $(sub_d)/*, $(SRCEXTS))))  # src == root + subdir
endif
SRCS_C = $(filter %.c,$(SRCS))                                                  
SRCS_CPP = $(filter %.cpp,$(SRCS))

#################################
#    obj/dep
#################################
OBJS = $(SRCS_C:.c=.o) $(SRCS_CPP:.cpp=.o)
OBJS := $(addprefix $(DIR_OBJS)/, $(OBJS))
DEPS = $(SRCS_C:.c=.dep) $(SRCS_CPP:.cpp=.dep)
DEPS := $(addprefix $(DIR_DEPS)/, $(DEPS))

#################################
#   update exe/lib direction
#################################
ifneq ("$(EXE)", "")
EXE := $(addprefix $(DIR_EXES)/, $(EXE))
RMS += $(EXE)
endif

ifneq ("$(LIB)", "")
LIB := $(addprefix $(DIR_LIBS)/, $(LIB))
RMS += $(LIB)
endif

#################################
# update obj/dep sub direction
#################################
ifneq ("$(SUB_DIRS)", "")
DIR_SUBS_OBJS = $(foreach sub_d, $(SUB_DIRS), $(addprefix objs/, $(sub_d)))  
DIR_SUBS_DEPS = $(foreach sub_d, $(SUB_DIRS), $(addprefix deps/, $(sub_d)))  
endif

#################################
# update dep dir
#################################
ifeq ("$(wildcard $(DIR_OBJS))", "")
DEP_DIR_OBJS := $(DIR_OBJS) $(DIR_SUBS_OBJS)
endif
ifeq ("$(wildcard $(DIR_EXES))", "")
DEP_DIR_EXES := $(DIR_EXES) 
endif
ifeq ("$(wildcard $(DIR_LIBS))", "")
DEP_DIR_LIBS := $(DIR_LIBS) 
endif
ifeq ("$(wildcard $(DIR_DEPS))", "")
DEP_DIR_DEPS := $(DIR_DEPS) $(DIR_SUBS_DEPS)
endif

#################################
#  all dir
#################################
DIRS = $(DIR_OBJS) $(DIR_EXES) $(DIR_DEPS) $(DIR_SUBS_OBJS) $(DIR_SUBS_DEPS) $(DIR_LIBS)

#################################
#   target to build
#################################
all: $(EXE) $(LIB)

#################################
# include deps
#################################
ifneq ($(MAKECMDGOALS), clean)
include $(DEPS)
endif

#################################
#  include/lib handle
#################################
ifneq ($(INCLUDE_DIRS), "")
INCLUDE_DIRS := $(strip $(INCLUDE_DIRS))
INCLUDE_DIRS := $(addprefix -I, $(INCLUDE_DIRS))
endif 

ifneq ($(LINK_LIBS), "")
LINK_LIBS := $(strip $(LINK_LIBS))
LINK_LIBS := $(addprefix -l, $(LINK_LIBS))
endif

ifneq ($(DEF_LISTS), "")
DEF_LISTS := $(strip $(DEF_LISTS))
DEF_LISTS := $(addprefix -D, $(DEF_LISTS))
endif

ifneq ($(DIR_LINK_LIBS), "")
DIR_LINK_LIBS := $(strip $(DIR_LINK_LIBS))
DIR_LINK_LIBS := $(addprefix -L, $(DIR_LINK_LIBS))
endif

#################################
#   targets defined
#################################
$(DIRS):
	$(MKDIR) $@
$(EXE):$(DEP_DIR_EXES) $(OBJS)
	@echo $(DEF_LISTS)
	$(CC) $(DIR_LINK_LIBS) -o $@ $(filter %.o, $^) $(LINK_LIBS)
	@echo "**************************************"                                                                                                                 
	@echo "@_@ Successfully build exe $@"
	@echo "**************************************"
$(LIB): $(DEP_DIR_LIBS) $(OBJS)	
	$(AR) $(ARFLAGS) $@ $(filter %.o, $^)
	@echo "**************************************"
	@echo "*_* Successfully Create Lib $@"
	@echo "**************************************"
$(DIR_OBJS)/%.o: $(DEP_DIR_OBJS) %.cpp
	$(CC) $(DEF_LISTS) $(INCLUDE_DIRS) -o $@ -c $(filter %.cpp, $^) 
$(DIR_OBJS)/%.o: $(DEP_DIR_OBJS) %.c
	@echo "INCLUDE_DIRS = $(INCLUDE_DIRS)"
	$(CC) $(DEF_LISTS) $(INCLUDE_DIRS) -o $@ -c $(filter %.c, $^)
$(DIR_DEPS)/%.dep:$(DEP_DIR_DEPS) $(SUB_DIRS) %.c
	@echo "Creating $@ ..."
	@set -e; \
	$(RM) $(RMFLAGS) $@.tmp; \
	$(CC) $(INCLUDE_DIRS) -E -MM $(filter %.c, $^) >$@.tmp; \
	sed 's,\(.*\)\.o[ :]*,objs/\1.o: ,g' < $@.tmp >$@ ;\
	$(RM) $(RMFLAGS) $@.tmp; 
$(DIR_DEPS)/%.dep:$(DEP_DIR_DEPS) $(SUB_DIRS) %.cpp
	@echo "Creating $@ ..."
	@set -e; \
	$(RM) $(RMFLAGS) $@.tmp; \
	$(CC) $(INCLUDE_DIRS) -E -MM $(filter %.c, $^) >$@.tmp; \
	sed 's,\(.*\)\.o[ :]*,objs/\1.o: ,g' < $@.tmp >$@ ;\
	$(RM) $(RMFLAGS) $@.tmp; 	

clean:
	$(RM) $(RMFLAGS) $(RMS)

