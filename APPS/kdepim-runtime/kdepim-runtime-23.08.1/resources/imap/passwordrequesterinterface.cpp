/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "passwordrequesterinterface.h"

PasswordRequesterInterface::PasswordRequesterInterface(QObject *parent)
    : QObject(parent)
{
}

void PasswordRequesterInterface::cancelPasswordRequests()
{
}
