#!/bin/bash
# This file was generated automatically.
# Please edit generate-configure.sh rather than this file.

# configuration

PRODUCT=KDSOAP
Product=KDSoap
product=kdsoap
ProductSpace="KD Soap"

VERSION=1.1.0

INSTALLATION_SUPPORTED=true
STATIC_BUILD_SUPPORTED=true
HAS_DESIGNER_PLUGIN=false

# function definitions

function die {
    echo "$1" 1>&2
    exit 1
}

# find SRCDIR

SRCDIR="$(dirname "$0")"
case "$SRCDIR" in
    /*) # absolute
        ;;
    *)  # relative - prepend $PWD
        SRCDIR="$PWD/$SRCDIR"
        ;;
esac
BUILDDIR="$PWD"
PACKSCRIPTS_DIR="$SRCDIR/../admin/packscripts"

# find Qt

if [ -z "$QTDIR" ] ;  then
    QTDIR="$(qmake -query QT_INSTALL_PREFIX)"
    if [ $? -ne 0 ] ; then
	QTDIR=
    fi
fi

[ -z "$QTDIR" ] && die "You need QTDIR defined, or qmake in the PATH"

# config variables

hide_symbols=yes
shared=yes
debug=no
release=yes
prefix=
plugins_prefix=
unittests=no


if [ -d "$PACKSCRIPTS_DIR" ] ; then
    unittests=yes

fi

# check licence

function check_license {

    [ -d "$PACKSCRIPTS_DIR" ] && return 0 
    [ -f "$SRCDIR/.license.accepted" ] && return 0

    if [ -f "$SRCDIR/LICENSE.GPL.txt" -a -f "$SRCDIR/LICENSE.US.txt" -a -f "$SRCDIR/LICENSE.txt" ] ; then
        echo
        echo "Please choose your license."
        echo
        echo "Type 1 for the GNU General Public License (GPL)."
        echo "Type 2 for the $ProductSpace Commercial License for USA/Canada."
        echo "Type 3 for the $ProductSpace Commercial License for anywhere outside USA/Canada."
        echo "Anything else cancels."
        echo
        echo -n "Select: "
        read license

    elif [ -f "$SRCDIR/LICENSE.GPL.txt" ] ; then
        license="1"

    elif [ -f "$SRCDIR/LICENSE.US.txt" ] ; then
        license="2"

    elif [ -f "$SRCDIR/LICENSE.txt" ] ; then
        license="3"
    else
        die "Couldn't find license file, aborting"
    fi

    if [ "$license" = "1" ]; then
        license_name="GNU General Public License (GPL)"
        license_file=LICENSE.GPL.txt
    elif [ "$license" = "2" ]; then
        license_name="$ProductSpace USA/Canada Commercial License"
        license_file=LICENSE.US.txt
    elif [ "$license" = "3" ]; then
        license_name="$ProductSpace Commercial License"
        license_file=LICENSE.txt
    else
        return 1
    fi

    while true ; do
	cat <<EOF

License Agreement

You are licensed to use this software under the terms of
the $license_name.

Type '?' to view the $license_name
Type 'yes' to accept this license offer.
Type 'no' to decline this license offer.

Do you accept the terms of this license?
EOF
        read answer

	[ "$answer" = "no" ]  && return 1
	[ "$answer" = "yes" ] && touch "$SRCDIR/.license.accepted" && return 0
	[ "$answer" = "?" ]   && more "$SRCDIR/$license_file"
    done
}

if ! check_license ; then
    die "You are not licensed to use this software."
fi

# options parsing

function usage {
    [ -z "$1" ] || echo "$0: unknown option \"$1\"" 1>&2
    echo "usage: $0 [options]" 1>&2
    cat <<EOF 1>&2
where options include:

EOF
if [ "$INSTALLATION_SUPPORTED" = "true" ]; then
    cat <<EOF 1>&2
  -prefix <path>
      install $ProductSpace into <path>
EOF
if [ "$HAS_DESIGNER_PLUGIN" = "true" ]; then
    cat <<EOF 1>&2
  -plugins-prefix <path>
      install $Product\'s designer plugin into <path>/designer
EOF
fi  
fi  
cat <<EOF 1>&2

  -release / -debug
      build in debug/release mode
EOF
if [ "$STATIC_BUILD_SUPPORTED" = "true" ]; then
  cat <<EOF 1>&2

  -static / -shared
      build static/shared libraries
EOF
fi
cat <<EOF 1>&2

  -[no-]hide-symbols (Unix only)
      reduce the number of exported symbols

  -spec <mkspec>
      compile $ProductSpace for specific Qt-supported target <mkspec>

  -unittests / -no-unittests
      enable/disable compiled-in unittests


EOF
    exit 1
}

while [ $# -ne 0 ] ; do
    case "$1" in
        -plugins-prefix)
            if [ "$INSTALLATION_SUPPORTED" != "true" -o "$HAS_DESIGNER_PLUGIN" != "true" ]; then
              echo "Installation not supported, -plugins-prefix option ignored" 2>&1
#             usage
            fi
            shift
            if [ $# -eq 0 ] ; then
                    echo "-plugins-prefix needs an argument" 2>&1
                    usage
            fi
            plugins_prefix="$1"
            ;;
	-prefix)
            if [ "$INSTALLATION_SUPPORTED" != "true" ]; then
	      echo "Installation not supported, -prefix option ignored" 2>&1
#	      usage
	    fi
	    shift
            if [ $# -eq 0 ] ; then
		    echo "-prefix needs an argument" 2>&1
		    usage
	    fi
            prefix="$1"
	    ;;
        -no-hide-symbols)
            hide_symbols=no
            ;;
        -hide-symbols)
            hide_symbols=yes
            ;;
        -override-version) # undocumented by design
            shift
            if [ $# -eq 0 ] ; then
                    echo "-override-version needs an argument" 2>&1
                    usage
            fi
            VERSION="$1"
            ;;
        -shared)
            shared=yes
            ;;
        -static)
            if [ "$STATIC_BUILD_SUPPORTED" != "true" ]; then
                echo "Static build not supported, -static option not allowed" 2>&1
                usage
            fi
            shared=no
            ;;
        -debug)
            debug=yes
            release=no
            ;;
        -release)
            debug=no
            release=yes
            ;;
        -spec)
            shift
            if [ $# -eq 0 ] ; then
                echo "-prefix needs an argument" 2>&1
                usage
            fi
            SPEC="-spec $1"
            ;;
        -no-unittests)
            unittests=no
            ;;
        -unittests)
            unittests=yes
            ;;

        *)
            usage "$1"
            ;;
    esac
    shift
done

# apply defaults to unset options

readonly default_prefix=/usr/local/KDAB/$Product-$VERSION
readonly default_plugins_prefix="$("$QTDIR/bin/qmake" -query QT_INSTALL_PLUGINS)"

if [ -z "$prefix" ] ; then
    prefix="$default_prefix"
fi
if [ -z "$plugins_prefix" ] ; then
    if [ -w "$default_plugins_prefix" ] ; then
        plugins_prefix="$default_plugins_prefix"
    else
        plugins_prefix="$prefix/plugins"
    fi
fi


# clean previous build

find . -name debug -o -name release -o -name Makefile\* | xargs rm -rf

if [ -f "$SRCDIR/src/src.pro" ] ; then
    rm -rf lib bin
fi

# call makeincludes.pl

if false ; then   ### TODO if [ -d "$PACKSCRIPTS_DIR" ] ; then
    echo
    echo -n "Creating include directory..."
    (
        cd "$SRCDIR" || die "Failed to cd into SRCDIR ($SRCDIR)"
        perl "$PACKSCRIPTS_DIR/makeincludes.pl" > makeincludes.log 2>&1 || die "Failed to run $PACKSCRIPTS_DIR/makeincludes.pl"
        rm makeincludes.log
    ) || exit 1
    echo done
    echo
fi

# write .qmake.cache

echo -n > ".qmake.cache"
(
    echo "${PRODUCT}_SOURCE_DIR=\"$SRCDIR\""
    echo "${PRODUCT}_BUILD_DIR=\"$BUILDDIR\""

    echo "VERSION=$VERSION"
    echo "CONFIG += ${product}_target"

    if [ "$debug" = "yes" ]; then
      echo "CONFIG -= release"
      echo "CONFIG += debug"
      echo "CONFIG -= debug_and_release"
    fi

    if [ "$release" = "yes" ]; then
      echo "CONFIG += release"
      echo "CONFIG -= debug"
      echo "CONFIG -= debug_and_release"
    fi

    [ "$hide_symbols" = "yes" ] &&  echo "CONFIG += hide_symbols"
    [ "$unittests" = "yes" ] && echo "CONFIG += unittests"


    if [ "$shared" = "yes" ]; then
      echo "CONFIG -= static"
      echo "CONFIG -= staticlib"
      echo "CONFIG += shared"
    else
      echo "CONFIG += static"
      echo "CONFIG += staticlib"
      echo "CONFIG -= shared"
    fi

    if [ -d "$QTDIR/include/Qt/private" ] ; then
	echo "CONFIG += have_private_qt_headers"
	echo "INCLUDEPATH += $QTDIR/include/Qt/private"
    #else
        #echo "QTDIR must point to an installation that has private headers installed."
        #echo "Some features will not be available."    
    fi
    if [ "$INSTALLATION_SUPPORTED" = "true" ] ; then
        echo "${PRODUCT}_INSTALL_PREFIX = $prefix"
        if [ "$HAS_DESIGNER_PLUGIN" = "true" ] ; then
            echo "${PRODUCT}_PLUGINS_PREFIX = $plugins_prefix"
        fi
    fi

) >> ".qmake.cache"

# link .prf files to builddir

if [ ! -f ${product}.pro ] ; then # ie. srcdir != builddir
    if [ -d "$SRCDIR/features" ] ; then
        [ -d "$BUILDDIR/features" ] || mkdir "$BUILDDIR/features" || die "Could not create directory $BUILDDIR/features"
        for P in "$SRCDIR/features"/*.prf ; do
            name=$(basename "$P")
            echo "include( \"\$\$${PRODUCT}_SOURCE_DIR/features/$name\" )" > "$BUILDDIR/features/$name" || die "Could not create forward .prf file $BUILDDIR/features/$name"
        done
    fi
fi

# call qmake

# Make a copy so that each run of qmake on $product.pro starts clean
cp -f .qmake.cache .confqmake.cache

"$QTDIR/bin/qmake" ${SPEC} -recursive  "$SRCDIR/$product.pro" "${PRODUCT}_BASE=`pwd`" || die "qmake failed"

# display configuration summary

cat <<EOF 1>&2

$ProductSpace v$VERSION configuration:
EOF

if [ "$INSTALLATION_SUPPORTED" = "true" ]; then
cat <<EOF 1>&2
  Install Prefix.............: $prefix
    (default: $default_prefix)
EOF
if [ "$HAS_DESIGNER_PLUGIN" = "true" ]; then
cat <<EOF 1>&2
  Install Plugins Prefix.....: $plugins_prefix
    (default: $default_plugins_prefix)
EOF
fi
fi

cat <<EOF 1>&2
  Debug......................: $debug (default: no)
  Release....................: $release (default: yes)
EOF
if [ "$STATIC_BUILD_SUPPORTED" = "true" ]; then
  cat <<EOF 1>&2
  Shared build...............: $shared (default: yes)
EOF
fi
if [ "$SPEC" != "" ]; then
  cat <<EOF 1>&2
  Spec.......................: ${SPEC#-spec }
EOF
fi
cat <<EOF 1>&2
  Restricted symbol export
    (shared build only)......: $hide_symbols (default: yes)
  Compiled-In Unit Tests.....: $unittests (default: no)

EOF

echo
if [ "$INSTALLATION_SUPPORTED" = "true" ]; then
  echo "Ok, now run make, then make install to install into $prefix"
  if [ "$HAS_DESIGNER_PLUGIN" = "true" ] ; then
  if [ "$plugins_prefix" != "$default_plugins_prefix" ] ; then
      echo "Please add $plugins_prefix to Qt's list of plugin paths."
  fi
  fi
else
  echo "Ok, now run make to build the framework."
fi 
