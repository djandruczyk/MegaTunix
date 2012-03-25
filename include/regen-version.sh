#!/bin/bash
#set -x
if ! test -f version.h ; then touch version.h ; fi
GITHASH=`git log | head -1 |cut -f2 -d\  `
VERHASH=`head -1 version.h |cut -f2 -d\" `
if test "$GITHASH" != "$VERHASH" ; then
	if test -f version.h ; then rm version.h ; fi
	if test `grep RELEASE ../config.h |cut -f3 -d\ ` -lt 1 ; then
		if test -d ../.git ; then
			echo "#define MTX_GIT_HASH \"`git rev-parse HEAD`"\"
			echo "#define MTX_VER_SUFFIX \"SNAPSHOT_`git rev-parse HEAD |cut -b1-6`"\"
		else
			echo "#define MTX_GIT_HASH \"Not a git release\""
			echo "#define MTX_VER_SUFFIX \"RELEASE\""
		fi > version.h
	else
		echo "#define MTX_GIT_HASH \"Not a git release\""
		echo "#define MTX_VER_SUFFIX \"RELEASE\""
	fi > version.h
fi
