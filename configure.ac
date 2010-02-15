dnl Transmission Suite

AC_INIT([TransmissionSuite],[master],,[txsuite])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

BUILDTOOLS_INIT

AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC

top_builddir="`pwd`"

examplesdir='${datarootdir}/examples/${PACKAGE_TARNAME}'
AC_SUBST([examplesdir])

dnl ###### Top-level subprojects

TX_ENABLE_SUBDIR(dss,AS_HELP_STRING([--enable-dss],[build Darwin Streaming Server]),no)
TX_ENABLE_SUBDIR(tsdemux, AS_HELP_STRING([--enable-tsdemux],[don't build the tsdemux utility]),no)
TX_ENABLE_SUBDIR(png2rgb, AS_HELP_STRING([--disable-png2rgb],[don't build the png2rgb utility]))
TX_ENABLE_SUBDIR(tstools, AS_HELP_STRING([--disable-tstools],[don't build the Transport Stream Tools]))
TX_ENABLE_SUBDIR(mpeg4ip, AS_HELP_STRING([--disable-mpeg4ip],[don't build the MPEG4IP suite]))
TX_ENABLE_SUBDIR(mhegc, AS_HELP_STRING([--disable-mhegc],[don't build the RedButton MHEG compiler/decompiler]))
TX_ENABLE_SUBDIR(mhegenc, AS_HELP_STRING([--enable-mhegc],[build the SpongeLava MHEG compiler/decompiler]))
TX_ENABLE_SUBDIR(schroedinger, AS_HELP_STRING([--disable-schroedinger],[don't build the Schroedinger Dirac encoder libraries]))
TX_ENABLE_SUBDIR(vorbis-tools, AS_HELP_STRING([--disable-vorbis-tools],[don't build the Ogg Vorbis command-line tools]))

dnl ###### Dependencies

test x"$build_png2rgb" = x"yes" && need_libpng=yes

if test x"$need_libpng" = x"yes" ; then
	AC_MSG_CHECKING([for libpng])
	AC_ARG_WITH([libpng],[AS_HELP_STRING([--with-libpng=DIR],[specify libpng installation prefix])],[libpng_config="$withval/bin/libpng-config"],[libpng_config="libpng-config"])
	libpng_prefix=`$libpng_config --prefix`
	if test x"$libpng_prefix" = x"" ; then
		have_libpng=no
		AC_MSG_RESULT([not found])
	else
		libpng_version=`$libpng_config --version`
		have_libpng=yes
		AC_MSG_RESULT([$libpng_version installed in $libpng_prefix])
	fi
fi

test x"$build_vorbistools" = x"yes" && need_libogg=yes
test x"$build_vorbistools" = x"yes" && need_libvorbis=yes

test x"$build_rbbrowser" = x"yes" && need_ffmpeg=yes
test x"$build_rbbrowser" = x"yes" && need_libpng=yes

test x"$build_schroedinger" = x"yes" && need_liboil=yes

test x"$build_mpeg4ip" = x"yes" && need_SDL=yes

TX_WITH_INCLUDED_SUBDIR(libpng, AS_HELP_STRING([--with-included-libpng],[use the bundled version of libpng (default=auto)])])
TX_WITH_INCLUDED_SUBDIR(liboil, AS_HELP_STRING([--with-included-liboil],[use the bundled version of liboil (default=auto)])])
TX_WITH_INCLUDED_SUBDIR(x264, AS_HELP_STRING([--with-included-x264],[use the bundled version of x264 (default=auto)])])
TX_WITH_INCLUDED_SUBDIR(ffmpeg, AS_HELP_STRING([--with-included-ffmpeg],[use the bundled version of ffmpeg (default=auto)])])
TX_WITH_INCLUDED_SUBDIR(snacc, AS_HELP_STRING([--with-included-snacc],[use the bundled version of snacc (default=auto)])])
TX_WITH_INCLUDED_SUBDIR(libogg, AS_HELP_STRING([--with-included-libogg],[use the bundled version of libogg (default=auto)])])
TX_WITH_INCLUDED_SUBDIR(libvorbis, AS_HELP_STRING([--with-included-libvorbis],[use the bundled version of libvorbis (default=auto)])])
TX_WITH_INCLUDED_SUBDIR(libtheora, AS_HELP_STRING([--with-included-libtheora],[use the bundled version of libtheora (default=auto)])])
TX_WITH_INCLUDED_SUBDIR(boost, AS_HELP_STRING([--with-included-boost],[use the bundled version of boost (default=auto)])])
TX_WITH_INCLUDED_SUBDIR(SDL, AS_HELP_STRING([--with-included-SDL],[use the bundled version of SDL (default=auto)])])
TX_WITH_INCLUDED_SUBDIR(libflac, AS_HELP_STRING([--with-included-libflac],[use the bundled version of libflac (default=auto)])])

TX_BUILD_SUBPROJ(libpng)

TX_BUILD_SUBPROJ(png2rgb)

export PKG_CONFIG_PATH

AC_CONFIG_FILES([
Makefile
buildtools/Makefile
mhegc/Makefile
mhegc/helloworld/Makefile
])

AC_OUTPUT

AC_MSG_RESULT([

Configuration summary:

Darwin Streaming Server:     $build_dss
RedButton mhegc/mhegd:       $build_mhegc
tsdemux:                     $build_tsdemux
libpng:                      $build_libpng
png2rgb:                     $build_png2rgb
Transport Stream Tools:      $build_tstools
SDL:                         $build_SDL
MPEG4IP:                     $build_mpeg4ip
liboil:                      $build_liboil
Schroedinger:                $build_schroedinger
x264:                        $build_x264
Boost:                       $build_boost
Ogg:                         $build_libogg
Vorbis:                      $build_libvorbis
Theora:                      $build_libtheora
FLAC:                        $build_libflac
Vorbis Tools:                $build_vorbistools
ffmpeg:                      $build_ffmpeg
])
