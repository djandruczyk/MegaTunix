# If you want to build these components, please cd to their respective
# directories and type `make'.
AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

subdirs = include mtxmatheval mtxcommon widgets msloader src gaugedesigner dashdesigner ecu_snapshots Interrogator LookupTables RealtimeMaps RuntimeSliders RuntimeStatus RuntimeText Gui menu category icons desktop

SUBDIRS = $(subdirs) 

DIST_SUBDIRS = $(subdirs)  

DEPDIR = ${subdirs}/.deps

CLEANFILES = *~ core autom4te.cache gmon.out

EXTRA_DIST =  configure CREDITS AUTHORS TODO README INSTALL LICENSE megatunix.spec

