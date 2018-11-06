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

#include "answermessagedialog.h"
#include "createmessagedialog.h"
#include "messagemodel.h"
#include "messageswindow.h"
#include "styledhtmldelegate.h"
#include "ui_messageswindow.h"

#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QFileInfo>

namespace OCC {

MessagesWindow::MessagesWindow(const Sharee &_currentUser,
    const QString &localPath,
    const QVector<QSharedPointer<Sharee>> &recipientList,
    QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MessagesWindow)
    , messageModel(new MessageModel(localPath + "/AMP", _currentUser, this))
    , filterProxy(new QSortFilterProxyModel(this))
    , _answerMessageDialog(new AnswerMessageDialog(_currentUser, localPath + "/AMP", filterProxy, this))
    , _createMessageDialog(new CreateMessageDialog(recipientList, localPath + "/AMP", _currentUser, this))
    , currentUser(_currentUser)
    , localPath(localPath)

{
    ui->setupUi(this);

    // dialog for creating new messages
    connect(ui->createMessageButton, SIGNAL(clicked()), this, SLOT(on_createMessageButton_clicked()));
    filterProxy->setSourceModel(messageModel);
    filterProxy->setSortRole(MessageModel::SortRole);
    filterProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    filterProxy->setFilterRole(MessageModel::ArchivedForRole);
    filterProxy->setFilterRegExp(QRegExp("false", Qt::CaseInsensitive, QRegExp::FixedString));

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

void MessagesWindow::slotShowDetails(const QModelIndex &current, const QModelIndex &previous)
{
    // selection has changed -> disable resolved and archive button
    ui->resolvedButton->setEnabled(false);
    ui->archiveButton->setEnabled(false);

    ui->detailView->setHtml(filterProxy->data(current, MessageModel::DetailRole).toString().toUtf8(), QUrl("qrc:/"));

    // update status only if selection has changed
    if (current.row() != previous.row()) {
        // set message status to 'read'
        filterProxy->setData(current, currentUser.shareWith(), MessageModel::StatusRole);
    }

    // is message 'read' and the user == receiver
    MessageObject _messageItem(filterProxy->data(current, MessageModel::MessageObjectRole).value<MessageObject>());
    if ((_messageItem.status == MessageObject::ReadStatus || _messageItem.status == MessageObject::RereadStatus) && _messageItem.recipient == currentUser.shareWith()) {
        ui->resolvedButton->setEnabled(true);
    }

    // is message 'resolved' ?
    if (_messageItem.status == MessageObject::ResolvedStatus) {
        ui->archiveButton->setEnabled(true);
    }
}

void MessagesWindow::on_messageList_doubleClicked(const QModelIndex &index)
{
    int _currentStatus = filterProxy->data(index, MessageModel::StatusRole).toInt();
    MessageObject _messageItem(filterProxy->data(index, MessageModel::MessageObjectRole).value<MessageObject>());

    if (_currentStatus == MessageObject::DraftStatus) { // message is a draft
        // load values into dialog
        _createMessageDialog->reset();
        _createMessageDialog->setValues(_messageItem);
        _createMessageDialog->show();
    } else { // message was sent, only answers are allowed
        _answerMessageDialog->reset();
        _answerMessageDialog->setMessageObject(_messageItem);
        _answerMessageDialog->setValues(filterProxy->data(index, MessageModel::DetailRole).toString());
        _answerMessageDialog->show();
    }
}

void MessagesWindow::on_createMessageButton_clicked()
{
    // reset dialog/messageId
    _createMessageDialog->reset();
    _createMessageDialog->show();
}

void MessagesWindow::on_archiveButton_clicked()
{
    // get filename and path for export
    QString fileName = QFileDialog::getSaveFileName((QWidget *)0, "Export PDF", QString(), "*.pdf");
    if (QFileInfo(fileName).suffix().isEmpty()) {
        fileName.append(".pdf");
    }

    // print content of detailView to pdf
    ui->detailView->page()->printToPdf(fileName);

    // set message status to 'archived'
    filterProxy->setData(ui->messageList->currentIndex(), "", MessageModel::MessageArchivedRole);
}

void MessagesWindow::on_resolvedButton_clicked()
{
    // set message status to 'resolved'
    filterProxy->setData(ui->messageList->currentIndex(), "", MessageModel::MessageResolvedRole);
}

void MessagesWindow::on_videocallButton_clicked()
{
    MessageObject _messageItem(filterProxy->data(ui->messageList->currentIndex(), MessageModel::MessageObjectRole).value<MessageObject>());
    QString _callRecipient = _messageItem.sender;
    // change callRecipient of video window and show it
    emit callRecipientChanged(_callRecipient);
}


} // end namespace
