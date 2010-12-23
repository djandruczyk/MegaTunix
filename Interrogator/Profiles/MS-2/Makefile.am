profilesdir = $(datadir)/MegaTunix/$(INTERROGATOR_DATA_DIR)/Profiles/MS-2

profiles_in = \
	MS-II-Extra_2.1.0.prof.in \
	MS-II-Extra_2.1.0e.prof.in \
	MS-II-Extra_3.0.x.prof.in \
	MS-II-Extra_3.0.3w.prof.in \
	MS-II-Extra_3.0.3y.prof.in \
	MS-II.prof.in 


profiles_DATA = details.cfg \
		tests.cfg \
		comm.xml \
		$(profiles_in:.prof.in=.prof)
@INTLTOOL_PROF_RULE@

EXTRA_DIST = details.cfg tests.cfg comm.xml $(profiles_in)

CLEANFILES = *.h *.prof
