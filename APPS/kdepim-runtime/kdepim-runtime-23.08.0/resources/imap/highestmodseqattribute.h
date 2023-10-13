/*
 * SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#pragma once

#include <Akonadi/Attribute>

class HighestModSeqAttribute : public Akonadi::Attribute
{
public:
    explicit HighestModSeqAttribute(qint64 highestModSequence = -1);
    void setHighestModSeq(qint64 highestModSequence);
    Q_REQUIRED_RESULT qint64 highestModSequence() const;

    void deserialize(const QByteArray &data) override;
    Q_REQUIRED_RESULT QByteArray serialized() const override;
    Akonadi::Attribute *clone() const override;
    Q_REQUIRED_RESULT QByteArray type() const override;

private:
    qint64 m_highestModSeq;
};
