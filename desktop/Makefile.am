desktopdir = $(datadir)/desktop-directories/$(DESKTOP_DATA_DIR)

desktop_files = 	Automotive.directory 
		       	
desktop_in_files =	Automotive.directory.in

desktop_DATA = $(desktop_files:.directory.in=.directory)	
@INTLTOOL_DIRECTORY_RULE@

EXTRA_DIST = $(desktop_DATA)

