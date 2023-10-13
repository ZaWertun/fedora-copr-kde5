/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsitembase.h"
#include "ewstypes.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class EwsFolderPrivate;

class EwsFolder : public EwsItemBase
{
public:
    typedef QList<EwsFolder> List;

    EwsFolder();
    explicit EwsFolder(QXmlStreamReader &reader);
    EwsFolder(const EwsFolder &other);
    EwsFolder(EwsFolder &&other);
    ~EwsFolder() override;

    EwsFolder &operator=(const EwsFolder &other);
    EwsFolder &operator=(EwsFolder &&other);

    EwsFolderType type() const;
    void setType(EwsFolderType type);

    const QVector<EwsFolder> childFolders() const;
    void addChild(EwsFolder &child);
    EwsFolder *parentFolder() const;
    void setParentFolder(EwsFolder *parent);

    bool write(QXmlStreamWriter &writer) const;

protected:
    bool readBaseFolderElement(QXmlStreamReader &reader);
};

Q_DECLARE_METATYPE(EwsFolder)
