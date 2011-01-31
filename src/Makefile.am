SUBDIRS = plugins

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/widgets \
	-I$(top_srcdir)/mtxmatheval \
	@GTK_CFLAGS@ \
	@GLEXT_CFLAGS@ \
	@LIBGLADE_CFLAGS@ \
	@MEGATUNIX_DEFINES@

bin_PROGRAMS = megatunix

megatunix_SOURCES = \
	Makefile \
	3d_vetable.c \
	about_gui.c \
	apicheck.c \
	args.c \
	binlogger.c \
	comms.c \
	comms_gui.c \
	combo_loader.c \
	conversions.c \
	core_gui.c \
	dashboard.c \
	dataio.c \
	datalogging_gui.c \
	debugging.c \
	dispatcher.c \
	gui_handlers.c \
	helpers.c \
	init.c \
	keybinder.c \
	keyparser.c \
	listmgmt.c \
	locking.c \
	logviewer_core.c \
	logviewer_events.c \
	logviewer_gui.c \
	lookuptables.c \
	main.c \
	menu_handlers.c \
	mem_mgmt.c \
	memory_gui.c \
	mode_select.c \
	multi_expr_loader.c \
	notifications.c \
	offline.c \
	personalities.c \
	plugin.c \
	post_process.c \
	raweditor.c \
	rtv_map_loader.c \
	rtv_processor.c \
	runtime_gui.c \
	runtime_sliders.c \
	runtime_status.c \
	runtime_text.c \
	serialio.c \
	sleep_calib.c \
	stringmatch.c \
	tabloader.c \
	tag_loader.c \
	threads.c \
	timeout_handlers.c \
	vetable_gui.c \
	visibility.c \
	warmwizard_gui.c \
	watches.c \
	widgetmgmt.c \
	winserialio.c \
	wrappers.c \
	xmlcomm.c 

megatunix_DEPENDENCIES = @WIN32_RESOURCES@  ../widgets/libmtxgauge.la ../widgets/libmtxprogress.la ../widgets/libmtxpie.la ../widgets/libmtxstripchart.la ../mtxcommon/libmtxcommon.la ../mtxmatheval/libmtxmatheval.la ../widgets/libmtxcombo_mask.la

resources.o:
	$(WINDRES) -i resources.rc -o resources.o

megatunix_LDADD = @GTK_LIBS@ @GLEXT_LIBS@ @LIBGLADE_LIBS@ ../mtxmatheval/libmtxmatheval.la -lfl ../mtxcommon/libmtxcommon.la ../widgets/libmtxprogress.la ../widgets/libmtxpie.la ../widgets/libmtxstripchart.la ../widgets/libmtxgauge.la ../widgets/libmtxcombo_mask.la @EXTRA_LIBS@ @WIN32_RESOURCES@ @PROF_FLAGS@

megatunix_LDFLAGS = $(LTLIBINTL)

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS = $(COMMON_FLAGS) -Wall 

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README NEWS TODO AUTHORS mtx.ico template.c

