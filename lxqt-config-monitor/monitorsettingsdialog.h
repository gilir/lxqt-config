/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>
    Copyright (C) 2013  <copyright holder> <email>

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

#ifndef MONITORSETTINGSDIALOG_H
#define MONITORSETTINGSDIALOG_H

#include <QDialog>
#include <KScreen/GetConfigOperation>
#include "ui_mainwindow.h"
#include "monitor.h"


class TimeoutDialog;
class QTimer;

class MonitorSettingsDialog: public QDialog {
  Q_OBJECT

public:
  MonitorSettingsDialog();
  virtual ~MonitorSettingsDialog();
  virtual void accept();

private:
  void setMonitorsConfig();
  void setupUi();

  void deleteTimeoutData(); // Used to delete data from TimeoutDialog

private Q_SLOTS:
    void configReceived(KScreen::ConfigOperation *op);

  // Timeout dialog signals
  void onCancelSettings();

  void onDialogButtonClicked(QAbstractButton* button);

private:
  Ui::MonitorSettingsDialog ui;
  KScreen::ConfigPtr mConfig;
  // TimeoutDialog data
  TimeoutDialog* timeoutDialog;
  QTimer* timer;
};

#endif // MONITORSETTINGSDIALOG_H
