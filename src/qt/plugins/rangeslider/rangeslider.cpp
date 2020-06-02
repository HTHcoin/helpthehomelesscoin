// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2017-2018 The Swipp developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QApplication>

#include "rangeslider.h"

namespace
{
    const int scHandleSideLength = 15;
    const int scSliderBarHeight = 5;
    const int scLeftRightMargin = 1;
}

RangeSlider::RangeSlider(QWidget* aParent) : QWidget(aParent), mMinimum(0), mMaximum(100),
      mLowerValue(0), mUpperValue(100),
      mFirstHandlePressed(false), mSecondHandlePressed(false),
      mInterval(mMaximum - mMinimum),
      mBackgroudColorEnabled(QApplication::palette().color(QPalette::Highlight)),
      mBackgroudColorDisabled(QApplication::palette().color(QPalette::Dark)),
      mBackgroudColor(mBackgroudColorEnabled)
{
    setMouseTracking(true);
}

void RangeSlider::paintEvent(QPaintEvent* aEvent)
{
    Q_UNUSED(aEvent);
    QPainter painter(this);

    // Background
    QRectF backgroundRect = QRectF(scLeftRightMargin, (height() - scSliderBarHeight) / 2, width() - scLeftRightMargin * 2, scSliderBarHeight);
    QPen pen(QApplication::palette().color(QPalette::Dark), 0.8);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Qt4CompatiblePainting);
    QBrush backgroundBrush(QApplication::palette().color(QPalette::Mid));
    painter.setBrush(backgroundBrush);
    painter.drawRoundedRect(backgroundRect, 1, 1);

    // First value handle rect
    pen.setColor(QApplication::palette().color(QPalette::Dark));
    pen.setWidth(0.4);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);
    QBrush handleBrush(QApplication::palette().color(QPalette::Light));
    painter.setBrush(handleBrush);
    QRectF leftHandleRect = firstHandleRect();
    painter.drawRoundedRect(leftHandleRect, 6, 6);

    // Second value handle rect
    QRectF rightHandleRect = secondHandleRect();
    painter.drawRoundedRect(rightHandleRect, 6, 6);

    // Handles
    painter.setRenderHint(QPainter::Antialiasing, false);
    QRectF selectedRect(backgroundRect);
    selectedRect.setLeft(leftHandleRect.right() + 0.5);
    selectedRect.setRight(rightHandleRect.left() - 0.5);
    QBrush selectedBrush(mBackgroudColor);
    painter.setBrush(selectedBrush);
    painter.drawRect(selectedRect);
}

QRectF RangeSlider::firstHandleRect() const
{
    float percentage = (mLowerValue - mMinimum) * 1.0 / mInterval;
    return handleRect(percentage * validWidth() + scLeftRightMargin);
}

QRectF RangeSlider::secondHandleRect() const
{
    float percentage = (mUpperValue - mMinimum) * 1.0 / mInterval;
    return handleRect(percentage * validWidth() + scLeftRightMargin + scHandleSideLength);
}

QRectF RangeSlider::handleRect(int aValue) const
{
    return QRect(aValue, (height()-scHandleSideLength) / 2, scHandleSideLength, scHandleSideLength);
}

void RangeSlider::mousePressEvent(QMouseEvent* aEvent)
{
    if(aEvent->buttons() & Qt::LeftButton)
    {
        mSecondHandlePressed = secondHandleRect().contains(aEvent->pos());
        mFirstHandlePressed = !mSecondHandlePressed && firstHandleRect().contains(aEvent->pos());

        if(mFirstHandlePressed)
        {
            mDelta = aEvent->pos().x() - (firstHandleRect().x() + scHandleSideLength / 2);
        }
        else if(mSecondHandlePressed)
        {
            mDelta = aEvent->pos().x() - (secondHandleRect().x() + scHandleSideLength / 2);
        }

        if(aEvent->pos().y() >= 2 && aEvent->pos().y() <= height()- 2)
        {
            int step = mInterval / 10 < 1 ? 1 : mInterval / 10;
            if(aEvent->pos().x() < firstHandleRect().x())
            {
                setLowerValue(mLowerValue - step);
            }
            else if(aEvent->pos().x() > firstHandleRect().x() + scHandleSideLength && aEvent->pos().x() < secondHandleRect().x())
            {
                if(aEvent->pos().x() - (firstHandleRect().x() + scHandleSideLength) <
                   (secondHandleRect().x() - (firstHandleRect().x() + scHandleSideLength)) / 2)
                {
                    if(mLowerValue + step < mUpperValue)
                        setLowerValue(mLowerValue + step);
                    else
                        setLowerValue(mUpperValue);
                }
                else
                {
                    if(mUpperValue - step > mLowerValue)
                        setUpperValue(mUpperValue - step);
                    else
                        setUpperValue(mLowerValue);
                }
            }
            else if(aEvent->pos().x() > secondHandleRect().x() + scHandleSideLength)
                setUpperValue(mUpperValue + step);
        }
    }
}

void RangeSlider::mouseMoveEvent(QMouseEvent* aEvent)
{
    if(aEvent->buttons() & Qt::LeftButton)
    {
        if(mFirstHandlePressed)
        {
            if(aEvent->pos().x() - mDelta + scHandleSideLength / 2 <= secondHandleRect().x())
                setLowerValue((aEvent->pos().x() - mDelta - scLeftRightMargin -
                              scHandleSideLength / 2) * 1.0 / validWidth() * mInterval + mMinimum, false);
            else
                setLowerValue(mUpperValue, false);
        }
        else if(mSecondHandlePressed)
        {
            if(firstHandleRect().x() + scHandleSideLength * 1.5 <= aEvent->pos().x() - mDelta)
            {
                setUpperValue((aEvent->pos().x() - mDelta - scLeftRightMargin -
                              scHandleSideLength / 2 - scHandleSideLength) * 1.0 / validWidth() * mInterval + mMinimum, false);
            }
            else
                setUpperValue(mLowerValue, false);
        }
    }
}

void RangeSlider::mouseReleaseEvent(QMouseEvent* aEvent)
{
    Q_UNUSED(aEvent);

    if (mFirstHandlePressed)
    {
        emit lowerValueChanged(mLowerValue);
        emit valueChanged(mLowerValue, mUpperValue);
    }
    else if (mSecondHandlePressed)
    {
        emit upperValueChanged(mUpperValue);
        emit valueChanged(mLowerValue, mUpperValue);
    }

    mFirstHandlePressed = false;
    mSecondHandlePressed = false;
}

void RangeSlider::changeEvent(QEvent* aEvent)
{
    if(aEvent->type() == QEvent::EnabledChange)
    {
        if(isEnabled())
            mBackgroudColor = mBackgroudColorEnabled;
        else
            mBackgroudColor = mBackgroudColorDisabled;

        update();
    }
}

QSize RangeSlider::minimumSizeHint() const
{
    return QSize(scHandleSideLength * 2 + scLeftRightMargin * 2, scHandleSideLength);
}

int RangeSlider::GetMinimun() const
{
    return mMinimum;
}

int RangeSlider::GetMaximun() const
{
    return mMaximum;
}

int RangeSlider::GetLowerValue() const
{
    return mLowerValue;
}

void RangeSlider::setLowerValue(int aLowerValue, bool fireEvent)
{
    if(aLowerValue > mMaximum)
        aLowerValue = mMaximum;

    if(aLowerValue < mMinimum)
        aLowerValue = mMinimum;

    mLowerValue = aLowerValue;

    if (mLowerValue == mUpperValue)
        mLowerValue--;

    if (fireEvent)
    {
        emit lowerValueChanged(mLowerValue);
        emit valueChanged(mLowerValue, mUpperValue);
    }

    update();
}

int RangeSlider::GetUpperValue() const
{
    return mUpperValue;
}

void RangeSlider::setUpperValue(int aUpperValue, bool fireEvent)
{
    if(aUpperValue > mMaximum)
        aUpperValue = mMaximum;

    if(aUpperValue < mMinimum)
        aUpperValue = mMinimum;

    mUpperValue = aUpperValue;

    if (mUpperValue == mLowerValue)
        mUpperValue++;

    if (fireEvent)
    {
        emit lowerValueChanged(mLowerValue);
        emit valueChanged(mLowerValue, mUpperValue);
    }

    update();
}

void RangeSlider::setMinimum(int aMinimum)
{
    if(aMinimum <= mMaximum)
        mMinimum = aMinimum;
    else
    {
        int oldMax = mMaximum;
        mMinimum = oldMax;
        mMaximum = aMinimum;
    }

    mInterval = mMaximum - mMinimum;
    update();

    if (mLowerValue < mMinimum)
        setLowerValue(mMinimum);

    if (mUpperValue > mMaximum)
        setUpperValue(mMaximum);

    emit rangeChanged(mMinimum, mMaximum);
}

void RangeSlider::setMaximum(int aMaximum)
{
    if(aMaximum >= mMinimum)
        mMaximum = aMaximum;
    else
    {
        int oldMin = mMinimum;
        mMaximum = oldMin;
        mMinimum = aMaximum;
    }

    mInterval = mMaximum - mMinimum;
    update();

    if (mLowerValue < mMinimum)
        setLowerValue(mMinimum);

    if (mUpperValue > mMaximum)
        setUpperValue(mMaximum);

    emit rangeChanged(mMinimum, mMaximum);
}

int RangeSlider::validWidth() const
{
    return width() - scLeftRightMargin * 2 - scHandleSideLength * 2;
}

void RangeSlider::setRange(int aMinimum, int aMaximum)
{
    setMinimum(aMinimum);
    setMaximum(aMaximum);
}
