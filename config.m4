dnl $Id$
dnl config.m4 for extension idebug

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(idebug, for idebug support,
dnl Make sure that the comment is aligned:
dnl [  --with-idebug             Include idebug support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(idebug, whether to enable idebug support,
dnl Make sure that the comment is aligned:
dnl [  --enable-idebug           Enable idebug support])

if test "$PHP_IDEBUG" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-idebug -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/idebug.h"  # you most likely want to change this
  dnl if test -r $PHP_IDEBUG/$SEARCH_FOR; then # path given as parameter
  dnl   IDEBUG_DIR=$PHP_IDEBUG
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for idebug files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       IDEBUG_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$IDEBUG_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the idebug distribution])
  dnl fi

  dnl # --with-idebug -> add include path
  dnl PHP_ADD_INCLUDE($IDEBUG_DIR/include)

  dnl # --with-idebug -> check for lib and symbol presence
  dnl LIBNAME=idebug # you may want to change this
  dnl LIBSYMBOL=idebug # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $IDEBUG_DIR/$PHP_LIBDIR, IDEBUG_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_IDEBUGLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong idebug lib version or lib not found])
  dnl ],[
  dnl   -L$IDEBUG_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(IDEBUG_SHARED_LIBADD)

  PHP_NEW_EXTENSION(idebug, idebug.c, $ext_shared)
fi
