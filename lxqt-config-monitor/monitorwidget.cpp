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

#include "monitorwidget.h"
#include "monitor.h"
#include <QDebug>
#include <QDialogButtonBox>
#include <KScreen/EDID>


QString modeToString(KScreen::ModePtr mode) {
    // mode->name() can be anything, not just widthxheight. eg if added with cvt.
    return QString("%1x%2").arg(mode->size().width()).arg(mode->size().height());
}


KScreen::OutputPtr getOutputById(int id, KScreen::OutputList outputs) {
    Q_FOREACH(const KScreen::OutputPtr output, outputs) {
        if (output->id() == id) return output;
    }
    return KScreen::OutputPtr(NULL);
}


KScreen::ModePtr getModeById(QString id, KScreen::ModeList modes) {
    Q_FOREACH(const KScreen::ModePtr mode, modes) {
        if (mode->id() == id) return mode;
    }
    return KScreen::ModePtr(NULL);
}


MonitorWidget::MonitorWidget(KScreen::OutputPtr output, KScreen::ConfigPtr config, QWidget* parent):
    QGroupBox(parent)
{
    this->output = output;
    this->config = config;

    ui.setupUi(this);

    ui.enabledCheckbox->setChecked(output->isEnabled());

    // Add the preferred mode at the top of the list
    KScreen::ModePtr preferredMode = output->preferredMode();
    if (preferredMode) {
        ui.resolutionCombo->addItem(modeToString(preferredMode), preferredMode->id());
        // Make it bold, for good measure
        QFont font = ui.resolutionCombo->font();
        font.setBold(true);
        ui.resolutionCombo->setItemData(0, font, Qt::FontRole);
    }

    // Add each mode to the list
    Q_FOREACH(const KScreen::ModePtr &mode, output->modes()) {
        // TODO better check for duplicates
        if (mode == preferredMode) continue;
        ui.resolutionCombo->addItem(modeToString(mode), mode->id());
    }
    connect(ui.resolutionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onResolutionChanged(int)));

    if (output->currentMode()) {
        // Set the current mode in dropdown
        int idx = ui.resolutionCombo->findData(output->currentMode()->id());
        ui.resolutionCombo->setCurrentIndex(idx);
    }
    updateRefreshRates();

    // Update EDID information
    // KScreen doesn't make much public but that's ok...
    KScreen::Edid* edid = output->edid();
    if (edid && edid->isValid()) {
        ui.outputInfoLabel->setText(
            tr("Name: %1\n").arg(edid->name()) +
            tr("Vendor: %1\n").arg(edid->vendor()) +
            tr("Serial: %1\n").arg(edid->serial()) +
            tr("Display size: %1cm x %2cm\n").arg(edid->width()).arg(edid->height()) +
            tr("Serial number: %1\n").arg(edid->serial()) +
            tr("EISA device ID: %1\n").arg(edid->eisaId())
        );
    }

    if (config->connectedOutputs().count() == 1) {
        setOnlyMonitor(true);
        // There isn't always a primary output. Gross.
        output->setPrimary(true);
    } else {
        Q_FOREACH(KScreen::OutputPtr other, config->connectedOutputs()) {
            // We can't clone ourselves, or an output that already clones another
            if (other == output) continue;
            ui.clonesCombo->addItem(other->name(), other->id());
            ui.relativeScreensCombo->addItem(other->name(), other->id());
        }
    }

    // Behavior chooser
    if (output->isPrimary()) {
        ui.behaviorCombo->setCurrentIndex(PrimaryDisplay);
    } else if (!output->clone()) {
        // Is this right?
        ui.behaviorCombo->setCurrentIndex(CloneDisplay);
        int idx = ui.resolutionCombo->findData(output->clone()->id());
        ui.clonesCombo->setCurrentIndex(idx);
    } else {
        ui.behaviorCombo->setCurrentIndex(ExtendDisplay);
        ui.xPosSpinBox->setValue(output->pos().x());
        ui.yPosSpinBox->setValue(output->pos().y());
    }

    connect(ui.behaviorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onBehaviorChanged(int)));
    connect(ui.positioningCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onPositioningChanged(int)));
    connect(ui.xPosSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPositionChanged(int)));
    connect(ui.yPosSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPositionChanged(int)));

    // Force update behavior visibility
    onBehaviorChanged(ui.behaviorCombo->currentIndex());
}


void MonitorWidget::onBehaviorChanged(int idx) {
    // Behavior should match the index of the selected element
    ui.positioningCombo->setVisible(idx == ExtendDisplay);
    ui.clonesCombo->setVisible(idx == CloneDisplay);
    ui.relativeScreensCombo->setVisible(idx == ExtendDisplay);
    ui.xPosSpinBox->setVisible(idx == ExtendDisplay);
    ui.yPosSpinBox->setVisible(idx == ExtendDisplay);
    ui.relativeScreensCombo->setEnabled(true);

    output->setPrimary(idx == PrimaryDisplay);
}


void MonitorWidget::onPositioningChanged(int idx) {
    // Update the x/y spinboxes with the correct values
    KScreen::OutputPtr other = getOutputById(
        ui.relativeScreensCombo->currentData().toInt(),
        config->outputs()
    );

    if (!other->currentMode()) {
        // TODO: Figure out what to do here
        return;
    }
    QSize otherSize = other->currentMode()->size();
    QSize thisSize = output->currentMode()->size();

    int x = other->pos().x();
    int y = other->pos().y();

    switch (idx) {
    case RightOf:
        x += otherSize.width();
        break;
    case LeftOf:
        x += thisSize.width();
        break;
    case Above:
        y += otherSize.height();
        break;
    case Below:
        y += thisSize.height();
        break;
    case Manually:
    default:
        break;
    }
    ui.xPosSpinBox->setValue(x);
    ui.yPosSpinBox->setValue(y);
    // Disable the other screens combo box if we don't need it
    ui.relativeScreensCombo->setEnabled(idx && idx != Manually);
}


void MonitorWidget::onPositionChanged(int value) {
    output->setPos(QPoint(ui.xPosSpinBox->value(), ui.yPosSpinBox->value()));
}


void MonitorWidget::onResolutionChanged(int index) {
    updateRefreshRates();
    output->setCurrentModeId(ui.resolutionCombo->currentData().toString());
}


void MonitorWidget::updateRefreshRates() {
    ui.rateCombo->clear();
    KScreen::ModePtr selectedMode = output->currentMode(); // XXX That's wrong
    Q_FOREACH(const KScreen::ModePtr &mode, output->modes()) {
        if (selectedMode && mode->size() == selectedMode->size()) {
            ui.rateCombo->addItem(tr("%1 Hz").arg(mode->refreshRate()), mode->id());
        }
    }
}


void MonitorWidget::setOnlyMonitor(bool isOnlyMonitor) {
    ui.enabledCheckbox->setEnabled(!isOnlyMonitor);
    ui.enabledCheckbox->setEnabled(!isOnlyMonitor);
    ui.behaviorCombo->setEnabled(!isOnlyMonitor);
    ui.xPosSpinBox->setVisible(!isOnlyMonitor);
    ui.yPosSpinBox->setVisible(!isOnlyMonitor);
    ui.relativeScreensCombo->setVisible(!isOnlyMonitor);
    ui.clonesCombo->setVisible(!isOnlyMonitor);
}
