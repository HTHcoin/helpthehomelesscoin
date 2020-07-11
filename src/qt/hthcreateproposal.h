// Copyright (c) 2018-2019 The Blocknet developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BLOCKNET_QT_BLOCKNETCREATEPROPOSAL_H
#define BLOCKNET_QT_BLOCKNETCREATEPROPOSAL_H

#include <qt/breadcrumb.h>
#include <qt/hthcreateproposalutil.h>
#include <qt/hthcreateproposal1.h>
#include <qt/hthcreateproposal2.h>
#include <qt/hthcreateproposal3.h>

#include <QFrame>
#include <QSet>
#include <QVBoxLayout>
#include <QWidget>

class HTHCreateProposal : public QFrame
{
    Q_OBJECT

public:
    explicit HTHCreateProposal(QWidget *parent = nullptr);
    void setWalletModel(WalletModel *w) { walletModel = w; }
    void clear() {
        page1->clear();
        page2->clear();
        page3->clear();
    }

Q_SIGNALS:
    void done();

protected:
    bool event(QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void showEvent(QShowEvent *event) override;

private Q_SLOTS:
    void crumbChanged(int crumb);
    void nextCrumb(int crumb);
    void prevCrumb(int crumb);
    void onCancel(int crumb);
    void onDone();
    void reset();

private:
    QVector<HTHCreateProposalPage*> pages;
    WalletModel *walletModel = nullptr;

    QVBoxLayout *layout;
    HTHCreateProposal1 *page1;
    HTHCreateProposal2 *page2;
    HTHCreateProposal3 *page3;
    BreadCrumb *breadCrumb;
    HTHCreateProposalPage *screen = nullptr;

    void positionCrumb(QPoint pt = QPoint());
    void goToDone();
};

#endif // BLOCKNET_QT_BLOCKNETCREATEPROPOSAL_H
