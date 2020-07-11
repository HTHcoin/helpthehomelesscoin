// Copyright (c) 2018-2019 The Blocknet developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/hthlineeditwithtitle.h>

#include <qt/guiutil.h>

#include <QEvent>

HTHLineEditWithTitle::HTHLineEditWithTitle(QString title, QString placeholder, int w, QFrame *parent)
                                                    : QFrame(parent), layout(new QVBoxLayout)
{
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    this->setContentsMargins(QMargins());
    layout->setContentsMargins(QMargins());
    layout->setSpacing(BGU::spi(3));
    this->setLayout(layout);

    titleLbl = new QLabel(title);
    titleLbl->setObjectName("lineEditTitle");
    titleLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(titleLbl);

    lineEdit = new BlocknetLineEdit(w);
    lineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    lineEdit->setPlaceholderText(placeholder);
    layout->addWidget(lineEdit, 1);

    this->setFocusProxy(lineEdit);
}

QSize HTHLineEditWithTitle::sizeHint() const {
    return { lineEdit->width(), BGU::spi(30) + layout->spacing() + titleLbl->height() };
}

bool HTHLineEditWithTitle::isEmpty() {
    return lineEdit->text().trimmed().isEmpty();
}

void HTHLineEditWithTitle::setID(const QString id) {
    this->id = id;
}

QString HTHLineEditWithTitle::getID() {
    return this->id;
}

void HTHLineEditWithTitle::setError(bool flag) {
    lineEdit->setProperty("error", flag);
}

void HTHLineEditWithTitle::setTitle(const QString &title) {
    titleLbl->setText(title);
}

void HTHLineEditWithTitle::setExpanding() {
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    this->lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}
