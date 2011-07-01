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


doc:
	@doxygen

check-gettext:
	@if test x$(USE_NLS) != "xyes" ; then echo "Missing gettext. Rerun configure and check for" \
	"'checking whether to use NLS... yes'!" ; exit 1 ; fi

update-po: check-gettext
	(if [ -f $(srcdir)/po/POTFILES.in.2 ] ; then \
	 	rm -f $(srcdir)/po/POTFILES.in.2 ; \
	 fi ; \
	  for dir in $$(echo Gui src dashdesigner gaugedesigner widgets msloader); do \
		find $$dir -name "*.c" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; \
		find $$dir -name "*.glade" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; \
	  done ;\
	if diff $(srcdir)/po/POTFILES.in $(srcdir)/po/POTFILES.in.2 >/dev/null 2>&1 ; then \
		rm -f $(srcdir)/po/POTFILES.in.2 ; \
	else \
		mv $(srcdir)/po/POTFILES.in.2 $(srcdir)/po/POTFILES.in ; \
	fi ;\
	find $(srcdir)/Gui -name "*.in" -print | sort > $(srcdir)/po/SPECIALS.in.2 ; \
	find $(srcdir)/RealtimeMaps -name "*map.in" -print | sort >> $(srcdir)/po/SPECIALS.in.2 ; \
	find $(srcdir)/RuntimeStatus -name "*conf.in" -print | sort >> $(srcdir)/po/SPECIALS.in.2 ; \
	find $(srcdir)/Interrogator -name "*prof.in" -print | sort >> $(srcdir)/po/SPECIALS.in.2 ; \
	if diff $(srcdir)/po/SPECIALS.in $(srcdir)/po/SPECIALS.in.2 >/dev/null 2>&1 ; then \
		rm -f $(srcdir)/po/SPECIALS.in.2 ; \
	else \
		mv $(srcdir)/po/SPECIALS.in.2 $(srcdir)/po/SPECIALS.in ; \
	fi \
	&& cd po && $(MAKE) $(AM_MAKEFLAGS) update-po)

.PHONY: check-gettext update-po
