/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_spray_shape_option.h"
#include <klocale.h>

#include <QImage>

#include "ui_wdgshapeoptions.h"
#include "kis_spray_paintop_settings.h"

class KisShapeOptionsWidget: public QWidget, public Ui::WdgShapeOptions
{
public:
    KisShapeOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }

};

KisSprayShapeOption::KisSprayShapeOption()
        : KisPaintOpOption(i18n("Particle type"), false)
{
    m_checkable = false;
    // save this to be able to restore it back
    m_maxSize = 1000;
    
    m_options = new KisShapeOptionsWidget();
    m_useAspect = m_options->aspectButton->keepAspectRatio();
    computeAspect();

    // UI signals
    connect(m_options->aspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(aspectToggled(bool)));
    connect(m_options->randomSlider,SIGNAL(valueChanged(int)),this,SLOT(randomValueChanged(int)));
    connect(m_options->followSlider,SIGNAL(valueChanged(int)),this,SLOT(followValueChanged(int)));
    connect(m_options->imageUrl,SIGNAL(textChanged(QString)),this,SLOT(prepareImage()));

    connect(m_options->widthSpin, SIGNAL(valueChanged(int)), SLOT(updateHeight(int)));
    connect(m_options->heightSpin, SIGNAL(valueChanged(int)), SLOT(updateWidth(int)));

    connect(m_options->proportionalBox, SIGNAL(clicked(bool)), SLOT(changeSizeUI(bool)));
    
    connect(m_options->fixedRotation, SIGNAL(toggled(bool)), m_options->angleSlider, SLOT(setEnabled(bool)));
    connect(m_options->randomRotation, SIGNAL(toggled(bool)), m_options->randomSlider, SLOT(setEnabled(bool)));
    connect(m_options->followCursor, SIGNAL(toggled(bool)), m_options->followSlider, SLOT(setEnabled(bool)));
    connect(m_options->fixedRotation, SIGNAL(toggled(bool)), m_options->fixedRotationSPBox, SLOT(setEnabled(bool)));
    connect(m_options->randomRotation, SIGNAL(toggled(bool)), m_options->randomWeightSPBox, SLOT(setEnabled(bool)));
    connect(m_options->followCursor, SIGNAL(toggled(bool)), m_options->followCursorWeightSPBox, SLOT(setEnabled(bool)));
    
    setupBrushPreviewSignals();
    
    setConfigurationPage(m_options);
}


void KisSprayShapeOption::setupBrushPreviewSignals()
{
    connect(m_options->angleSlider,SIGNAL(sliderReleased()),SIGNAL(sigSettingChanged()));
    connect(m_options->shapeBox, SIGNAL(currentIndexChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->widthSpin, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->heightSpin, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->randomSizeCHBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->proportionalBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->aspectButton, SIGNAL(keepAspectRatioChanged(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->randomSlider,SIGNAL(sliderReleased()),SIGNAL(sigSettingChanged()));
    connect(m_options->followSlider,SIGNAL(sliderReleased()),SIGNAL(sigSettingChanged()));
    connect(m_options->imageUrl,SIGNAL(textChanged(QString)),SIGNAL(sigSettingChanged()));
    connect(m_options->widthSpin, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->heightSpin, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->proportionalBox, SIGNAL(clicked(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->fixedRotation, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->randomRotation, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->followCursor, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->fixedRotation, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->randomRotation, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->followCursor, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->fixedRotationSPBox, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->randomWeightSPBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->followCursorWeightSPBox, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
}


KisSprayShapeOption::~KisSprayShapeOption()
{
    // delete m_options;
}

int KisSprayShapeOption::shape() const
{
    return m_options->shapeBox->currentIndex();
}

int KisSprayShapeOption::width() const
{
    return m_options->widthSpin->value();
}

int KisSprayShapeOption::height() const
{
    return m_options->heightSpin->value();
}

bool KisSprayShapeOption::randomSize() const
{
    return m_options->randomSizeCHBox->isChecked();
}

bool KisSprayShapeOption::proportional() const
{
    return m_options->proportionalBox->isChecked();
}


void KisSprayShapeOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty(SPRAYSHAPE_SHAPE,shape());
    setting->setProperty(SPRAYSHAPE_PROPORTIONAL, proportional());
    setting->setProperty(SPRAYSHAPE_WIDTH, width());
    setting->setProperty(SPRAYSHAPE_HEIGHT, height());
    setting->setProperty(SPRAYSHAPE_RANDOM_SIZE, randomSize());
    setting->setProperty(SPRAYSHAPE_FIXED_ROTATION, fixedRotation());
    setting->setProperty(SPRAYSHAPE_FIXED_ANGEL, fixedAngle());
    setting->setProperty(SPRAYSHAPE_RANDOM_ROTATION, randomRotation());
    setting->setProperty(SPRAYSHAPE_RANDOM_ROTATION_WEIGHT, randomRotationWeight());
    setting->setProperty(SPRAYSHAPE_FOLLOW_CURSOR, followCursor());
    setting->setProperty(SPRAYSHAPE_FOLLOW_CURSOR_WEIGHT, followCursorWeigth());
    setting->setProperty(SPRAYSHAPE_IMAGE_URL,m_options->imageUrl->text());
    static_cast<KisSprayPaintOpSettings*>(setting)->setQImage(m_image);
}


void KisSprayShapeOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->shapeBox->setCurrentIndex(setting->getInt(SPRAYSHAPE_SHAPE));
    m_options->widthSpin->setValue(setting->getInt(SPRAYSHAPE_WIDTH));
    m_options->heightSpin->setValue(setting->getInt(SPRAYSHAPE_HEIGHT));
    m_options->randomSizeCHBox->setChecked(setting->getBool(SPRAYSHAPE_RANDOM_SIZE));
    m_options->proportionalBox->setChecked(setting->getBool(SPRAYSHAPE_PROPORTIONAL));
    m_options->fixedRotation->setChecked(setting->getBool(SPRAYSHAPE_FIXED_ROTATION));
    m_options->fixedRotationSPBox->setValue(setting->getDouble(SPRAYSHAPE_FIXED_ANGEL));
    m_options->followCursor->setChecked(setting->getBool(SPRAYSHAPE_FOLLOW_CURSOR));
    m_options->followCursorWeightSPBox->setValue(setting->getDouble(SPRAYSHAPE_FOLLOW_CURSOR) );
    m_options->randomRotation->setChecked(setting->getBool(SPRAYSHAPE_RANDOM_ROTATION));
    m_options->randomWeightSPBox->setValue(setting->getDouble(SPRAYSHAPE_RANDOM_ROTATION_WEIGHT) );
    m_options->imageUrl->setText(setting->getString(SPRAYSHAPE_IMAGE_URL));
}

bool KisSprayShapeOption::fixedRotation() const
{
    return m_options->fixedRotation->isChecked();
}


int KisSprayShapeOption::fixedAngle() const
{
    return m_options->fixedRotationSPBox->value();
}


bool KisSprayShapeOption::followCursor() const
{
    return m_options->followCursor->isChecked();
}


qreal KisSprayShapeOption::followCursorWeigth() const
{
    return m_options->followCursorWeightSPBox->value();
}


bool KisSprayShapeOption::randomRotation() const
{
    return m_options->randomRotation->isChecked();
}


qreal KisSprayShapeOption::randomRotationWeight() const
{
    return m_options->randomWeightSPBox->value();
}


void KisSprayShapeOption::randomValueChanged(int value)
{
    qreal relative = value / (qreal)m_options->randomSlider->maximum() ;
    m_options->randomWeightSPBox->setValue( relative * m_options->randomWeightSPBox->maximum() );
}


void KisSprayShapeOption::followValueChanged(int value)
{
    qreal relative = value / (qreal)m_options->followSlider->maximum() ;
    m_options->followCursorWeightSPBox->setValue( relative * m_options->followCursorWeightSPBox->maximum() );
}


void KisSprayShapeOption::prepareImage()
{
    QString path = m_options->imageUrl->url().toLocalFile();
    if (QFile::exists(path)){
        m_image = QImage(path);
        if (!m_image.isNull())
        {
            m_options->widthSpin->setValue( m_image.width() );
            m_options->heightSpin->setValue( m_image.height() );
        }
    }
}


void KisSprayShapeOption::aspectToggled(bool toggled)
{
    m_useAspect = toggled;
}



void KisSprayShapeOption::updateHeight(int value)
{
    if (m_useAspect){
        int newHeight = qRound(value * 1.0/m_aspect);
        m_options->heightSpin->blockSignals(true);
        m_options->heightSpin->setValue(newHeight);
        m_options->heightSlider->setValue(newHeight);
        m_options->heightSpin->blockSignals(false);
    }else{
        computeAspect();
    }
}


void KisSprayShapeOption::updateWidth(int value)
{
    if (m_useAspect){
        int newWidth = qRound(value * m_aspect);
        m_options->widthSpin->blockSignals(true);
        m_options->widthSpin->setValue( newWidth );
        m_options->widthSlider->setValue( newWidth );
        m_options->widthSpin->blockSignals(false);
    }else{
        computeAspect();
    }
}


void KisSprayShapeOption::computeAspect()
{
    qreal w = m_options->widthSpin->value();
    qreal h = m_options->heightSpin->value();
    m_aspect = w / h;
}



void KisSprayShapeOption::changeSizeUI(bool proportionalSize)
{
    // if proportionalSize is false, pixel size is used
    if (!proportionalSize){
        m_options->widthSlider->setMaximum( m_maxSize );
        m_options->widthSpin->setMaximum( m_maxSize );
        m_options->widthSpin->setSuffix("");
        m_options->heightSlider->setMaximum( m_maxSize );
        m_options->heightSpin->setMaximum( m_maxSize );
        m_options->heightSpin->setSuffix("");
    }else{
        m_options->widthSlider->setMaximum( 100 );
        m_options->widthSpin->setMaximum( 100 );
        m_options->widthSpin->setSuffix("%");
        m_options->heightSlider->setMaximum( 100 );
        m_options->heightSpin->setMaximum( 100 );
        m_options->heightSpin->setSuffix("%");
    }
    
    m_options->widthSlider->setPageStep( m_options->widthSlider->maximum() / 10 );
    m_options->heightSlider->setPageStep( m_options->widthSlider->maximum() / 10 );
}
