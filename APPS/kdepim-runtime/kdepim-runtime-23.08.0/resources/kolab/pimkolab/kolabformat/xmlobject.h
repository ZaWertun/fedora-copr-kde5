/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#pragma once

#include "kolab_export.h"

#include <kolabformat.h>

#include "kolabdefinitions.h"

namespace Kolab
{
class KOLAB_EXPORT XMLObject
{
public:
    XMLObject();

    std::string getSerializedUID() const;

    /// List of attachment names to be retrieved from the mime message (only when reading v2, for v3 attachments containing the cid: of the attachment-part are
    /// created )
    std::vector<std::string> getAttachments() const;

    Kolab::Event readEvent(const std::string &s, Kolab::Version version);
    std::string writeEvent(const Kolab::Event &, Kolab::Version version, const std::string &productId = std::string());

    Kolab::Todo readTodo(const std::string &s, Kolab::Version version);
    std::string writeTodo(const Kolab::Todo &, Kolab::Version version, const std::string &productId = std::string());

    Kolab::Journal readJournal(const std::string &s, Kolab::Version version);
    std::string writeJournal(const Kolab::Journal &, Kolab::Version version, const std::string &productId = std::string());

    Kolab::Freebusy readFreebusy(const std::string &s, Kolab::Version version);
    std::string writeFreebusy(const Kolab::Freebusy &, Kolab::Version version, const std::string &productId = std::string());

    std::string pictureAttachmentName() const;
    std::string logoAttachmentName() const;
    std::string soundAttachmentName() const;
    /**
     * Find the attachments and set them on the read Contact object.
     *
     * V2 Notes:
     * Picture, logo and sound must be retrieved from Mime Message attachments using they're corresponding attachment name.
     */
    Kolab::Contact readContact(const std::string &s, Kolab::Version version);

    /**
     * V2 Notes:
     * * Uses the following attachment names:
     * ** kolab-picture.png
     * ** kolab-logo.png
     * ** sound
     */
    std::string writeContact(const Kolab::Contact &, Kolab::Version version, const std::string &productId = std::string());

    Kolab::DistList readDistlist(const std::string &s, Kolab::Version version);
    std::string writeDistlist(const Kolab::DistList &, Kolab::Version version, const std::string &productId = std::string());

    /**
     * V2 notes:
     * * set the creation date from the mime date header.
     */
    Kolab::Note readNote(const std::string &s, Kolab::Version version);
    std::string writeNote(const Kolab::Note &, Kolab::Version version, const std::string &productId = std::string());

    Kolab::Configuration readConfiguration(const std::string &s, Kolab::Version version);
    std::string writeConfiguration(const Kolab::Configuration &, Kolab::Version version, const std::string &productId = std::string());

    Kolab::File readFile(const std::string &s, Kolab::Version version);
    std::string writeFile(const Kolab::File &, Kolab::Version version, const std::string &productId = std::string());

private:
    std::vector<std::string> mAttachments;
    std::string mLogoAttachmentName;
    std::string mSoundAttachmentName;
    std::string mPictureAttachmentName;
    std::string mWrittenUID;
};
}
