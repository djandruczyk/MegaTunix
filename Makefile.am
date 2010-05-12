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

check-gettext:
	@if test x$(USE_NLS) != "xyes" ; then echo "Missing gettext. Rerun configure and check for" \
	"'checking whether to use NLS... yes'!" ; exit 1 ; fi

update-po: check-gettext
	echo "[encoding: UTF-8]" > $(srcdir)/po/POTFILES.in.2 ;
	@find $(srcdir)/src/ -name "*.c" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; 
	@find $(srcdir)/gaugedesigner/ -name "*.c" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; 
	@find $(srcdir)/dashdesigner/ -name "*.c" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; 
	@find $(srcdir)/widgets/ -name "*.c" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; 
	@find $(srcdir)/msloader/ -name "*.c" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; 
	@find $(srcdir)/Gui/ -name "*.glade" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; 
	@find $(srcdir)/Gui/ -name "*.datamap.in" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; 
	@find $(srcdir)/desktop/ -name "*.directory.in" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; 
	@find $(srcdir)/menu/ -name "*.desktop.in" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; 
	@find $(srcdir)/RealtimeMaps/ -name "*.rtv_map.in" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; 
	@find $(srcdir)/RuntimeStatus/ -name "*.status_conf.in" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; 
	@find $(srcdir)/Interrogator/Profiles/ -name "*.prof.in" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; \
	if diff $(srcdir)/po/POTFILES.in $(srcdir)/po/POTFILES.in.2 >/dev/null 2>&1 ; then \
		rm -f $(srcdir)/po/POTFILES.in.2 ; \
	else \
		mv $(srcdir)/po/POTFILES.in.2 $(srcdir)/po/POTFILES.in ; \
	fi
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-po

update-gmo: check-gettext
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

force-update-gmo: check-gettext
	touch po/*.po
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

force-update-gmo-%: check-gettext
	@language=`echo $@ | sed s/force-update-gmo-//` ; \
	if test ! -f po/$$language.po ; then echo "file po/$$language.po does not exist" ; exit 1 ; fi ; \
	touch po/$$language.po ; \
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

.PHONY: check-gettext update-po update-gmo force-update-gmo
