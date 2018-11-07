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

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMessageBox>

namespace OCC {

MessageModel::MessageModel(const QString &rootPath, const Sharee &currentUser, QObject *parent)
    : QAbstractTableModel(parent)
    , _rootPath(rootPath)
    , _currentUser(currentUser)
{
    qInfo() << "MessageModel at" << rootPath;
    _filters << "*.json";
    addEntities();
    connect(&_watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(onDirectoryChanged(const QString &)));
}

MessageModel::~MessageModel() {}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : _messageList.size();
}

int MessageModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

QVariant MessageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case PriorityColumn:
            return " ";
        case TitleColumn:
            return tr("Title");
        case DateColumn:
            return tr("Date");
        case StatusColumn:
            return tr("Status");
        }
    }
    return QVariant();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    const MessageObject &_messageItem = _messageList.at(index.row());

    switch (role) {
    // data for viewing on listView separated into columns
    case Qt::DisplayRole:
        switch (index.column()) {
        case TitleColumn:
            return _messageItem.shortTitle();
        case DateColumn:
            return _messageItem.authoredOn;
        }
        return QVariant();

    case Qt::DecorationRole:
        switch (index.column()) {
        case PriorityColumn:
            return _messageItem.priorityIcon();
        case StatusColumn:
            return _messageItem.statusIcon();
        }
        break;

    case Qt::ToolTipRole:
        switch (index.column()) {
        case TitleColumn:
            return _messageItem.longTitle();
        case DateColumn:
            return _messageItem.authoredOn;
        case StatusColumn:
            return _messageItem.status;
        }
        break;

    case Qt::TextAlignmentRole:
        switch (index.column()) {
        case DateColumn:
            return Qt::AlignRight;
        }
        return Qt::AlignLeft;

    case SortRole:
        switch (index.column()) {
        case PriorityColumn:
            return _messageItem.priority;
        case TitleColumn:
            return _messageItem.title;
        case DateColumn:
            return _messageItem.authoredOn;
        case StatusColumn:
            return _messageItem.status;
        }
        break;

    // data for detailView (html)
    case DetailRole:
        return QVariant(_messageItem.details());

    // current status of message
    case StatusRole:
        return QVariant(_messageItem.status);

    // message object
    case MessageObjectRole:
        return QVariant::fromValue<MessageObject>(_messageItem);

    // dont show archived messages
    case ArchivedForRole:
        return QVariant(_messageItem.isArchivedFor(_currentUser.shareWith()));
    }

    return QVariant();
}

bool MessageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    MessageObject _messageItem = _messageList.value(index.row());

    switch (role) {
    // set new status - message was read
    case StatusRole:
        // check if the recipient clicked the message
        if (_messageItem.recipient == value.toString()) {
            if (_messageItem.status == MessageObject::SentStatus) {
                _messageItem.status = MessageObject::ReadStatus;
                break;
            }
            return true;
        // check if the sender clicked the message
        } else if (_messageItem.sender == value.toString()) {
            if (_messageItem.status == MessageObject::ResentStatus) {
                _messageItem.status = MessageObject::RereadStatus;
                break;
            }
            return true;
        }
        // a message should be either from the sender or the recipient
        return false;

    // set new status - message was resolved
    case MessageResolvedRole:
        _messageItem.status = MessageObject::ResolvedStatus;
        break;

    // set new status - message was archived
    case MessageArchivedRole:
        _messageItem.archivedFor.append({ _currentUser.shareWith(), QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") });
        _messageItem.status = MessageObject::ArchivedStatus;
        break;
    default:
        return false;
    }

    if (writeMessage(_messageItem)) {
        _messageList.replace(index.row(), _messageItem);
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

bool MessageModel::writeMessage(MessageObject &msg)
{
    bool isDraft = (msg.status == MessageObject::DraftStatus);
    bool wasDraft = false;
    // create new uuid if none is set
    if (msg.messageId.isNull()) {
        msg.messageId = QUuid::createUuid();
    } else {
        // if the message was a draft, delete it
        if (!isDraft) {
            QString filePath = _rootPath + "/drafts/messages/" + msg.messageId.toString() + ".json";
            if (QFileInfo::exists(filePath) && QFileInfo(filePath).isFile()) {
                QFile::remove(filePath);
                wasDraft = true;
            }
        }
    }

    // create filename and correct path
    // complete path: <_rootPath>/<recipientid>/messages/<messageId>.json

    QString _userFolder = (msg.recipient == _currentUser.shareWith()) ? msg.sender : msg.recipient;
    // save drafts to drafts folder
    QString dirPath = _rootPath + (isDraft ? QString("/drafts/") : QString("/" + _userFolder));

    // create messages directory if it doesnt exist
    QDir dir(dirPath + "/messages/");
    if (!dir.exists(dirPath + "/messages/")) {
        if (!dir.mkpath(dirPath + "/messages/")) {
            // display error
            QMessageBox msgBox;
            msgBox.setText(QObject::tr("Error on creating folder for messages!"));
            msgBox.exec();
            return false;
        }
    }

    // process images list
    for (MessageObject::ImageDetails &image : msg.imagesList) {
        // get full file path
        if (!image.path.isEmpty()) {
            QString fullFilePath = image.path;

            // create assets directory if it doesn't exist
            QDir dir(dirPath + "/assets/");
            if (!dir.exists(dirPath + "/assets/")) {
                if (!dir.mkpath(dirPath + "/assets/")) {
                    // display error
                    QMessageBox msgBox;
                    msgBox.setText(QObject::tr("Error on creating folder for assets!"));
                    msgBox.exec();
                    return false;
                }
            }

            if (wasDraft) { // move assets belonging to draft to new assets folder
                // assure filename is unique
                while (QFileInfo::exists(dirPath + "/assets/" + image.name)) {
                    image.name = QUuid::createUuid().toString() + image.name;
                }
                if (!QFile::rename(fullFilePath, dirPath + "/assets/" + image.name)) {
                    // display error
                    QMessageBox msgBox;
                    msgBox.setText(QObject::tr("Error on moving assets!"));
                    msgBox.exec();
                    return false;
                }
            } else { // copy file to assets folder
                // assure filename is unique
                while (QFileInfo::exists(dirPath + "/assets/" + image.name)) {
                    image.name = QUuid::createUuid().toString() + image.name;
                }
                if (!QFile::copy(fullFilePath, dirPath + "/assets/" + image.name)) {
                    // display error
                    QMessageBox msgBox;
                    msgBox.setText(QObject::tr("Error on copying assets!"));
                    msgBox.exec();
                    return false;
                }
            }
        }
    }

    // write contents to file
    QString filePath = dirPath + "/messages/" + msg.messageId.toString() + ".json";
    msg.path = filePath;

    QJsonObject content;
    msg.buildJson(content, isDraft);

    qInfo() << "Write JSON to" << filePath;
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument saveDoc(content);
        file.write(saveDoc.toJson());
        return true;
    }
    return false;
}

void MessageModel::addEntities()
{
    _watcher.addPath(_rootPath);
    qInfo() << "watch" << _rootPath;
    for (const QFileInfo &info : QDir(_rootPath).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        qInfo() << "entity" << info.absoluteFilePath();
        addMessages(info.absoluteFilePath());
    }
}

void MessageModel::addMessages(const QString &path)
{
    QString messagePath = path + "/messages";
    _watcher.addPath(messagePath);
    qInfo() << "watch" << messagePath;
    for (const QFileInfo &info : QDir(messagePath).entryInfoList(_filters, QDir::Files)) {
        qInfo() << "add" << info.absoluteFilePath();
        MessageObject messageObject;
        QFile file(info.absoluteFilePath());
        file.open(QIODevice::ReadOnly);
        messageObject.setJson(QJsonDocument::fromJson(file.readAll()).object());
        messageObject.path = info.absoluteFilePath();
        _messageList << messageObject;
    }
}

void MessageModel::onDirectoryChanged(const QString &path)
{
    emit layoutAboutToBeChanged();
    qInfo() << "directory changed" << path;
    _watcher.removePaths(_watcher.directories());
    _messageList.clear();
    addEntities();
    emit layoutChanged();
}

} // end namespace OCC
