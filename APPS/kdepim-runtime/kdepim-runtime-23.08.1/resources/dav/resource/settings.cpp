/*
    SPDX-FileCopyrightText: 2009 Grégory Oestreicher <greg@kamago.net>
      Based on an original work for the IMAP resource which is :
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "settings.h"

#include "davresource_debug.h"
#include "settingsadaptor.h"
#include "utils.h"

#include <KAuthorized>
#include <KLocalizedString>

#include <KDAV/ProtocolInfo>

#include <KPasswordLineEdit>
#include <QByteArray>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDataStream>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QUrl>
#include <QVBoxLayout>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <qt5keychain/keychain.h>
#else
#include <qt6keychain/keychain.h>
#endif
using namespace QKeychain;

class SettingsHelper
{
public:
    SettingsHelper() = default;

    ~SettingsHelper()
    {
        delete q;
    }

    Settings *q = nullptr;
};

Q_GLOBAL_STATIC(SettingsHelper, s_globalSettings)

Settings::UrlConfiguration::UrlConfiguration() = default;

Settings::UrlConfiguration::UrlConfiguration(const QString &serialized)
{
    const QStringList splitString = serialized.split(QLatin1Char('|'));

    if (splitString.size() == 3) {
        mUrl = splitString.at(2);
        mProtocol = KDAV::ProtocolInfo::protocolByName(splitString.at(1));
        mUser = splitString.at(0);
    }
}

QString Settings::UrlConfiguration::serialize()
{
    QString serialized = mUser;
    serialized.append(QLatin1Char('|')).append(KDAV::ProtocolInfo::protocolName(KDAV::Protocol(mProtocol)));
    serialized.append(QLatin1Char('|')).append(mUrl);
    return serialized;
}

Settings *Settings::self()
{
    if (!s_globalSettings->q) {
        new Settings;
        s_globalSettings->q->load();
    }

    return s_globalSettings->q;
}

Settings::Settings()
    : SettingsBase()
    , mWinId(0)
{
    Q_ASSERT(!s_globalSettings->q);
    s_globalSettings->q = this;

    new SettingsAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"),
                                                 this,
                                                 QDBusConnection::ExportAdaptors | QDBusConnection::ExportScriptableContents);

    if (settingsVersion() == 1) {
        updateToV2();
    } else if (settingsVersion() == 2) {
        updateToV3();
    }
}

Settings::~Settings()
{
    QMapIterator<QString, UrlConfiguration *> it(mUrls);
    while (it.hasNext()) {
        it.next();
        delete it.value();
    }
}

void Settings::setWinId(WId winId)
{
    mWinId = winId;
}

void Settings::cleanup()
{
    const QString entry = mResourceIdentifier + QLatin1Char(',') + QStringLiteral("$default$");
    auto deleteJob = new DeletePasswordJob(QStringLiteral("Passwords"));
    deleteJob->setKey(entry);
    deleteJob->start();
    QFile cacheFile(mCollectionsUrlsMappingCache);
    cacheFile.remove();
}

void Settings::setResourceIdentifier(const QString &identifier)
{
    mResourceIdentifier = identifier;
}

void Settings::setDefaultPassword(const QString &password)
{
    savePassword(mResourceIdentifier, QStringLiteral("$default$"), password);
}

QString Settings::defaultPassword()
{
    return loadPassword(mResourceIdentifier, QStringLiteral("$default$"));
}

KDAV::DavUrl::List Settings::configuredDavUrls()
{
    if (mUrls.isEmpty()) {
        buildUrlsList();
    }
    KDAV::DavUrl::List davUrls;
    davUrls.reserve(mUrls.count());
    QMap<QString, UrlConfiguration *>::const_iterator it = mUrls.cbegin();
    const QMap<QString, UrlConfiguration *>::const_iterator itEnd = mUrls.cend();
    for (; it != itEnd; ++it) {
        const QStringList split = it.key().split(QLatin1Char(','));
        davUrls << configuredDavUrl(KDAV::ProtocolInfo::protocolByName(split.at(1)), split.at(0));
    }

    return davUrls;
}

KDAV::DavUrl Settings::configuredDavUrl(KDAV::Protocol proto, const QString &searchUrl, const QString &finalUrl)
{
    if (mUrls.isEmpty()) {
        buildUrlsList();
    }

    QUrl fullUrl;

    if (!finalUrl.isEmpty()) {
        fullUrl = QUrl::fromUserInput(finalUrl);
        if (finalUrl.startsWith(QLatin1Char('/'))) {
            QUrl searchQUrl(searchUrl);
            fullUrl.setHost(searchQUrl.host());
            fullUrl.setScheme(searchQUrl.scheme());
            fullUrl.setPort(searchQUrl.port());
        }
    } else {
        fullUrl = QUrl::fromUserInput(searchUrl);
    }

    const QString user = username(proto, searchUrl);
    fullUrl.setUserName(user);
    fullUrl.setPassword(password(proto, searchUrl));

    return KDAV::DavUrl(fullUrl, proto);
}

KDAV::DavUrl Settings::davUrlFromCollectionUrl(const QString &collectionUrl, const QString &finalUrl)
{
    if (mCollectionsUrlsMapping.isEmpty()) {
        loadMappings();
    }

    KDAV::DavUrl davUrl;
    const QString targetUrl = finalUrl.isEmpty() ? collectionUrl : finalUrl;

    if (mCollectionsUrlsMapping.contains(collectionUrl)) {
        const QStringList split = mCollectionsUrlsMapping.value(collectionUrl).split(QLatin1Char(','));
        if (split.size() == 2) {
            davUrl = configuredDavUrl(KDAV::ProtocolInfo::protocolByName(split.at(1)), split.at(0), targetUrl);
        }
    }

    return davUrl;
}

void Settings::addCollectionUrlMapping(KDAV::Protocol proto, const QString &collectionUrl, const QString &configuredUrl)
{
    if (mCollectionsUrlsMapping.isEmpty()) {
        loadMappings();
    }

    const QString value = configuredUrl + QLatin1Char(',') + KDAV::ProtocolInfo::protocolName(proto);
    mCollectionsUrlsMapping.insert(collectionUrl, value);

    // Update the cache now
    // QMap<QString, QString> tmp( mCollectionsUrlsMapping );
    QFileInfo cacheFileInfo = QFileInfo(mCollectionsUrlsMappingCache);
    if (!cacheFileInfo.dir().exists()) {
        QDir::root().mkpath(cacheFileInfo.dir().absolutePath());
    }

    QFile cacheFile(mCollectionsUrlsMappingCache);
    if (cacheFile.open(QIODevice::WriteOnly)) {
        QDataStream cache(&cacheFile);
        cache.setVersion(QDataStream::Qt_4_7);
        cache << mCollectionsUrlsMapping;
        cacheFile.close();
    }
}

QStringList Settings::mappedCollections(KDAV::Protocol proto, const QString &configuredUrl)
{
    if (mCollectionsUrlsMapping.isEmpty()) {
        loadMappings();
    }

    const QString value = configuredUrl + QLatin1Char(',') + KDAV::ProtocolInfo::protocolName(proto);
    return mCollectionsUrlsMapping.keys(value);
}

void Settings::reloadConfig()
{
    buildUrlsList();
    updateRemoteUrls();
    loadMappings();
}

void Settings::newUrlConfiguration(Settings::UrlConfiguration *urlConfig)
{
    const QString key = urlConfig->mUrl + QLatin1Char(',') + KDAV::ProtocolInfo::protocolName(KDAV::Protocol(urlConfig->mProtocol));

    if (mUrls.contains(key)) {
        removeUrlConfiguration(KDAV::Protocol(urlConfig->mProtocol), urlConfig->mUrl);
    }

    mUrls[key] = urlConfig;
    if (urlConfig->mUser != QLatin1String("$default$")) {
        savePassword(key, urlConfig->mUser, urlConfig->mPassword);
    }
    updateRemoteUrls();
}

void Settings::removeUrlConfiguration(KDAV::Protocol proto, const QString &url)
{
    const QString key = url + QLatin1Char(',') + KDAV::ProtocolInfo::protocolName(proto);

    if (!mUrls.contains(key)) {
        return;
    }

    delete mUrls[key];
    mUrls.remove(key);
    updateRemoteUrls();
}

Settings::UrlConfiguration *Settings::urlConfiguration(KDAV::Protocol proto, const QString &url)
{
    const QString key = url + QLatin1Char(',') + KDAV::ProtocolInfo::protocolName(proto);

    UrlConfiguration *ret = nullptr;
    if (mUrls.contains(key)) {
        ret = mUrls[key];
    }

    return ret;
}

// KDAV::Protocol Settings::protocol( const QString &url ) const
// {
//   if ( mUrls.contains( url ) )
//     return KDAV::Protocol( mUrls[ url ]->mProtocol );
//   else
//     returnKDAV::CalDav;
// }

QString Settings::username(KDAV::Protocol proto, const QString &url) const
{
    const QString key = url + QLatin1Char(',') + KDAV::ProtocolInfo::protocolName(proto);

    if (mUrls.contains(key)) {
        if (mUrls[key]->mUser == QLatin1String("$default$")) {
            return defaultUsername();
        } else {
            return mUrls[key]->mUser;
        }
    } else {
        return {};
    }
}

QString Settings::password(KDAV::Protocol proto, const QString &url)
{
    const QString key = url + QLatin1Char(',') + KDAV::ProtocolInfo::protocolName(proto);

    if (mUrls.contains(key)) {
        if (mUrls[key]->mUser == QLatin1String("$default$")) {
            return defaultPassword();
        } else {
            return mUrls[key]->mPassword;
        }
    } else {
        return {};
    }
}

QDateTime Settings::getSyncRangeStart() const
{
    QDateTime start = QDateTime::currentDateTimeUtc();
    start.setTime(QTime());
    const int delta = -syncRangeStartNumber().toUInt();

    if (syncRangeStartType() == QLatin1Char('D')) {
        start = start.addDays(delta);
    } else if (syncRangeStartType() == QLatin1Char('M')) {
        start = start.addMonths(delta);
    } else if (syncRangeStartType() == QLatin1Char('Y')) {
        start = start.addYears(delta);
    } else {
        start = QDateTime();
    }

    return start;
}

void Settings::buildUrlsList()
{
    const auto remoteUrlsLst = remoteUrls();
    for (const QString &serializedUrl : remoteUrlsLst) {
        auto urlConfig = new UrlConfiguration(serializedUrl);
        const QString key = urlConfig->mUrl + QLatin1Char(',') + KDAV::ProtocolInfo::protocolName(KDAV::Protocol(urlConfig->mProtocol));
        const QString pass = loadPassword(key, urlConfig->mUser);
        if (!pass.isNull()) {
            urlConfig->mPassword = pass;
            mUrls[key] = urlConfig;
        } else {
            delete urlConfig;
        }
    }
}

void Settings::loadMappings()
{
    const QString collectionsMappingCacheBase = QStringLiteral("akonadi-davgroupware/%1_c2u.dat").arg(QCoreApplication::applicationName());
    mCollectionsUrlsMappingCache = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + collectionsMappingCacheBase;
    QFile collectionsMappingsCache(mCollectionsUrlsMappingCache);

    if (collectionsMappingsCache.exists()) {
        if (collectionsMappingsCache.open(QIODevice::ReadOnly)) {
            QDataStream cache(&collectionsMappingsCache);
            cache >> mCollectionsUrlsMapping;
            collectionsMappingsCache.close();
        }
    } else if (!collectionsUrlsMappings().isEmpty()) {
        QByteArray rawMappings = QByteArray::fromBase64(collectionsUrlsMappings().toLatin1());
        QDataStream stream(&rawMappings, QIODevice::ReadOnly);
        stream >> mCollectionsUrlsMapping;
        setCollectionsUrlsMappings(QString());
    }
}

void Settings::updateRemoteUrls()
{
    QStringList newUrls;
    newUrls.reserve(mUrls.count());

    QMapIterator<QString, UrlConfiguration *> it(mUrls);
    while (it.hasNext()) {
        it.next();
        newUrls << it.value()->serialize();
    }

    setRemoteUrls(newUrls);
}

void Settings::savePassword(const QString &key, const QString &user, const QString &password)
{
    const QString entry = key + QLatin1Char(',') + user;
    mPasswordsCache[entry] = password;

    auto writeJob = new WritePasswordJob(QStringLiteral("Passwords"), this);
    connect(writeJob, &QKeychain::Job::finished, this, [](QKeychain::Job *baseJob) {
        if (baseJob->error()) {
            qCWarning(DAVRESOURCE_LOG) << "Error writing password using QKeychain:" << baseJob->errorString();
        }
    });
    writeJob->setKey(entry);
    writeJob->setTextData(password);
    writeJob->start();
}

QString Settings::loadPassword(const QString &key, const QString &user)
{
    QString entry;
    QString pass;

    if (user == QLatin1String("$default$")) {
        entry = mResourceIdentifier + QLatin1Char(',') + user;
    } else {
        entry = key + QLatin1Char(',') + user;
    }

    if (mPasswordsCache.contains(entry)) {
        return mPasswordsCache[entry];
    }

    QKeychain::ReadPasswordJob job(QStringLiteral("Passwords"));
    job.setAutoDelete(false);
    job.setKey(entry);
    QEventLoop loop; // Ideally we should have an async API
    QKeychain::ReadPasswordJob::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error() == QKeychain::Error::EntryNotFound) {
        pass = promptForPassword(user);
        if (!pass.isEmpty()) {
            if (user == QLatin1String("$default$")) {
                savePassword(mResourceIdentifier, user, pass);
            } else {
                savePassword(key, user, pass);
            }
        }
    } else if (job.error() != QKeychain::Error::NoError) {
        // Other type of errors
        pass = promptForPassword(user);
    } else {
        pass = QString::fromLatin1(job.binaryData());
    }

    if (!pass.isNull()) {
        mPasswordsCache[entry] = pass;
    }

    return pass;
}

QString Settings::promptForPassword(const QString &user)
{
    QPointer<QDialog> dlg = new QDialog();
    QString password;

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg);
    auto mainLayout = new QVBoxLayout(dlg);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, dlg.data(), &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dlg.data(), &QDialog::reject);

    auto mainWidget = new QWidget(dlg);
    mainLayout->addWidget(mainWidget);
    mainLayout->addWidget(buttonBox);
    auto vLayout = new QVBoxLayout();
    mainWidget->setLayout(vLayout);
    auto label = new QLabel(i18n("A password is required for user %1", (user == QLatin1String("$default$") ? defaultUsername() : user)), mainWidget);
    vLayout->addWidget(label);
    auto hLayout = new QHBoxLayout();
    label = new QLabel(i18n("Password: "), mainWidget);
    hLayout->addWidget(label);
    auto lineEdit = new KPasswordLineEdit();
    lineEdit->setRevealPasswordAvailable(KAuthorized::authorize(QStringLiteral("lineedit_reveal_password")));
    hLayout->addWidget(lineEdit);
    vLayout->addLayout(hLayout);
    lineEdit->setFocus();

    const int result = dlg->exec();

    if (result == QDialog::Accepted && !dlg.isNull()) {
        password = lineEdit->password();
    }

    delete dlg;
    return password;
}

void Settings::updateToV2()
{
    // Take the first URL that was configured to get the username that
    // has the most chances being the default

    QStringList urls = remoteUrls();
    if (urls.isEmpty()) {
        return;
    }

    const QString urlConfigStr = urls.at(0);
    UrlConfiguration urlConfig(urlConfigStr);
    const QRegularExpression regexp(QStringLiteral("^") + urlConfig.mUser);

    QMutableStringListIterator it(urls);
    while (it.hasNext()) {
        it.next();
        it.value().replace(regexp, QStringLiteral("$default$"));
    }

    setDefaultUsername(urlConfig.mUser);
    QString key = urlConfig.mUrl + QLatin1Char(',') + KDAV::ProtocolInfo::protocolName(KDAV::Protocol(urlConfig.mProtocol));
    QString pass = loadPassword(key, urlConfig.mUser);
    if (!pass.isNull()) {
        setDefaultPassword(pass);
    }
    setRemoteUrls(urls);
    setSettingsVersion(2);
    save();
}

void Settings::updateToV3()
{
    QStringList updatedUrls;

    const auto remoteUrlsLst = remoteUrls();
    for (const QString &url : remoteUrlsLst) {
        QStringList splitUrl = url.split(QLatin1Char('|'));

        if (splitUrl.size() == 3) {
            const KDAV::Protocol protocol = Utils::protocolByTranslatedName(splitUrl.at(1));
            splitUrl[1] = KDAV::ProtocolInfo::protocolName(protocol);
            updatedUrls << splitUrl.join(QLatin1Char('|'));
        }
    }

    setRemoteUrls(updatedUrls);
    setSettingsVersion(3);
    save();
}
