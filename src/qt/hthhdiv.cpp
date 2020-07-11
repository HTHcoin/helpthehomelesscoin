// Copyright (c) 2018-2019 The Blocknet developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/hthhdiv.h>

HTHHDiv::HTHHDiv(QLabel *parent) : QLabel(parent) {
    this->setObjectName("hdiv");
    this->setFixedHeight(1);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}
