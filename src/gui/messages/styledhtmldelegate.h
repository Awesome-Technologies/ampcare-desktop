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

#ifndef STYLEDHTMLDELEGATE_H
#define STYLEDHTMLDELEGATE_H

#include <QStyledItemDelegate>

namespace OCC {

/**
 * @brief The StyledHtmlDelegate draws HTML using QTextDocument
 * @ingroup gui
 */
class StyledHtmlDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    StyledHtmlDelegate(QWidget *parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    virtual ~StyledHtmlDelegate();

    /**
     * displays the html from the message model
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    /**
     * estimates the ideal size for the entries
     */
    QSize sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
};

} // end namespace

#endif // STYLEDHTMLDELEGATE_H
