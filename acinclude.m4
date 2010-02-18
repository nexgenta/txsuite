dnl Transmission suite

m4_define([_TX_S],[m4_translit([$1],[A-Z.-],[a-z__])])

dnl TX_ENABLE_SUBDIR(subdir, help, default=yes)
dnl - Optionally enable building a subproject
dnl
AC_DEFUN([TX_ENABLE_SUBDIR],[
def=]m4_if[($3,,yes,$3)
_TX_S([build_$1])=no
AC_MSG_CHECKING([whether to build $1])
if test -d $srcdir/$1 ; then
	AC_ARG_ENABLE([$1],[$2],
		_TX_S([build_$1])=$enableval,
		_TX_S([build_$1])=$def)
	AC_MSG_RESULT($_TX_S([build_$1]))
else
	AC_MSG_RESULT([no (subproject not present)])
fi
])

dnl TX_CHECK_SUBPROJ_ENABLED(project)
dnl - Check that $need_$1 and $build_$1 don't disagree
AC_DEFUN([TX_CHECK_SUBPROJ_ENABLED],[
	if test x"$_TX_S([need_$1])" = x"yes" ; then
		if test x"$_TX_S([build_$1])" = x"yes" ; then
			true
		elif test x"$_TX_S([have_$1])" = x"yes" ; then
			true
		else
			AC_MSG_ERROR([$1 is required by other sub-projects but is not present and cannot be found on your system])
		fi
	fi
])

dnl TX_WITH_INCLUDED_SUBDIR(subdir, help, default=auto)
dnl - Optionally enable building a dependent subproject
dnl
AC_DEFUN([TX_WITH_INCLUDED_SUBDIR],[
def=]m4_if[($3,,no,$3)
build_$1=no
AC_MSG_CHECKING([whether to build included $1])
if test -d $srcdir/$1 ; then
	AC_ARG_WITH(included-[$1],[$2],_TX_S([build_$1])[=$withval],_TX_S([build_$1])[=auto])
	if test x"$_TX_S([build_$1])" = x"yes" ; then
		true
	elif test x"$_TX_S([build_$1])" = x"auto" ; then
	   	if test x"$_TX_S([have_$1])" = x"yes" ; then
			_TX_S([build_$1])=no
		elif test x"$_TX_S([need_$1])" = x"yes" ; then
			_TX_S([build_$1])=yes
		elif test x"$_TX_S([want_$1])" = x"yes" ; then
			_TX_S([build_$1])=yes
		else
			_TX_S([build_$1])=$def
		fi
	elif test x"$_TX_S([build_$1])" = x"no" ; then
		if test x"$_TX_S([have_$1])" = x"yes" ; then
			true
		elif test x"$_TX_S([need_$1])" = x"yes" ; then
			AC_MSG_ERROR([$1 is required, but is not installed has been explicitly disabled])
		fi
	fi
    AC_MSG_RESULT($_TX_S([build_$1]))
else
	_TX_S([build_$1])=no
	if test x"$_TX_S([build_$1])" = x"no" && test x"$_TX_S([have_$1])" = x"no" && test x"$_TX_S([need_$1])" = x"yes" ; then
			AC_MSG_ERROR([$1 is required by other sub-projects but is not present and cannot be found on your system])
	fi
	AC_MSG_RESULT([no (subproject not present)])
fi
])

dnl TX_BUILD_SUBPROJ(subdir)
dnl - Build a subproject
dnl
AC_DEFUN([TX_BUILD_SUBPROJ],[
if test x"$_TX_S([build_$1])" = x"yes" ; then
	if test x"$NO_RECURSE" = x"" ; then
		AC_CONFIG_SUBDIRS([$1])
		PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$top_builddir/$1"
	else
		AC_MSG_RESULT([skipping configuration of sub-project in $1])	
		extra_subdirs="$extra_subdirs [$1]"
	fi
fi
AC_SUBST([extra_subdirs])
])

dnl TX_BUILD_SUBDIR(subdir)
dnl - Build a subproject which doesn't use GNU configure
dnl
AC_DEFUN([TX_BUILD_SUBDIR],[
if test x"$_TX_S([build_$1])" = x"yes" ; then
	extra_subdirs="$extra_subdirs [$1]"
	PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$top_builddir/$1"
fi
AC_SUBST([extra_subdirs])
])
