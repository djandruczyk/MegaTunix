
headers = \
	3d_vetable.h \
	about_gui.h \
	api-versions.h \
	apicheck.h \
	args.h \
	binlogger.h \
	comms_gui.h \
	combo_loader.h \
	combo_mask.h \
	configfile.h \
	conversions.h \
	core_gui.h \
	dashboard.h \
	dataio.h \
	datalogging_gui.h \
	debugging.h \
	defines.h \
	dispatcher.h \
	enums.h \
	firmware.h \
	getfiles.h \
	gui_handlers.h \
	hc08_loader.h \
	helpers.h \
	init.h \
	keybinder.h \
	keyparser.h \
	listmgmt.h \
	loader_common.h \
	locking.h \
	logo.h \
	logviewer_core.h \
	logviewer_events.h \
	logviewer_gui.h \
	lookuptables.h \
	main.h \
	menu_handlers.h \
	mem_mgmt.h \
	mtxloader.h \
	multi_expr_loader.h \
	notifications.h \
	offline.h \
	personalities.h \
	plugin.h \
	plugindefines.h \
	rtv_map_loader.h \
	rtv_processor.h \
	runtime_sliders.h \
	runtime_status.h \
	runtime_text.h \
	s12x_loader.h \
	serialio.h \
	sleep_calib.h \
	stringmatch.h \
	tabloader.h \
	tag_loader.h \
	template.h \
	threads.h \
	timeout_handlers.h \
	vetable_gui.h \
	visibility.h \
	warmwizard_gui.h \
	watches.h \
	widgetmgmt.h \
	winserialio.h \
	winserialbuffer.h \
	winserialcommhelper.h \
	wrappers.h \
	xmlbase.h \
	xmlcomm.h 

version: 
	@if test -f version.h ; then rm version.h ; fi 
	@if test `grep RELEASE ../config.h |cut -f3 -d\ ` -lt 1 ; then if test -d ../.git ; then echo "#define MTX_GIT_HASH \"`git rev-parse HEAD`"\" ; echo "#define MTX_VER_SUFFIX \"SNAPSHOT_`git rev-parse HEAD |cut -b1-6`"\" ; else echo "#define MTX_GIT_HASH \"Not a git release\"" ; echo "#define MTX_VER_SUFFIX \"RELEASE\"" ; fi > version.h ; else echo "#define MTX_GIT_HASH \"Not a git release\"" ; echo "#define MTX_VER_SUFFIX \"RELEASE\"" ; fi >version.h

all: version

EXTRA_DIST=$(headers)

CLEANFILES = version.h

