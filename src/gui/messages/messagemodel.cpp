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

#include "logger.h"
#include "messagemodel.h"

#include <libsnore/snore.h>
#include <libsnore/snore_p.h>
#include <libsnore/notification/notification.h>

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMessageBox>

namespace OCC {

MessageModel::MessageModel(const QString &rootPath, const Sharee &currentUser, QObject *parent)
    : QAbstractTableModel(parent)
    , _snoreCore(&Snore::SnoreCore::instance())
    , _rootPath(rootPath)
    , _currentUser(currentUser)
{
    qInfo() << "MessageModel at" << rootPath;
    _filters << "*.json";
    addEntities();
    connect(&_watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(onDirectoryChanged(const QString &)));

    // setup notification backend
    _snoreCore = &Snore::SnoreCore::instance();
    try {
        _snoreCore->loadPlugins(Snore::SnorePlugin::Backend);
    } catch (...) {
        qDebug() << "Exception when loading plugins";
    }

    Snore::Icon appIcon = Snore::Icon(QIcon(":/client/theme/amp/logo.png"));

    _snoreCore->setSettingsValue(QStringLiteral("PrimaryBackend"), "Snore", Snore::SettingsType::LocalSetting);
    _snoreCore->setSettingsValue(QStringLiteral("Timeout"), 0, Snore::SettingsType::LocalSetting); // set default timeout for notifications to 0 (sticky)
    //SnoreNotifierSettings::setSettingsValue("Position", 2); // show notifications in bottom right corner


    _snoreApplication = new Snore::Application("AMPcare", appIcon);

    Snore::Icon criticalIcon = Snore::Icon(QIcon(":/client/theme/amp/icon_a_critical.png"));
    _snoreCriticalAlert = new Snore::Alert(QString::number(MessageObject::Priority::CriticalPriority), "newCriticalMessage", criticalIcon);
    _snoreApplication->addAlert(*_snoreCriticalAlert);

    Snore::Icon urgentIcon = Snore::Icon(QIcon(":/client/theme/amp/icon_b_urgent.png"));
    _snoreUrgentAlert = new Snore::Alert(QString::number(MessageObject::Priority::UrgentPriority), "newUrgentMessage", urgentIcon);
    _snoreApplication->addAlert(*_snoreUrgentAlert);

    Snore::Icon goodIcon = Snore::Icon(QIcon(":/client/theme/amp/icon_c_good.png"));
    _snoreGoodAlert = new Snore::Alert(QString::number(MessageObject::Priority::GoodPriority), "newGoodMessage", goodIcon);
    _snoreApplication->addAlert(*_snoreGoodAlert);

    Snore::Icon infoIcon = Snore::Icon(QIcon(":/client/theme/amp/icon_d_info.png"));
    _snoreInfoAlert = new Snore::Alert(QString::number(MessageObject::Priority::InfoPriority), "newInfoMessage", infoIcon);
    _snoreApplication->addAlert(*_snoreInfoAlert);

    _snoreApplication->hints().setValue("use-markup", true); // use html to style notifications
    _snoreApplication->hints().setValue("windows-app-id", "AMPcare"); // id for windows notification framework

    _snoreCore->registerApplication(*_snoreApplication);
    _snoreNotification = nullptr;
    _notificationCounter = 0;
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
        case RecipientColumn:
            return tr("Recipient");
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
        case RecipientColumn:
            return _messageItem.getRecipient();
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
        case RecipientColumn:
            return _messageItem.recipientName;
        case DateColumn:
            return _messageItem.authoredOn;
        case StatusColumn:
            return _messageItem.statusText;
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
        case RecipientColumn:
            return _messageItem.recipientName;
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

    // message object
    case MessageObjectRole:
        _messageItem = value.value<MessageObject>();
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
        if (index.isValid()) {
            _messageList.replace(index.row(), _messageItem);
            emit dataChanged(index, index);
        } else {
            emit layoutAboutToBeChanged();
            _messageList.insert(0, _messageItem);
            emit layoutChanged();
        }
        return true;
    }

    return false;
}

bool MessageModel::writeMessage(MessageObject &msg)
{
    QString dirPath;
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

    // message is created or was a draft
    if (msg.path.isEmpty() || wasDraft) {
        // create filename and correct path
        // complete path: <_rootPath>/<recipientid>/messages/<messageId>.json

        QString _userFolder = (msg.recipient == _currentUser.shareWith()) ? msg.sender : msg.recipient;
        // save drafts to drafts folder
        dirPath = _rootPath + (isDraft ? QString("/drafts") : QString("/" + _userFolder));

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
        msg.path = dirPath + "/messages/" + msg.messageId.toString() + ".json";
    } else {
        QString _userFolder = (msg.recipient == _currentUser.shareWith()) ? msg.sender : msg.recipient;
        // save drafts to drafts folder
        dirPath = _rootPath + QString("/" + _userFolder);
    }

    // process attachment list
    if (!msg.newImagesList.isEmpty() || !msg.newDocumentsList.isEmpty()) {
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

        // process images list
        for (MessageObject::AttachmentDetails &image : msg.newImagesList) {
            // get full file path
            if (!image.path.isEmpty()) {
                QString fullFilePath = image.path;

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
            msg.imagesList.append(image);
        }
        msg.newImagesList.clear();

        // process documents list
        for (MessageObject::AttachmentDetails &document : msg.newDocumentsList) {
            // get full file path
            if (!document.path.isEmpty()) {
                QString fullFilePath = document.path;

                if (wasDraft) { // move assets belonging to draft to new assets folder
                    // assure filename is unique
                    while (QFileInfo::exists(dirPath + "/assets/" + document.name)) {
                        document.name = QUuid::createUuid().toString() + document.name;
                    }
                    if (!QFile::rename(fullFilePath, dirPath + "/assets/" + document.name)) {
                        // display error
                        QMessageBox msgBox;
                        msgBox.setText(QObject::tr("Error on moving assets!"));
                        msgBox.exec();
                        return false;
                    }
                } else { // copy file to assets folder
                    // assure filename is unique
                    while (QFileInfo::exists(dirPath + "/assets/" + document.name)) {
                        document.name = QUuid::createUuid().toString() + document.name;
                    }
                    if (!QFile::copy(fullFilePath, dirPath + "/assets/" + document.name)) {
                        // display error
                        QMessageBox msgBox;
                        msgBox.setText(QObject::tr("Error on copying assets!"));
                        msgBox.exec();
                        return false;
                    }
                }
            }
            msg.documentsList.append(document);
        }
        msg.newDocumentsList.clear();
    }


    // write contents to file
    QJsonObject content;
    msg.buildJson(content, isDraft);

    qInfo() << "Write JSON to" << msg.path;
    QFile file(msg.path);
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
        messageObject.path = info.absoluteFilePath();
        messageObject.setJson(QJsonDocument::fromJson(file.readAll()).object());
        _messageList << messageObject;
    }
}

void MessageModel::onDirectoryChanged(const QString &path)
{
    emit layoutAboutToBeChanged();
    qInfo() << "directory changed" << path;

    // check if a file was added
    for (const QFileInfo &info : QDir(path).entryInfoList(_filters, QDir::Files)) {
        bool found = false;
        for (const MessageObject &message : _messageList) {
            if (message.path == info.absoluteFilePath()) {
                found = true;
            }
        }

        // file was not in original list
        if (found == false) {
            qInfo() << "This file was added: " << info.absoluteFilePath();

            // parse message
            MessageObject messageObject;
            QFile file(info.absoluteFilePath());
            file.open(QIODevice::ReadOnly);
            messageObject.path = info.absoluteFilePath();
            messageObject.setJson(QJsonDocument::fromJson(file.readAll()).object());
            showNotification(tr("New Message"), messageObject.title, messageObject.priorityIcon(), messageObject.priority);
        }
    }

    _watcher.removePaths(_watcher.directories());
    _messageList.clear();
    addEntities();
    emit layoutChanged();
}

void MessageModel::showNotification(QString title, QString message, QIcon msgIcon, int priority) {
    bool silent = false;
    int prio = 2; // -2 to 2

    Snore::Icon notiIcon = Snore::Icon(msgIcon);

    QString msgs = tr("message");
    Snore::Alert alert;
    _notificationCounter++;

    if(_snoreNotification != nullptr){ // replace last notification
        QString oldMessage = _snoreNotification->text();
        if(_notificationCounter > 1){
            msgs = tr("messages");
            // remove message counter from old message
            oldMessage = oldMessage.split("<br/>").first().split("\n").first();
        }

        // test if new message has a higher priority
        if(_snoreNotification->alert().key().toInt() >= priority){ // keep notification text and icon
            title = _snoreNotification->title();
            message = QString(tr("%1 <br/> ... and %2 other %3")).arg(oldMessage).arg(_notificationCounter-1).arg(msgs);
            notiIcon = _snoreNotification->icon();
            priority = _snoreNotification->alert().key().toInt();
        }
        else { // replace notification text and icon
            message = QString(tr("%1 <br/> ... and %2 other %3")).arg(message).arg(_notificationCounter-1).arg(msgs);
        }

        // close old notification
        _snoreCore->requestCloseNotification(*_snoreNotification, Snore::Notification::Replaced);

    }

    switch(priority){
        case MessageObject::Priority::CriticalPriority:
            alert = *_snoreCriticalAlert;
            break;
        case MessageObject::Priority::UrgentPriority:
            alert = *_snoreUrgentAlert;
            break;
        case MessageObject::Priority::GoodPriority:
            alert = *_snoreGoodAlert;
            break;
        case MessageObject::Priority::InfoPriority:
            alert = *_snoreInfoAlert;
            break;
        default:
            alert = *_snoreCriticalAlert;
            break;
    }

    // create new notification
    _snoreNotification = new Snore::Notification(*_snoreApplication, alert, title, message, notiIcon, Snore::Notification::defaultTimeout(), static_cast<Snore::Notification::Prioritys>(prio));

    // register close event
    connect(_snoreCore, &Snore::SnoreCore::notificationClosed, [&](Snore::Notification noti) {
        if (silent) {
            QString reason;
            QDebug(&reason) << noti.closeReason();
        }
        notificationClosed(noti.closeReason());
    });

    _snoreCore->broadcastNotification(*_snoreNotification);
}

void MessageModel::notificationClosed(int returnCode){
    // notification was dismissed or closed by user
    if(returnCode == Snore::Notification::Dismissed || returnCode == Snore::Notification::Closed){
        _snoreNotification = nullptr;
        _notificationCounter = 0;
    }
}

} // end namespace OCC
