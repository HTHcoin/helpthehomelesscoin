// Copyright (c) 2018-2020 The Blocknet developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/hthcreateproposal.h>

#include <qt/guiutil.h>

#include <QEvent>
#include <QLineEdit>

enum HTHCreateProposalCrumbs {
    CREATE = 1,
    REVIEW,
    SUBMIT,
};

HTHCreateProposal::HTHCreateProposal(QWidget *parent) : QFrame(parent), layout(new QVBoxLayout) {
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->setContentsMargins(QMargins());
    this->setLayout(layout);

    page1 = new HTHCreateProposal1(CREATE);
    page2 = new HTHCreateProposal2(REVIEW);
    page3 = new HTHCreateProposal3(SUBMIT);
    pages = { page1, page2, page3 };

    breadCrumb = new BreadCrumb;
    breadCrumb->setParent(this);
    breadCrumb->addCrumb(tr("Create Proposal"), CREATE);
    breadCrumb->addCrumb(tr("Pay Submission Fee"), REVIEW);
    breadCrumb->addCrumb(tr("Submit Proposal"), SUBMIT);
    breadCrumb->show();

    connect(breadCrumb, &BreadCrumb::crumbChanged, this, &HTHCreateProposal::crumbChanged);
    connect(page1, &HTHCreateProposalPage::next, this, &HTHCreateProposal::nextCrumb);
    connect(page2, &HTHCreateProposalPage::next, this, &HTHCreateProposal::nextCrumb);
    connect(page3, &HTHCreateProposalPage::next, this, &HTHCreateProposal::nextCrumb);
    connect(page2, &HTHCreateProposalPage::back, this, &HTHCreateProposal::prevCrumb);
    connect(page1, &HTHCreateProposalPage::cancel, this, &HTHCreateProposal::onCancel);
    connect(page2, &HTHCreateProposalPage::cancel, this, &HTHCreateProposal::onCancel);
    connect(page3, &HTHCreateProposalPage::cancel, this, &HTHCreateProposal::onCancel);
    connect(page3, &HTHCreateProposal3::done, this, &HTHCreateProposal::onDone);

    // Estimated position
    positionCrumb(QPoint(BGU::spi(175), BGU::spi(-4)));
    breadCrumb->goToCrumb(CREATE);
}

void HTHCreateProposal::focusInEvent(QFocusEvent *event) {
    QWidget::focusInEvent(event);
    if (screen)
        screen->setFocus();
}

void HTHCreateProposal::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    if (screen)
        screen->setFocus();
}

void HTHCreateProposal::crumbChanged(int crumb) {
    // Prevent users from jumping around the crumb widget without validating previous pages
    auto validatePages = [](const int toPage, const QVector<HTHCreateProposalPage*> &pages) -> bool {
        if (toPage - 1 > pages.count())
            return false;
        for (int i = 0; i < toPage - 1; ++i) {
            auto *page = pages[i];
            if (!page->validated())
                return false;
        }
        return true;
    };

    if (screen && crumb > breadCrumb->getCrumb() && breadCrumb->showCrumb(breadCrumb->getCrumb()) && !validatePages(crumb, pages))
        return;
    breadCrumb->showCrumb(crumb);

    if (screen) {
        layout->removeWidget(screen);
        screen->hide();
    }

    breadCrumb->setEnabled(true);

    switch(crumb) {
        case CREATE:
            screen = page1;
            break;
        case REVIEW:
            screen = page2;
            page2->setModel(page1->getModel());
            break;
        case SUBMIT:
            screen = page3;
            page3->setModel(page2->getModel());
            breadCrumb->setEnabled(false); // don't let the user change screens here (unless explicit cancel), to prevent them from inadvertently losing fee hash
            break;
        default:
            return;
    }

    layout->addWidget(screen);

    positionCrumb();
    screen->setWalletModel(walletModel);
    screen->show();
    screen->setFocus();
}

void HTHCreateProposal::nextCrumb(int crumb) {
    if (screen && crumb > breadCrumb->getCrumb() && breadCrumb->showCrumb(breadCrumb->getCrumb()) && !screen->validated())
        return;
    if (crumb >= SUBMIT) // do nothing if at the end
        return;
    breadCrumb->goToCrumb(++crumb);
}

void HTHCreateProposal::prevCrumb(int crumb) {
    if (!screen)
        return;
    if (crumb <= CREATE) // do nothing if at the beginning
        return;
    breadCrumb->goToCrumb(--crumb);
}

void HTHCreateProposal::onCancel(int crumb) {
    reset();
    Q_EMIT done();
}

void HTHCreateProposal::onDone() {
    reset();
    Q_EMIT done();
}

bool HTHCreateProposal::event(QEvent *event) {
    if (screen && event->type() == QEvent::LayoutRequest) {
        positionCrumb();
    } else if (event->type() == QEvent::Type::MouseButtonPress) {
        auto *w = this->focusWidget();
        if (dynamic_cast<const QLineEdit*>(w) != nullptr && w->hasFocus() && !w->underMouse())
            w->clearFocus();
    }
    return QFrame::event(event);
}

void HTHCreateProposal::reset() {
    for (HTHCreateProposalPage *page : pages)
        page->clear();
    breadCrumb->goToCrumb(CREATE);
}

void HTHCreateProposal::positionCrumb(QPoint pt) {
    if (pt != QPoint() || pt.x() > BGU::spi(250) || pt.y() > 0) {
        breadCrumb->move(pt);
        breadCrumb->raise();
        return;
    }
    auto *pageHeading = screen->findChild<QWidget*>("h4", Qt::FindDirectChildrenOnly);
    QPoint npt = pageHeading->mapToGlobal(QPoint(pageHeading->width(), 0));
    npt = this->mapFromGlobal(npt);
    breadCrumb->move(npt.x() + BGU::spi(20), npt.y() + pageHeading->height() - breadCrumb->height() - BGU::spi(2));
    breadCrumb->raise();
}

void HTHCreateProposal::goToDone() {
    layout->removeWidget(screen);
    screen->hide();
}

