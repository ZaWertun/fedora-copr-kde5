/*
 * SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "highestmodseqattribute.h"

#include <QByteArray>

HighestModSeqAttribute::HighestModSeqAttribute(qint64 highestModSequence)
    : Akonadi::Attribute()
    , m_highestModSeq(highestModSequence)
{
}

void HighestModSeqAttribute::setHighestModSeq(qint64 highestModSequence)
{
    m_highestModSeq = highestModSequence;
}

qint64 HighestModSeqAttribute::highestModSequence() const
{
    return m_highestModSeq;
}

Akonadi::Attribute *HighestModSeqAttribute::clone() const
{
    return new HighestModSeqAttribute(m_highestModSeq);
}

QByteArray HighestModSeqAttribute::type() const
{
    static const QByteArray sType("highestmodseq");
    return sType;
}

void HighestModSeqAttribute::deserialize(const QByteArray &data)
{
    m_highestModSeq = data.toLongLong();
}

QByteArray HighestModSeqAttribute::serialized() const
{
    return QByteArray::number(m_highestModSeq);
}
