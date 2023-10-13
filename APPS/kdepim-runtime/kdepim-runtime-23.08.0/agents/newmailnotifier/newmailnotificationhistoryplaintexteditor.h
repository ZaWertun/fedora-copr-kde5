/*
    SPDX-FileCopyrightText: 2023 Laurent Montel <montel@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KPIMTextEdit/PlainTextEditor>

class NewMailNotificationHistoryPlainTextEditor : public KPIMTextEdit::PlainTextEditor
{
    Q_OBJECT
public:
    explicit NewMailNotificationHistoryPlainTextEditor(QWidget *parent = nullptr);
    ~NewMailNotificationHistoryPlainTextEditor() override;

Q_SIGNALS:
    void clearHistory();

protected:
    void addExtraMenuEntry(QMenu *menu, QPoint pos) override;
};
