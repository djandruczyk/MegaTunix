
INCLUDES = @GTK_CFLAGS@ -DGTK_DISABLE_COMPAT_H  -D_MAJOR_=@MEGASQUIRT_LIN_MAJOR_VERSION@  -D_MINOR_=@MEGASQUIRT_LIN_MINOR_VERSION@ -D_MICRO_=@MEGASQUIRT_LIN_MICRO_VERSION@

bin_PROGRAMS = megasquirt

megasquirt_SOURCES = \
	Makefile \
	globals.h \
	defines.h \
	protos.h \
	lookuptables.h \
	core_gui.c \
	gui_handlers.c \
	about_gui.c \
	comms_gui.c \
	main.c \
	serialio.c \
	dataio.c \
	threads.c \
	configfile.c \
	init.c \
	constants_gui.c \
	runtime_gui.c \
	enrichments_gui.c \
	vetable_gui.c \
	tuning_gui.c \
	tools_gui.c \
	general_gui.c \
	datalogging_gui.c \
	ms_constants.c

megasquirt_LDADD = @GTK_LIBS@ @EXTRA_LIBS@
#megasquirt_LDADD = @EXTRA_LIBS@

LIBS= -lpthread

COMMON_FLAGS = @PFLAGS@ @DFLAGS@ 

CFLAGS += $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core .deps/* gmon.out nohup.out -r .deps

EXTRA_DIST = README NEWS TODO AUTHORS
