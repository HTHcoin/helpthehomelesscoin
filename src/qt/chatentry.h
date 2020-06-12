// Copyright (c) 2011-2015 The Bitcoin Core developers
// Copyright (c) 2019- The IMAGEcOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_CHATENTRY_H
#define BITCOIN_QT_CHATENTRY_H

#include "walletmodel.h"
#include "guiutil.h"
#include <QStackedWidget>
#include <QKeyEvent>

class WalletModel;
class PlatformStyle;
class TransactionFilterProxy;
class QTableView;
class QMenu;

namespace Ui {
    class ChatEntry;
}

/**
 * A single entry in the dialog for sending bitcoins.
 * Stacked widget, with different UIs for payment requests
 * with a strong payee identity.
 */
class ChatEntry : public QStackedWidget
{
    Q_OBJECT

public:
    explicit ChatEntry(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~ChatEntry();

    void setModel(WalletModel *model);
    bool validate();
    SendCoinsRecipient getValue();

    /** Return whether the entry is still empty and unedited */
    bool isClear();

    void setValue(const SendCoinsRecipient &value);
    void setAddress(const QString &address,QString imgbase64);

    /** Set up the tab chain manually, as Qt messes up the tab chain by default in some cases
     *  (issue https://bugreports.qt-project.org/browse/QTBUG-10907).
     */
    QWidget *setupTabChain(QWidget *prev);

    void setFocus();

    enum ColumnWidths {
         STATUS_COLUMN_WIDTH = 30,
         WATCHONLY_COLUMN_WIDTH = 23,
         DATE_COLUMN_WIDTH = 120,
         TYPE_COLUMN_WIDTH = 180,
		 IMGBASE64_COLUMN_WIDTH = 580,
         AMOUNT_MINIMUM_COLUMN_WIDTH = 120,
         MINIMUM_COLUMN_WIDTH = 23
     };

public Q_SLOTS:
    void clear();

Q_SIGNALS:
    void removeEntry(ChatEntry *entry);
    void payAmountChanged();
    void subtractFeeFromAmountChanged();

private Q_SLOTS:
    void deleteClicked();
    void on_chatTo_textChanged(const QString &address);
    void on_addressBookButton_clicked();
    void on_addressReceiveBookButton_clicked();
    void on_pasteButton_clicked();
    void on_pasteReceiveAddressButton_clicked();
    void updateDisplayUnit();
    void on_chooserButton_clicked();
    void on_pasteButtonBase64_clicked();
    void copyImgbase64();
    void contextualMenu(const QPoint &);
    void on_chatReceive_textChanged(const QString &address);

private:
    SendCoinsRecipient recipient;
    Ui::ChatEntry *ui;
    WalletModel *model;
    const PlatformStyle *platformStyle;
    TransactionFilterProxy *transactionProxyModel;
    QTableView *transactionView;
    QMenu *contextMenu;
    GUIUtil::TableViewLastColumnResizingFixer *columnResizingFixer;
    bool updateLabel(const QString &address);
    void checkaddresstransactions(const QString &address);
};

#endif // BITCOIN_QT_CHATIMGENTRY_H
