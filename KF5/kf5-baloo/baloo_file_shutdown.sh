#!/bin/sh
# workaround for baloo_file processes persisting
# after logout, parent bug:
# https://bugzilla.redhat.com/show_bug.cgi?id=1861700

qdbus-qt5 org.kde.baloo >& /dev/null && \
qdbus-qt5 org.kde.baloo / org.kde.baloo.quit ||:
