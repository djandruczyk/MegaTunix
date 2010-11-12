rtmapdir = $(datadir)/MegaTunix/$(REALTIME_MAPS_DATA_DIR)

rtmap_in_files = 	\
		BnG-Std.rtv_map.in \
		DT-Std.rtv_map.in \
		MSnS-Extra-HR-08x.rtv_map.in \
		MSnS-Extra-HR-011d.rtv_map.in \
		MSnS-Extra-029x.rtv_map.in \
		MS2-Std.rtv_map.in \
		MS2-Extra-2.1.0.rtv_map.in \
		MS2-Extra-3.0.x.rtv_map.in \
		MS2-Extra-3.0.3w.rtv_map.in \
		PIS_V1_00-firmware.rtv_map.in \
		JimStim-2.x.rtv_map.in

rtmap_DATA = $(rtmap_in_files:.rtv_map.in=.rtv_map)
@INTLTOOL_RTVMAP_RULE@

EXTRA_DIST = $(rtmap_in_files) README

CLEANFILES = *.rtv_map *.h

