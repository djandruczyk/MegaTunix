AM_CPPFLAGS = -I$(top_srcdir)/include @MEGATUNIX_DEFINES@
lib_LTLIBRARIES = libmtxyaml.la
libmtxyaml_la_SOURCES = yaml_private.h api.c reader.c scanner.c parser.c loader.c writer.c emitter.c dumper.c
libmtxyaml_la_LDFLAGS = -shared $(no_undefined) -release $(YAML_LT_RELEASE) -version-info $(YAML_LT_CURRENT):$(YAML_LT_REVISION):$(YAML_LT_AGE)

if CROSS_COMPILING
no_undefined = -no-undefined
endif
YAML_LT_RELEASE = 0
YAML_LT_CURRENT = 2
YAML_LT_REVISION = 2
YAML_LT_AGE = 0

AM_CFLAGS = -DYAML_VERSION_MAJOR=0 -DYAML_VERSION_MINOR=1 -DYAML_VERSION_PATCH=4 -DYAML_VERSION_STRING=\"0.1.4\"
CLEANFILES = *~ core gmon.out nohup.out

EXTRA_DIST =
