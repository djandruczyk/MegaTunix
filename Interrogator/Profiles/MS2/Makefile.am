profilesdir = $(datadir)/MegaTunix/$(INTERROGATOR_DATA_DIR)/Profiles/MS-2

profiles_in = \
	MS2-Extra_2.1.0.prof.in \
	MS2-Extra_2.1.0e.prof.in \
	MS2-Extra_3.0.x.prof.in \
	MS2-Extra_3.0.3w.prof.in \
	MS2-Extra_3.0.3y.prof.in \
	MS2-Extra_3.1.0.prof.in \
	MS2-Extra_3.1.1_17.prof.in \
	MS2-Extra_3.1.x.als.prof.in \
	MS2-Extra_3.2.1.prof.in \
	MS2.prof.in  \
	MS2-2.905.prof.in 


profiles_DATA = details.cfg \
		tests.cfg \
		comm.xml \
		$(profiles_in:.prof.in=.prof)
@INTLTOOL_PROF_RULE@

EXTRA_DIST = details.cfg tests.cfg comm.xml $(profiles_in)

CLEANFILES = *.h *.prof
