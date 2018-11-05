/*
 * Copyright (C) by Michael Albert <michael.albert@awesome-technologies.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "messagemodel.h"
#include "messageswindow.h"
#include "styledhtmldelegate.h"
#include "ui_messageswindow.h"

#include <QSortFilterProxyModel>

namespace OCC {

MessagesWindow::MessagesWindow(const QString &_currentUser,
    const QString &localPath,
    QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MessagesWindow)
    , messageModel(new MessageModel(localPath + "/AMP", _currentUser, this))
    , filterProxy(new QSortFilterProxyModel(this))
    , currentUser(_currentUser)
    , localPath(localPath)
{
    ui->setupUi(this);

    filterProxy->setSourceModel(messageModel);
    filterProxy->setSortRole(MessageModel::SortRole);
    filterProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    // set message delegate to listview
    QTableView *msgList = ui->messageList;
    msgList->setModel(filterProxy);
    msgList->setItemDelegateForColumn(MessageModel::TitleColumn, new StyledHtmlDelegate(this));
    msgList->setColumnWidth(MessageModel::PriorityColumn, MessageObject::ICON_IMAGE_HEIGHT + 10);
    msgList->setColumnWidth(MessageModel::TitleColumn, 220);
    msgList->setColumnWidth(MessageModel::DateColumn, 110);
    msgList->setColumnWidth(MessageModel::StatusColumn, 50);
    msgList->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    msgList->horizontalHeader()->setSectionResizeMode(MessageModel::TitleColumn, QHeaderView::Stretch);

    // connect slot for showing details of message on click on the message item in the listView
    connect(ui->messageList->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(slotShowDetails(QModelIndex, QModelIndex)));
    msgList->setCurrentIndex(messageModel->index(0, 0));
}

MessagesWindow::~MessagesWindow()
{
    delete ui;
}

void MessagesWindow::slotShowDetails(const QModelIndex &current, const QModelIndex &)
{
    ui->detailView->setHtml(filterProxy->data(current, MessageModel::DetailRole).toString().toUtf8(), QUrl("qrc:/"));

    // set message status to 'read'
    filterProxy->setData(current, currentUser, MessageModel::StatusRole);
}

} // end namespace
