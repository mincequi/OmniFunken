TEMPLATE = subdirs

CONFIG += debug_and_release

SUBDIRS += src
!isEmpty(BUILD_TESTS):SUBDIRS += tests

OTHER_FILES += \
    etc/init.d/omnifunken \
    etc/omnifunken.conf
