
INCLUDES = \
	-I$(top_srcdir)/include \
	@PACKAGE_CFLAGS@ \
	@MEGATUNIX_DEFINES@

bin_PROGRAMS = megatunix

megatunix_SOURCES = \
	Makefile \
	3d_vetable.c \
	about_gui.c \
	comms.c \
	comms_gui.c \
	configfile.c \
	conversions.c \
	core_gui.c \
	dataio.c \
	datalogging_gui.c \
	debugging.c \
	dep_loader.c \
	dep_processor.c \
	dispatcher.c \
	fileio.c \
	general_gui.c \
	getfiles.c \
	gui_handlers.c \
	init.c \
	interrogate.c \
	keybinder.c \
	keyparser.c \
	listmgmt.c \
	logviewer_core.c \
	logviewer_events.c \
	logviewer_gui.c \
	lookuptables.c \
	main.c \
	memory_gui.c \
	mode_select.c \
	notifications.c \
	offline.c \
	post_process.c \
	raweditor.c \
	req_fuel.c \
	rtv_map_loader.c \
	rtv_processor.c \
	runtime_gui.c \
	runtime_sliders.c \
	runtime_status.c \
	serialio.c \
	stringmatch.c \
	tabloader.c \
	tag_loader.c \
	threads.c \
	timeout_handlers.c \
	tuning_gui.c \
	user_outputs.c \
	vetable_gui.c \
	vex_support.c \
	warmwizard_gui.c \
	widgetmgmt.c \
	winserialio.c

#megatunix_LDADD = @PACKAGE_LIBS@  -L../custom-widgets -lmtxcustom -L../mtxmatheval -lmtxmatheval -lfl
megatunix_LDADD = @PACKAGE_LIBS@ ../mtxmatheval/libmtxmatheval.la -lfl  @EXTRA_LIBS@


megatunix_LDFLAGS =

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README NEWS TODO AUTHORS

