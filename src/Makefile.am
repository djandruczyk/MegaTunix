
INCLUDES = \
	-I$(top_srcdir)/include \
	@PACKAGE_CFLAGS@ \
	@MEGATUNIX_DEFINES@

bin_PROGRAMS = megatunix

megatunix_SOURCES = \
	Makefile \
	3d_vetable.c \
	about_gui.c \
	bitfield_handlers.c \
	comms_gui.c \
	configfile.c \
	conversions.c \
	core_gui.c \
	dataio.c \
	datalogging_gui.c \
	debugging.c \
	fileio.c \
	general_gui.c \
	gui_handlers.c \
	init.c \
	interrogate.c \
	logviewer_core.c \
	logviewer_gui.c \
	lookuptables.c \
	main.c \
	memory_gui.c \
	mode_select.c \
	notifications.c \
	post_process.c \
	req_fuel.c \
	rtv_map_loader.c \
	runtime_controls.c \
	runtime_gui.c \
	serialio.c \
	stringmatch.c \
	tabloader.c \
	threads.c \
	timeout_handlers.c \
	tools_gui.c \
	tuning_gui.c \
	vetable_gui.c \
	vex_support.c \
	warmwizard_gui.c

megatunix_LDADD = @PACKAGE_LIBS@  

megatunix_LDFLAGS =

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

CFLAGS += $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README NEWS TODO AUTHORS

