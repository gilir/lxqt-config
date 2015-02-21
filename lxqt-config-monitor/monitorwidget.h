/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>
    Copyright (C) 2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef _MONITORWIDGET_H_
#define _MONITORWIDGET_H_

#include <QGroupBox>
#include <QStringList>
#include <QHash>
#include <QList>
#include <KScreen/Config>
#include <KScreen/Output>

#include "ui_monitorwidget.h"


#define PrimaryDisplay 0
#define ExtendDisplay 1
#define CloneDisplay 2

#define RightOf 1
#define LeftOf 2
#define Above 3
#define Below 4
#define Manually 5


// Monitor info
class MonitorWidget : public QGroupBox {
  Q_OBJECT

public:
    MonitorWidget(KScreen::OutputPtr output, KScreen::ConfigPtr config, QWidget* parent = 0);

    void updateRefreshRates();

    KScreen::OutputPtr output;
    KScreen::ConfigPtr config;

    Ui::MonitorWidget ui;

public Q_SLOTS:
    void setOnlyMonitor(bool isOnlyMonitor);

private Q_SLOTS:
    void onBehaviorChanged(int);
    void onPositioningChanged(int);
    void onPositionChanged(int);
    void onResolutionChanged(int);
};

#endif // _MONITORWIDGET_H_
