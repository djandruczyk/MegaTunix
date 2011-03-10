profilesdir = $(datadir)/MegaTunix/$(INTERROGATOR_DATA_DIR)/Profiles/MS-2

profiles_in = \
	MS2-Extra_2.1.0.prof.in \
	MS2-Extra_2.1.0e.prof.in \
	MS2-Extra_3.0.x.prof.in \
	MS2-Extra_3.0.3w.prof.in \
	MS2-Extra_3.0.3y.prof.in \
	MS2-Extra_3.1.0.prof.in \
	MS3-Extra_1.0.0.prof.in \
	MS2.prof.in 


profiles_DATA = details.cfg \
		tests.cfg \
		comm.xml \
		$(profiles_in:.prof.in=.prof)
@INTLTOOL_PROF_RULE@

EXTRA_DIST = details.cfg tests.cfg comm.xml $(profiles_in)

CLEANFILES = *.h *.prof
