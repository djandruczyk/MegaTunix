
INCLUDES = @PACKAGE_CFLAGS@ -DGTK_DISABLE_COMPAT_H  -D_MAJOR_=@MEGATUNIX_LIN_MAJOR_VERSION@  -D_MINOR_=@MEGATUNIX_LIN_MINOR_VERSION@ -D_MICRO_=@MEGATUNIX_LIN_MICRO_VERSION@ -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED

bin_PROGRAMS = megatunix

megatunix_SOURCES = \
	Makefile \
	c_gauge.h \
	configfile.h \
	constants.h \
	datalogging.h \
	defines.h \
	globals.h \
	lookuptables.h \
	protos.h \
	runtime_gui.h \
	logo.h \
	about_gui.c \
	c_gauge.c \
	comms_gui.c \
	configfile.c \
	constants_gui.c \
	conversions.c \
	core_gui.c \
	dataio.c \
	datalogging_gui.c \
	enrichments_gui.c \
	general_gui.c \
	gui_handlers.c \
	ignition.c \
	init.c \
	lowlevel_gui.c \
	main.c \
	ms_constants.c \
	notifications.c \
	req_fuel.c \
	runtime_gui.c \
	serialio.c \
	threads.c \
	tools_gui.c \
	tuning_gui.c \
	vetable_gui.c 

megatunix_LDADD = @PACKAGE_LIBS@

LIBS= -lpthread

COMMON_FLAGS = @PFLAGS@ @DFLAGS@ 

CFLAGS += $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core .deps/* gmon.out nohup.out -r .deps

EXTRA_DIST = README NEWS TODO AUTHORS
