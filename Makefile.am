# If you want to build these components, please cd to their respective
# directories and type `make'.
AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

#subdirs = include glib_override yaml-cpp mtxmatheval mtxcommon widgets msloader src gaugedesigner dashdesigner ecu_snapshots Interrogator LookupTables RealtimeMaps RuntimeSliders RuntimeStatus RuntimeText Gui menu category icons desktop po 
subdirs = include yaml-cpp mtxmatheval mtxcommon widgets msloader src gaugedesigner dashdesigner ecu_snapshots Interrogator LookupTables RealtimeMaps RuntimeSliders RuntimeStatus RuntimeText Gui menu category icons desktop po 

SUBDIRS =  $(subdirs) 

DIST_SUBDIRS =  $(subdirs)  

DEPDIR = ${subdirs}/.deps

INTLTOOL_FILES = intltool-extract.in \
		intltool-merge.in \
		intltool-update.in

# Release tag message file
RELTAG = gitTagMessage.txt
TAGNAME = gitTagName.txt

CLEANFILES = *~ core autom4te.cache gmon.out

DISTCLEANFILES = intltool-extract intltool-merge intltool-update po/.intltool-merge-cache

EXTRA_DIST = m4/ChangeLog configure CREDITS AUTHORS README.md INSTALL LICENSE ${INTLTOOL_FILES}
doc:
	@doxygen

check-gettext:
	@if test x$(USE_NLS) != "xyes" ; then echo "Missing gettext. Rerun configure and check for" \
	"'checking whether to use NLS... yes'!" ; exit 1 ; fi

update-po: check-gettext
	(if [ -f $(srcdir)/po/POTFILES.in.2 ] ; then \
	 	rm -f $(srcdir)/po/POTFILES.in.2 ; \
	 fi ; \
	  for dir in $$(echo Gui src dashdesigner gaugedesigner widgets msloader menu); do \
		find $$dir -name "*.c" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; \
		find $$dir -name "*.cpp" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; \
		find $$dir -name "*.glade" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; \
		find $$dir -name "*.ui" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; \
	  	find $$dir -name "*.desktop.in" -print | sort >> $(srcdir)/po/POTFILES.in.2 ; \
	  done ;\
	if diff $(srcdir)/po/POTFILES.in $(srcdir)/po/POTFILES.in.2 >/dev/null 2>&1 ; then \
		rm -f $(srcdir)/po/POTFILES.in.2 ; \
	else \
		mv $(srcdir)/po/POTFILES.in.2 $(srcdir)/po/POTFILES.in ; \
	fi ;\
	find $(srcdir)/Gui -name "*.in" -print | sort > $(srcdir)/po/SPECIALS.in.2 ; \
	find $(srcdir)/RealtimeMaps -name "*.xml" -print | sort >> $(srcdir)/po/SPECIALS.in.2 ; \
	find $(srcdir)/RuntimeStatus -name "*.xml" -print | sort >> $(srcdir)/po/SPECIALS.in.2 ; \
	find $(srcdir)/Interrogator -name "*prof.in" -print | sort >> $(srcdir)/po/SPECIALS.in.2 ; \
	if diff $(srcdir)/po/SPECIALS.in $(srcdir)/po/SPECIALS.in.2 >/dev/null 2>&1 ; then \
		rm -f $(srcdir)/po/SPECIALS.in.2 ; \
	else \
		mv $(srcdir)/po/SPECIALS.in.2 $(srcdir)/po/SPECIALS.in ; \
	fi \
	&& cd po && $(MAKE) $(AM_MAKEFLAGS) update-po)

testgit:
	@echo "################################################################################"
	@echo "#                            Checking git status...                            #"
	@echo "################################################################################"
	@echo -n "Are we on master? "
	@if [ `git symbolic-ref HEAD` = refs/heads/master ] ; then echo "Yes, we're on master" ; else echo "No we are NOT!" ; exit 1 ; fi
	@echo -n "Is everything checked in? "
	@if [ `git status -s | wc -l` -gt 0 ] ; then echo "NO! There's stuff not checked in, go fix that..." ; exit 1 ; fi
	@echo "Fetching from origin..."
	@git fetch
	@echo -n "Are we behind? "
	@if [ $$(git rev-list `git show-ref --hash refs/heads/master`..`git show-ref --hash refs/remotes/origin/master` | wc -l) -gt 0 ] ; then echo "Yes, go fix that!!" ; exit 1; else echo "nope, we're good..." ; fi
	@echo -n "Are we ahead? "
	@if [ $$(git rev-list `git show-ref --hash refs/remotes/origin/master`..`git show-ref --hash refs/heads/master` | wc -l) -gt 0 ] ; then echo "Yes, go fix that!!" ; exit 1; else echo "nope, we're good..." ; fi
		
tagmessage:
	vim $(RELTAG)

tagname:
	vim $(TAGNAME)

tagrelease: tagname tagmessage
	git tag -F $(RELTAG) `cat $(TAGNAME)` HEAD

release: testgit relbuild relperf
relperf: relcommit tagrelease
.PHONY: check-gettext update-po doc 
.PHONY: testgit relbuild relperf tagrelease relcommit
