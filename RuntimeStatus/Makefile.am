rtstatusdir = $(datadir)/MegaTunix/$(RTSTATUS_DATA_DIR)

rtstatus_in_files = \
		MS1-Std-firmware.status_conf \
		MS2-Std-firmware.status_conf \
		MS2-Extra-firmware.status_conf \
		MSnS-Extra-firmware.status_conf \
		PIS_V1_00-firmware.status_conf

rtstatus_DATA = $(rtstatus_in_files:.status_conf.in=.status_conf)
@INTLTOOL_STATUS_RULE@

EXTRA_DIST = $(rtstatus_DATA) README

CLEANFILES = *.status_conf
