profilesdir = $(datadir)/MegaTunix/$(INTERROGATOR_DATA_DIR)/Profiles

profiles_in = \
	B_and_G_1.x.prof.in \
	B_and_G_2.0-3.01.prof.in \
	Dualtable-1.02.prof.in \
	MS-II-Extra_2.1.0.prof.in \
	MS-II-Extra_3.0.x.prof.in \
	MS-II-Extra_3.0.3v.prof.in \
	MS-II.prof.in \
	PIS_V1_00.prof.in \
	SquirtnSpark-Extra-HR-029x.prof.in \
	SquirtnSpark-Extra-HR-010a.prof.in \
	SquirtnSpark-Extra-HR-011x.prof.in \
	SquirtnSpark-Extra-HR-011d.prof.in \
	SquirtnSpark-Extra-029x.prof.in \
	SquirtnSpark-Extra-029yx.prof.in 


profiles_DATA = $(profiles_in:.prof.in=.prof)
@INTLTOOL_PROF_RULE@

EXTRA_DIST = $(profiles_in) README

CLEANFILES = *.h *.prof
