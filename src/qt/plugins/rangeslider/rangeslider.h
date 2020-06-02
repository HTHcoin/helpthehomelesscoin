// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2017-2018 The Swipp developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class RangeSlider : public QWidget
{
    Q_OBJECT

public:
    RangeSlider(QWidget* aParent = Q_NULLPTR);

    QSize minimumSizeHint() const override;

    int GetMinimun() const;
    int GetMaximun() const;
    int GetLowerValue() const;
    int GetUpperValue() const;

    void setRange(int aMinimum, int aMaximum);

protected:
    void paintEvent(QPaintEvent* aEvent) override;
    void mousePressEvent(QMouseEvent* aEvent) override;
    void mouseMoveEvent(QMouseEvent* aEvent) override;
    void mouseReleaseEvent(QMouseEvent* aEvent) override;
    void changeEvent(QEvent* aEvent) override;

    QRectF firstHandleRect() const;
    QRectF secondHandleRect() const;
    QRectF handleRect(int aValue) const;

signals:
    void lowerValueChanged(int aLowerValue);
    void upperValueChanged(int aUpperValue);
    void valueChanged(int aLowerValue, int aUpperValue);
    void rangeChanged(int aMin, int aMax);

public slots:
    void setLowerValue(int aLowerValue, bool fireEvent = true);
    void setUpperValue(int aUpperValue, bool fireEvent = true);
    void setMinimum(int aMinimum);
    void setMaximum(int aMaximum);

private:
    Q_DISABLE_COPY(RangeSlider)
    float currentPercentage();
    int validWidth() const;

    int mMinimum;
    int mMaximum;
    int mLowerValue;
    int mUpperValue;
    bool mFirstHandlePressed;
    bool mSecondHandlePressed;
    int mInterval;
    int mDelta;
    QColor mBackgroudColorEnabled;
    QColor mBackgroudColorDisabled;
    QColor mBackgroudColor;
};
