menudir = $(datadir)/applications/$(MENU_DATA_DIR)

menu_files = 	\
		megatunix.desktop \
		dashdesigner.desktop \
		gaugedesigner.desktop \
		mtxloader.desktop

menu_in_files = 	\
		megatunix.desktop.in \
		dashdesigner.desktop.in \
		gaugedesigner.desktop.in \
		mtxloader.desktop.in

menu_DATA = $(menu_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = $(menu_in_files)

CLEANFILES = *.h $(menu_files)
