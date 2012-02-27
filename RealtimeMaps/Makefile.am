rtmapdir = $(datadir)/MegaTunix/$(REALTIME_MAPS_DATA_DIR)

rtmap_in_files = 	\
		FreeEMS.rtv_map.in \
		JimStim-2.x.rtv_map.in \
		MS1-BnG-Std.rtv_map.in \
		MS1-DT-Std.rtv_map.in \
		MS1-Extra-HR-08x.rtv_map.in \
		MS1-Extra-HR-011d.rtv_map.in \
		MS1-Extra-029x.rtv_map.in \
		MS2-Std.rtv_map.in \
		MS2-Extra-2.1.0.rtv_map.in \
		MS2-Extra-3.0.x.rtv_map.in \
		MS2-Extra-3.0.3w.rtv_map.in \
		MS2-Extra-3.0.3y.rtv_map.in \
		MS2-Extra-3.1.1_17.rtv_map.in \
		MS2-Extra-3.2.1.rtv_map.in \
		PIS_V1_00-firmware.rtv_map.in 

rtxml_files = 	\
		FreeEMS.xml \
		JimStim-2.x.xml \
		MS1-BnG-Std.xml \
		MS1-DT-Std.xml \
		MS1-Extra-HR-08x.xml \
		MS1-Extra-HR-011d.xml \
		MS1-Extra-029x.xml \
		MS2-Std.xml \
		MS2-Extra-2.1.0.xml \
		MS2-Extra-3.0.x.xml \
		MS2-Extra-3.0.3w.xml \
		MS2-Extra-3.0.3y.xml \
		MS2-Extra-3.1.1_17.xml \
		MS2-Extra-3.2.1.xml \
		PIS_V1_00-firmware.xml 


rtmap_DATA = $(rtmap_in_files:.rtv_map.in=.rtv_map) $(rtxml_files)
@INTLTOOL_RTVMAP_RULE@

EXTRA_DIST = $(rtmap_in_files) README

CLEANFILES = *.rtv_map *.h

