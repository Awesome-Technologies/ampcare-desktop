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

namespace OCC {

MessageModel::MessageModel(const QString &rootPath, const QString &currentUser, QObject *parent)
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

    // data for detailView (html)
    case DetailRole:
        return QVariant(_messageItem.details());

    // current status of message
    case StatusRole:
        return QVariant(_messageItem.status);
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
            } else {
                return true;
            }
        }

        // check if the sender clicked the message
        if (_messageItem.sender == value.toString()) {
            if (_messageItem.status == MessageObject::ResentStatus) {
                _messageItem.status = MessageObject::RereadStatus;
                break;
            } else {
                return true;
            }
        }
        // a message should be either from the sender or the recipient
        return false;
    }

    if (_messageItem.saveMessage(_rootPath, _currentUser, false)) {
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

void MessageModel::addEntities()
{
    _watcher.addPath(_rootPath);
    for (const QFileInfo &info : QDir(_rootPath).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        qInfo() << "entity" << info.absoluteFilePath();
        addMessages(info.absoluteFilePath());
    }
}

void MessageModel::addMessages(const QString &path)
{
    QString messagePath = path + "/messages";
    _watcher.addPath(messagePath);
    for (const QFileInfo &info : QDir(messagePath).entryInfoList(_filters, QDir::Files)) {
        qInfo() << "add" << info.absoluteFilePath();
        _messageList << MessageObject(info.absoluteFilePath());
    }
}

void MessageModel::onDirectoryChanged(const QString &path)
{
    qInfo() << "directory changed" << path;
    _watcher.removePaths(_watcher.directories());
    _messageList.clear();
    addEntities();
}

} // end namespace OCC
