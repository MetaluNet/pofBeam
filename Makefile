# Attempt to load a config.make file.
# If none is found, project defaults in config.project.make will be used.
ifneq ($(wildcard config.make),)
	include config.make
endif

# make sure the the OF_ROOT location is defined
ifndef OF_ROOT
    OF_ROOT=../../..
endif

# call the project makefile!
include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk

ifeq ($(PLATFORM_OS),Darwin)
macos: Release
	install_name_tool -change @executable_path/libfmodex.dylib @loader_path/libfmodex.dylib bin/$(EXTNAME).pd_darwin.app/Contents/MacOS/$(EXTNAME).pd_darwin
endif


