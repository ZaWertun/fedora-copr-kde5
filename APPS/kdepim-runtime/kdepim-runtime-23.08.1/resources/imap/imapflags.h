/*
 * SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
 * SPDX-FileCopyrightText: 2010 Leo Franchi <lfranchi@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

/**
 * Contains constants for IMAP flags from KIMAP.
 */
namespace ImapFlags
{
/**
 * The flag for a message being seen (i.e. opened by user).
 */
extern const char *Seen;

/**
 * The flag for a message being deleted by the user.
 */
extern const char *Deleted;

/**
 * The flag for a message being replied to by the user.
 */
extern const char *Answered;

/**
 * The flag for a message being marked as flagged.
 */
extern const char *Flagged;
}
