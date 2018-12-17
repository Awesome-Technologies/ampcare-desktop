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

#ifndef MESSAGEOBJECT_H
#define MESSAGEOBJECT_H

#include <QDateTime>
#include <QIcon>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QUuid>

namespace OCC {

/**
 * @brief The MessageObject class that holds all data of a message
 * @ingroup gui
 */
class MessageObject
{
public:
    static const int TEXT_PREVIEW_LENGTH = 40;
    static const int ICON_IMAGE_HEIGHT = 32;

    enum Priority {
        InfoPriority,
        GoodPriority,
        UrgentPriority,
        CriticalPriority,
    };

    enum Status {
        DraftStatus,
        SentStatus,
        ReadStatus,
        ResentStatus,
        RereadStatus,
        ResolvedStatus,
        ArchivedStatus,
    };

    enum Gender {
        MALE,
        FEMALE,
    };

    struct AttachmentDetails
    {
        QString name;
        QString path;
        QString attachedBy;
    };

    MessageObject();

    // file data
    QString path;

    // message metadata
    QUuid messageId;
    Priority priority;
    Status status;
    QString statusText;
    QString title;
    QString initials;
    QString note;
    QString recipient;
    QString recipientName;

    QDateTime authoredOn;
    QString sender;
    QString senderName;

    // patient data
    QString patientName;
    QDate birthday;
    Gender gender;

    // vital data
    int bpSys;
    int bpDia;
    int pulse;
    double temp;
    double sugar;
    double weight;
    QString misc;
    QString pain;
    QString response;
    QDateTime dtPulse;
    QDateTime dtBp;
    QDateTime dtSugar;
    QDateTime dtWeight;
    QDateTime dtTemp;
    QDateTime dtDefac;

    QList<AttachmentDetails> imagesList;
    QList<AttachmentDetails> documentsList;
    QList<QStringList> medicationList;
    QList<QStringList> archivedFor;

    /** returns HTML of a summary of the message */
    QString longTitle() const;

    /** returns HTML title of message, trimmed to stay into the list column */
    QString shortTitle() const;

    /** returns an icon corresponding to priority */
    QIcon priorityIcon() const;

    /** returns the time how long the 'new message' notification should be shown for this message in milliseconds */
    int notificationTimeout() const;

    /** returns an icon to display the current status of the message */
    QIcon statusIcon() const;

    /** returns HTML to display the recipient of the message */
    QString getRecipient() const;

    /** returns HTML of content of the message */
    QString details() const;

    /** parses json of the message file */
    void setJson(const QJsonObject &json);

    /** creates a HL7/FHIR conform json */
    void buildJson(QJsonObject &json, bool isDraft) const;

    /** isArchivedFor for @p user */
    QString isArchivedFor(const QString &user) const;
};

} // end namespace

Q_DECLARE_METATYPE(OCC::MessageObject)

#endif // MESSAGEOBJECT_H
