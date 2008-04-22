# If you want to build these components, please cd to their respective
# directories and type `make'.
AUTOMAKE_OPTIONS = foreign

subdirs = include mtxmatheval mtxcommon widgets src gaugedesigner dashdesigner ecu_snapshots Interrogator LookupTables RealtimeMaps RuntimeSliders RuntimeStatus RuntimeText Gui

SUBDIRS = $(subdirs) 

DIST_SUBDIRS = $(subdirs)  

DEPDIR = ${subdirs}/.deps

CLEANFILES = *~ core -rf autom4te.cache gmon.out

EXTRA_DIST =  configure CREDITS AUTHORS TODO README INSTALL LICENSE megatunix.spec

