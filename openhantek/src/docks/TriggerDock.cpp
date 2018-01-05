// SPDX-License-Identifier: GPL-2.0+

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDockWidget>
#include <QLabel>
#include <QSignalBlocker>

#include <cmath>

#include "TriggerDock.h"
#include "dockwindows.h"

#include "settings.h"
#include "sispinbox.h"
#include "utils/dsoStrings.h"
#include "utils/printutils.h"

TriggerDock::TriggerDock(DsoSettings *settings, const std::vector<std::string> &specialTriggers, QWidget *parent,
                         Qt::WindowFlags flags)
    : QDockWidget(tr("Trigger"), parent, flags), settings(settings) {

    // Initialize lists for comboboxes
    for (ChannelID channel = 0; channel < settings->deviceSpecification->channels; ++channel)
        this->sourceStandardStrings << tr("CH%1").arg(channel + 1);
    for(const std::string& name: specialTriggers)
        this->sourceSpecialStrings.append(QString::fromStdString(name));

    // Initialize elements
    this->modeLabel = new QLabel(tr("Mode"));
    this->modeComboBox = new QComboBox();
    for (Dso::TriggerMode mode: Dso::TriggerModeEnum)
        this->modeComboBox->addItem(Dso::triggerModeString(mode));

    this->slopeLabel = new QLabel(tr("Slope"));
    this->slopeComboBox = new QComboBox();
    for (Dso::Slope slope: Dso::SlopeEnum)
        this->slopeComboBox->addItem(Dso::slopeString(slope));

    this->sourceLabel = new QLabel(tr("Source"));
    this->sourceComboBox = new QComboBox();
    this->sourceComboBox->addItems(this->sourceStandardStrings);
    this->sourceComboBox->addItems(this->sourceSpecialStrings);

    this->dockLayout = new QGridLayout();
    this->dockLayout->setColumnMinimumWidth(0, 64);
    this->dockLayout->setColumnStretch(1, 1);
    this->dockLayout->addWidget(this->modeLabel, 0, 0);
    this->dockLayout->addWidget(this->modeComboBox, 0, 1);
    this->dockLayout->addWidget(this->sourceLabel, 1, 0);
    this->dockLayout->addWidget(this->sourceComboBox, 1, 1);
    this->dockLayout->addWidget(this->slopeLabel, 2, 0);
    this->dockLayout->addWidget(this->slopeComboBox, 2, 1);

    this->dockWidget = new QWidget();
    SetupDockWidget(this, dockWidget, dockLayout);

    // Connect signals and slots
    connect(this->modeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) {
        this->settings->scope.trigger.mode = (Dso::TriggerMode)index;
        emit modeChanged(this->settings->scope.trigger.mode);
    });
    connect(this->slopeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) {
        this->settings->scope.trigger.slope = (Dso::Slope)index;
        emit slopeChanged(this->settings->scope.trigger.slope);
    });
    connect(this->sourceComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) {
        bool special = false;

        if (index >= this->sourceStandardStrings.count()) {
            index -= this->sourceStandardStrings.count();
            special = true;
        }

        this->settings->scope.trigger.source = (unsigned) index;
        this->settings->scope.trigger.special = special;
        emit sourceChanged(special, (unsigned)index);
    });

    // Set values
    this->setMode(settings->scope.trigger.mode);
    this->setSlope(settings->scope.trigger.slope);
    this->setSource(settings->scope.trigger.special, settings->scope.trigger.source);
}

/// \brief Don't close the dock, just hide it
/// \param event The close event that should be handled.
void TriggerDock::closeEvent(QCloseEvent *event) {
    this->hide();

    event->accept();
}

void TriggerDock::setMode(Dso::TriggerMode mode) {
    QSignalBlocker blocker(modeComboBox);
    modeComboBox->setCurrentIndex((int)mode);
}

void TriggerDock::setSlope(Dso::Slope slope) {
    QSignalBlocker blocker(slopeComboBox);
    slopeComboBox->setCurrentIndex((int)slope);
}

void TriggerDock::setSource(bool special, unsigned int id) {
    if ((!special && id >= (unsigned int)this->sourceStandardStrings.count()) ||
        (special && id >= (unsigned int)this->sourceSpecialStrings.count()))
        return;

    int index = (int)id;
    if (special) index += this->sourceStandardStrings.count();
    QSignalBlocker blocker(sourceComboBox);
    sourceComboBox->setCurrentIndex(index);
}
