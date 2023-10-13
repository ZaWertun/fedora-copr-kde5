/*
    SPDX-FileCopyrightText: 2009 Grégory Oestreicher <greg@kamago.net>
      Based on an original work for the IMAP resource which is :
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "settingsbase.h"

#include <KDAV/DavUrl>

#include <QMap>

class Settings : public SettingsBase
{
    Q_OBJECT

public:
    class UrlConfiguration
    {
    public:
        UrlConfiguration();
        explicit UrlConfiguration(const QString &serialized);

        /**
         * Serializes the object.
         * The string is the concatenation of the fields separated by a "|" :
         * user|protocol|url
         * The "protocol" component is the symbolic name of the protocol,
         * as returned by Utils::protocolName().
         */
        QString serialize();
        QString mUrl;
        QString mUser;
        QString mPassword;
        int mProtocol;
    };

    Settings();
    ~Settings() override;
    static Settings *self();
    void setWinId(WId wid);
    void cleanup();

    void setResourceIdentifier(const QString &identifier);
    void setDefaultPassword(const QString &password);
    QString defaultPassword();

    KDAV::DavUrl::List configuredDavUrls();

    /**
     * Creates and returns the DavUrl that corresponds to the configuration for searchUrl.
     * If finalUrl is supplied, then it will be used in the returned object instead of the searchUrl.
     */
    KDAV::DavUrl configuredDavUrl(KDAV::Protocol protocol, const QString &searchUrl, const QString &finalUrl = QString());

    /**
     * Creates and return the DavUrl from the configured URL that has a mapping with @p collectionUrl.
     * If @p finalUrl is supplied it will be used in the returned object, else @p collectionUrl will
     * be used.
     * If no configured URL can be found the returned DavUrl will have an empty url().
     */
    KDAV::DavUrl davUrlFromCollectionUrl(const QString &collectionUrl, const QString &finalUrl = QString());

    /**
     * Add a new mapping between the collection URL, as seen on the backend, and the
     * URL configured by the user. A mapping here means that the collectionUrl has
     * been discovered by a KDAV::DavCollectionsFetchJob on the configuredUrl.
     */
    void addCollectionUrlMapping(KDAV::Protocol protocol, const QString &collectionUrl, const QString &configuredUrl);

    /**
     * Returns the collections URLs mapped behind @p configuredUrl and @p protocol.
     */
    QStringList mappedCollections(KDAV::Protocol protocol, const QString &configuredUrl);

    /**
     * Reloads the resource configuration taking into account any new modification
     *
     * Whenever the resource configuration is modified it needs to be reload in order
     * to make the resource use the new config. This slot will call the needed methods
     * to be sure that any new setting is taken into account.
     */
    void reloadConfig();

    void newUrlConfiguration(UrlConfiguration *urlConfig);
    void removeUrlConfiguration(KDAV::Protocol protocol, const QString &url);
    UrlConfiguration *urlConfiguration(KDAV::Protocol protocol, const QString &url);

    // KDAV::Protocol protocol( const QString &url ) const;
    QString username(KDAV::Protocol protocol, const QString &url) const;
    QString password(KDAV::Protocol protocol, const QString &url);
    QDateTime getSyncRangeStart() const;

private:
    void buildUrlsList();
    void loadMappings();
    void updateRemoteUrls();
    void savePassword(const QString &key, const QString &user, const QString &password);
    QString loadPassword(const QString &key, const QString &user);
    QString promptForPassword(const QString &user);

    void updateToV2();
    void updateToV3();

    WId mWinId;
    QString mResourceIdentifier;
    QMap<QString, UrlConfiguration *> mUrls;
    QMap<QString, QString> mPasswordsCache;
    QString mCollectionsUrlsMappingCache;
    QMap<QString, QString> mCollectionsUrlsMapping;
    QList<UrlConfiguration *> mToDeleteUrlConfigs;
};
