
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
	dep_loader.c \
	dep_processor.c \
	fileio.c \
	general_gui.c \
	getfiles.c \
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
	rtv_processor.c \
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

#megatunix_LDADD = @PACKAGE_LIBS@  -L../custom-widgets -lmtxcustom -L../mtxmatheval -lmtxmatheval -lfl
megatunix_LDADD = @PACKAGE_LIBS@  -L../mtxmatheval -lmtxmatheval -lfl

megatunix_LDFLAGS =

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README NEWS TODO AUTHORS

