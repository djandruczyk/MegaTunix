
INCLUDES = \
	-I$(top_srcdir)/include \
	@PACKAGE_CFLAGS@ \
	-DGTK_DISABLE_COMPAT_H \
	-D_MAJOR_=@MEGATUNIX_MAJOR_VERSION@ \
	-D_MINOR_=@MEGATUNIX_MINOR_VERSION@ \
	-D_MICRO_=@MEGATUNIX_MICRO_VERSION@ \
	-DGDK_DISABLE_DEPRECATED 

bin_PROGRAMS = megatunix

megatunix_SOURCES = \
	Makefile \
	3d_vetable.c \
	about_gui.c \
	bitfield_handlers.c \
	comms_gui.c \
	configfile.c \
	constants_gui.c \
	conversions.c \
	core_gui.c \
	dataio.c \
	datalogging_gui.c \
	dt_params_gui.c \
	eng_vitals_gui.c \
	enrichments_gui.c \
	fileio.c \
	general_gui.c \
	gui_handlers.c \
	ignition_gui.c \
	init.c \
	interrogate.c \
	logviewer_core.c \
	logviewer_gui.c \
	lowlevel_gui.c \
	main.c \
	mode_select.c \
	notifications.c \
	post_process.c \
	req_fuel.c \
	runtime_controls.c \
	runtime_gui.c \
	serialio.c \
	sparktable_gui.c \
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

