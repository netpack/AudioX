/***************************************************************************
 *   Copyright (C) 2008 - Giuseppe Cigala                                  *
 *   g_cigala@virgilio.it                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "qvumeter.h"
#include "qvumeterplugin.h"

#include <QtPlugin>

QVUMeterPlugin::QVUMeterPlugin(QObject *parent)
        : QObject(parent)
{
    initialized = false;
}

void QVUMeterPlugin::initialize(QDesignerFormEditorInterface *)
{
    if (initialized)
        return;

    initialized = true;
}

bool QVUMeterPlugin::isInitialized() const
{
    return initialized;
}

QWidget *QVUMeterPlugin::createWidget(QWidget *parent)
{
    return new QVUMeter(parent);
}

QString QVUMeterPlugin::name() const
{
    return "QVUMeter";
}

QString QVUMeterPlugin::group() const
{
    return "Lab Widgets";
}

QIcon QVUMeterPlugin::icon() const
{
    return QIcon(":qvumeter.png");
}

QString QVUMeterPlugin::toolTip() const
{
    return "";
}

QString QVUMeterPlugin::whatsThis() const
{
    return "";
}

bool QVUMeterPlugin::isContainer() const
{
    return false;
}

QString QVUMeterPlugin::domXml() const
{
    return "<widget class=\"QVUMeter\" name=\"qvumeter\">\n"
           " <property name=\"geometry\">\n"
           "  <rect>\n"
           "   <x>0</x>\n"
           "   <y>0</y>\n"
           "   <width>50</width>\n"
           "   <height>270</height>\n"
           "  </rect>\n"
           " </property>\n"
           " <property name=\"toolTip\" >\n"
           "  <string>VU Meter</string>\n"
           " </property>\n"
           " <property name=\"whatsThis\" >\n"
           "  <string>VU Meter</string>\n"
           " </property>\n"
           " </widget>\n";
}

QString QVUMeterPlugin::includeFile() const
{
    return "qvumeter.h";
}

Q_EXPORT_PLUGIN2(customwidgetplugin, QVUMeterPlugin)
