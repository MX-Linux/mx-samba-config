# **********************************************************************
# * Copyright (C) 2021 MX Authors
# *
# * Authors: Adrian <adrian@mxlinux.org>
# *          Dolphin_Oracle
# *          MX Linux <http://mxlinux.org>
# *
# * This is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this package. If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += core gui
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mx-samba-config
TEMPLATE = app


SOURCES += main.cpp\
    editshare.cpp \
    mainwindow.cpp \
    about.cpp \
    cmd.cpp

HEADERS  += \
    editshare.h \
    mainwindow.h \
    version.h \
    about.h \
    cmd.h

FORMS    += \
    editshare.ui \
    mainwindow.ui

TRANSLATIONS += translations/mx-samba-config_ca.ts \
                translations/mx-samba-config_de.ts \
                translations/mx-samba-config_el.ts \
                translations/mx-samba-config_es.ts \
                translations/mx-samba-config_fr.ts \
                translations/mx-samba-config_it.ts \
                translations/mx-samba-config_ja.ts \
                translations/mx-samba-config_nl.ts \
                translations/mx-samba-config_ro.ts \
                translations/mx-samba-config_sv.ts

RESOURCES += \
    images.qrc

