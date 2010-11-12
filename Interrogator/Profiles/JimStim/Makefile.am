profilesdir = $(datadir)/MegaTunix/$(INTERROGATOR_DATA_DIR)/Profiles/JimStim

profiles_in = \
	JimStim-2.x.prof.in 


profiles_DATA = details.cfg \
		tests.cfg \
		comm.xml \
		$(profiles_in:.prof.in=.prof)
@INTLTOOL_PROF_RULE@

EXTRA_DIST = details.cfg tests.cfg comm.xml $(profiles_in)

CLEANFILES = *.h *.prof
