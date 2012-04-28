profilesdir = $(datadir)/MegaTunix/$(INTERROGATOR_DATA_DIR)/Profiles/MS-1

profiles_in = \
	MS1-BnG_1.x.prof.in \
	MS1-BnG_3.x.prof.in \
	MS1-DT-1.02.prof.in \
	MS1-Extra-Highres-08x.prof.in \
	MS1-Extra-Highres-010x.prof.in \
	MS1-Extra-Highres-011x.prof.in \
	MS1-Extra-Highres-011d.prof.in \
	MS1-Extra-029-series.prof.in \
	MS1-Extra-029y-series.prof.in 


profiles_DATA = details.cfg \
		tests.cfg \
		comm.xml \
		$(profiles_in:.prof.in=.prof)
@INTLTOOL_PROF_RULE@

EXTRA_DIST = details.cfg tests.cfg comm.xml $(profiles_in)

CLEANFILES = *.h *.prof
