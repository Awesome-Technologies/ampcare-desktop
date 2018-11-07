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

#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractTableModel>
#include <QFileSystemWatcher>
#include <sharee.h>

#include "messageobject.h"

namespace OCC {
class Sharee;

/**
 * @brief The MessageModel class abstracts the filesystem and provides access to the message files
 * @ingroup gui
 */
class MessageModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /** Columns for DisplayRole */
    enum Columns {
        PriorityColumn,
        TitleColumn,
        DateColumn,
        StatusColumn,
        ColumnCount,
    };

    /** See also QFileSystemModel::Roles */
    enum Roles {
        SortRole = Qt::UserRole,
        DetailRole,
        StatusRole,
        MessageObjectRole,
        MessageResolvedRole,
        MessageArchivedRole,
        ArchivedForRole
    };

    MessageModel(const QString &rootPath, const Sharee &currentUser, QObject *parent = 0);
    virtual ~MessageModel();

    QString rootPath() const { return _rootPath; }
    const Sharee &currentUser() const { return _currentUser; }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /** provides access to the content of the message files */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /** allows to change the content of the message files */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    /** provides header texts for the columns in the listView */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private slots:
    void onDirectoryChanged(const QString &path);

private:
    QString _rootPath;
    Sharee _currentUser;
    QFileSystemWatcher _watcher;
    QStringList _filters;
    QList<MessageObject> _messageList;

    /** writes message to file */
    bool writeMessage(MessageObject &msg);

    void addEntities();
    void addMessages(const QString &path);
};

} // end namespace

#endif // MESSAGEMODEL_H
