/*
    SPDX-FileCopyrightText: 2023 Laurent Montel <montel@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "newmailnotificationhistoryplaintextedit.h"
#include "newmailnotificationhistoryplaintexteditor.h"

NewMailNotificationHistoryPlainTextEdit::NewMailNotificationHistoryPlainTextEdit(NewMailNotificationHistoryPlainTextEditor *editor, QWidget *parent)
    : KPIMTextEdit::PlainTextEditorWidget(editor, parent)
{
    connect(editor, &NewMailNotificationHistoryPlainTextEditor::clearHistory, this, &NewMailNotificationHistoryPlainTextEdit::clear);
}

NewMailNotificationHistoryPlainTextEdit::~NewMailNotificationHistoryPlainTextEdit() = default;
