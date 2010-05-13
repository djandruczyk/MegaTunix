# If you want to build these components, please cd to their respective
# directories and type `make'.
AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

subdirs = include mtxmatheval mtxcommon widgets msloader src gaugedesigner dashdesigner ecu_snapshots Interrogator LookupTables RealtimeMaps RuntimeSliders RuntimeStatus RuntimeText Gui menu category icons desktop po

SUBDIRS =  $(subdirs) 

DIST_SUBDIRS =  $(subdirs)  

DEPDIR = ${subdirs}/.deps

INTLTOOL_FILES = intltool-extract.in \
		intltool-merge.in \
		intltool-update.in

CLEANFILES = *~ core autom4te.cache gmon.out

DISTCLEANFILES = intltool-extract intltool-merge intltool-update po/.intltool-merge-cache

EXTRA_DIST = config.rpath m4/ChangeLog configure CREDITS AUTHORS TODO README INSTALL LICENSE ${INTLTOOL_FILES}

