/*
 *  Copyright (C) 2010 Celarek Adam <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_selector_settings.h"
#include "ui_wdg_color_selector_settings.h"

KisColorSelectorSettings::KisColorSelectorSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KisColorSelectorSettings)
{
    ui->setupUi(this);
    ui->lbl_lastUsedNumRows->hide();
    ui->lastUsedColorsNumRows->hide();

    ui->lbl_commonColorsNumCols->hide();
    ui->commonColorsNumCols->hide();
    resize(minimumSize());
}

KisColorSelectorSettings::~KisColorSelectorSettings()
{
    delete ui;
}

void KisColorSelectorSettings::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}