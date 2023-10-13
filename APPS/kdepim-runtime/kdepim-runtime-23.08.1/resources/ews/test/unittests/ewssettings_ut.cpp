/*
    SPDX-FileCopyrightText: 2017-2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KWallet>
#include <QTest>
#include <functional>

#include "ewssettings.h"
#include "ewssettings_ut_mock.h"

Q_LOGGING_CATEGORY(EWSRES_LOG, "org.kde.pim.ews", QtInfoMsg)

static const QString accessTokenMapKey = QStringLiteral("access-token");
static const QString refreshTokenMapKey = QStringLiteral("refresh-token");

class UtEwsSettings : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void readNoPassword();
    void readNullWallet();
    void readTimeout();
    void readTimeoutInterrupted();
    void readValidPassword();
    void writeNullPassword();
    void writeNullWallet();
    void writeTimeout();
    void writeValidPassword();
    void readValidMap();
    void writeValidMap();
};

namespace KWallet
{
class MyWallet : public Wallet
{
    Q_OBJECT
public:
    MyWallet();
    ~MyWallet() override;

    void doOpen(bool success);

    bool hasFolder(const QString &folder) override;
    bool setFolder(const QString &folder) override;
    bool createFolder(const QString &folder) override;
    int readPassword(const QString &key, QString &value) override;
    int writePassword(const QString &key, const QString &value) override;
    int readMap(const QString &key, QMap<QString, QString> &value) override;
    int writeMap(const QString &key, const QMap<QString, QString> &value) override;

    std::function<bool(const QString &)> hasFolderCallback;
    std::function<bool(const QString &)> setFolderCallback;
    std::function<bool(const QString &)> createFolderCallback;
    std::function<int(const QString &, QString &)> readPasswordCallback;
    std::function<int(const QString &, const QString &)> writePasswordCallback;
    std::function<int(const QString &, QMap<QString, QString> &)> readMapCallback;
    std::function<int(const QString &, const QMap<QString, QString> &)> writeMapCallback;
};

static std::function<void()> errorCallback;
static std::function<Wallet *(MyWallet *wallet)> openWalletCallback;

void reportError()
{
    if (errorCallback) {
        errorCallback();
    }
}

Wallet *openWallet(MyWallet *wallet)
{
    if (openWalletCallback) {
        return openWalletCallback(wallet);
    } else {
        qDebug() << "Wallet open callback not registered!";
        errorCallback();
        return wallet;
    }
}

static const QString networkWallet = QStringLiteral("test_network_wallet");
inline const QString Wallet::NetworkWallet()
{
    return networkWallet;
}

inline Wallet *Wallet::openWallet(const QString &name, WId, OpenType ot)
{
    qDebug() << "intercepted openWallet";
    if (name != networkWallet) {
        qDebug() << "Incorrect wallet name";
        reportError();
    }

    if (ot != Asynchronous) {
        qDebug() << "Unsupported open type";
        reportError();
    }

    auto wallet = new MyWallet();
    return KWallet::openWallet(wallet);
}

MyWallet::MyWallet()
    : Wallet(0, networkWallet)
{
}

MyWallet::~MyWallet() = default;

void MyWallet::doOpen(bool success)
{
    Q_EMIT walletOpened(success);
}

bool MyWallet::hasFolder(const QString &folder)
{
    if (hasFolderCallback) {
        return hasFolderCallback(folder);
    } else {
        qWarning() << "hasFolder() callback not set!";
        reportError();
        return false;
    }
}

bool MyWallet::setFolder(const QString &folder)
{
    if (setFolderCallback) {
        return setFolderCallback(folder);
    } else {
        qWarning() << "setFolder() callback not set!";
        reportError();
        return false;
    }
}

bool MyWallet::createFolder(const QString &folder)
{
    if (createFolderCallback) {
        return createFolderCallback(folder);
    } else {
        qWarning() << "createFolder() callback not set!";
        reportError();
        return false;
    }
}

int MyWallet::readPassword(const QString &key, QString &value)
{
    if (readPasswordCallback) {
        return readPasswordCallback(key, value);
    } else {
        qWarning() << "readPasswordCallback() callback not set!";
        reportError();
        return 0;
    }
}

int MyWallet::writePassword(const QString &key, const QString &value)
{
    if (writePasswordCallback) {
        return writePasswordCallback(key, value);
    } else {
        qWarning() << "writePasswordCallback() callback not set!";
        reportError();
        return 0;
    }
}

int MyWallet::readMap(const QString &key, QMap<QString, QString> &value)
{
    if (readMapCallback) {
        return readMapCallback(key, value);
    } else {
        qWarning() << "readMapCallback() callback not set!";
        reportError();
        return 0;
    }
}

int MyWallet::writeMap(const QString &key, const QMap<QString, QString> &value)
{
    if (writeMapCallback) {
        return writeMapCallback(key, value);
    } else {
        qWarning() << "writeMapCallback() callback not set!";
        reportError();
        return 0;
    }
}
}

void UtEwsSettings::readNoPassword()
{
    KWallet::MyWallet *wallet = nullptr;
    KWallet::openWalletCallback = [&wallet](KWallet::MyWallet *w) {
        wallet = w;
        return w;
    };

    QEventLoop loop;
    bool error = false;
    KWallet::errorCallback = [&]() {
        if (loop.isRunning()) {
            loop.exit(1);
        } else {
            error = true;
        }
    };

    bool hasFolderCalled = false;
    QString password;
    EwsSettings settings(0);
    connect(&settings, &EwsSettings::passwordRequestFinished, this, [&](const QString &p) {
        password = p;
        loop.exit(0);
    });
    QTimer::singleShot(100, this, [&]() {
        settings.requestPassword();
        if (!wallet) {
            qDebug() << "Wallet is null";
            loop.exit(1);
            return;
        }
        wallet->hasFolderCallback = [&hasFolderCalled](const QString &) {
            hasFolderCalled = true;
            return false;
        };
        wallet->createFolderCallback = [](const QString &) {
            return false;
        };
        wallet->doOpen(true);
    });
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        qDebug() << "Test timeout";
        loop.exit(1);
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(2000);

    QVERIFY(error != true);

    QVERIFY(loop.exec() == 0);

    QVERIFY(password.isNull());
    QVERIFY(hasFolderCalled);
}

void UtEwsSettings::readNullWallet()
{
    KWallet::openWalletCallback = [](KWallet::MyWallet *) {
        return nullptr;
    };

    QEventLoop loop;
    bool error = false;
    KWallet::errorCallback = [&]() {
        if (loop.isRunning()) {
            loop.exit(1);
        } else {
            error = true;
        }
    };

    QString password;
    EwsSettings settings(0);
    connect(&settings, &EwsSettings::passwordRequestFinished, this, [&](const QString &p) {
        password = p;
        loop.exit(0);
    });
    QTimer::singleShot(100, [&]() {
        settings.requestPassword();
    });
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        qDebug() << "Test timeout";
        loop.exit(1);
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(2000);

    QVERIFY(error != true);

    QVERIFY(loop.exec() == 0);

    QVERIFY(password.isNull());
}

void UtEwsSettings::readTimeout()
{
    KWallet::MyWallet *wallet = nullptr;
    KWallet::openWalletCallback = [&wallet](KWallet::MyWallet *w) {
        wallet = w;
        return w;
    };

    QEventLoop loop;
    bool error = false;
    KWallet::errorCallback = [&]() {
        if (loop.isRunning()) {
            loop.exit(1);
        } else {
            error = true;
        }
    };

    bool hasFolderCalled = false;
    QString password;
    EwsSettings settings(0);
    connect(&settings, &EwsSettings::passwordRequestFinished, this, [&](const QString &p) {
        password = p;
        loop.exit(0);
    });
    QTimer::singleShot(100, this, [&]() {
        settings.requestPassword();
        if (!wallet) {
            qDebug() << "Wallet is null";
            loop.exit(1);
            return;
        }
        wallet->hasFolderCallback = [&hasFolderCalled](const QString &) {
            hasFolderCalled = true;
            return false;
        };
        wallet->createFolderCallback = [](const QString &) {
            return false;
        };
    });
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        qDebug() << "Test timeout";
        loop.exit(1);
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(5000);

    QVERIFY(error != true);

    QVERIFY(loop.exec() == 0);

    QVERIFY(password.isNull());
    QVERIFY(!hasFolderCalled);
}

void UtEwsSettings::readTimeoutInterrupted()
{
    KWallet::MyWallet *wallet = nullptr;
    KWallet::openWalletCallback = [&wallet](KWallet::MyWallet *w) {
        wallet = w;
        return w;
    };

    QEventLoop loop;
    bool error = false;
    KWallet::errorCallback = [&]() {
        if (loop.isRunning()) {
            loop.exit(1);
        } else {
            error = true;
        }
    };

    bool hasFolderCalled = false;
    QString password;
    EwsSettings settings(0);
    bool testSecondSignal = false;
    connect(&settings, &EwsSettings::passwordRequestFinished, this, [&](const QString &p) {
        if (!testSecondSignal) {
            password = p;
            loop.exit(0);
        } else {
            loop.exit(1);
        }
    });
    QTimer::singleShot(100, this, [&]() {
        settings.requestPassword();
        if (!wallet) {
            qDebug() << "Wallet is null";
            loop.exit(1);
            return;
        }
        wallet->hasFolderCallback = [&hasFolderCalled](const QString &) {
            hasFolderCalled = true;
            return false;
        };
        wallet->createFolderCallback = [](const QString &) {
            return false;
        };
    });
    QTimer::singleShot(1000, this, [&]() {
        settings.setTestPassword(QStringLiteral("foo"));
    });
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        qDebug() << "Test timeout";
        loop.exit(2);
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(5000);

    QVERIFY(error != true);

    QVERIFY(loop.exec() == 0);

    QVERIFY(password == QLatin1String("foo"));
    QVERIFY(!hasFolderCalled);

    // Check for second passwordRequestFinished signal
    QVERIFY(loop.exec() == 2);
}

void UtEwsSettings::readValidPassword()
{
    KWallet::MyWallet *wallet = nullptr;
    KWallet::openWalletCallback = [&wallet](KWallet::MyWallet *w) {
        wallet = w;
        return w;
    };

    QEventLoop loop;
    bool error = false;
    KWallet::errorCallback = [&]() {
        if (loop.isRunning()) {
            loop.exit(1);
        } else {
            error = true;
        }
    };

    bool hasFolderCalled = false;
    bool setFolderCalled = false;
    QString password;
    EwsSettings settings(0);
    connect(&settings, &EwsSettings::passwordRequestFinished, this, [&](const QString &p) {
        password = p;
        loop.exit(0);
    });
    QTimer::singleShot(100, this, [&]() {
        settings.requestPassword();
        if (!wallet) {
            qDebug() << "Wallet is null";
            loop.exit(1);
            return;
        }
        wallet->hasFolderCallback = [&hasFolderCalled](const QString &) {
            hasFolderCalled = true;
            return true;
        };
        wallet->createFolderCallback = [](const QString &) {
            return false;
        };
        wallet->setFolderCallback = [&setFolderCalled](const QString &) {
            setFolderCalled = true;
            return true;
        };
        wallet->readPasswordCallback = [](const QString &, QString &password) {
            password = QStringLiteral("foo");
            return true;
        };
        wallet->doOpen(true);
    });
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        qDebug() << "Test timeout";
        loop.exit(1);
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(2000);

    QVERIFY(error != true);

    QVERIFY(loop.exec() == 0);

    QVERIFY(password == QLatin1String("foo"));
    QVERIFY(hasFolderCalled);
    QVERIFY(setFolderCalled);
}

void UtEwsSettings::writeNullPassword()
{
    KWallet::MyWallet *wallet = nullptr;
    KWallet::openWalletCallback = [&wallet](KWallet::MyWallet *w) {
        wallet = w;
        return w;
    };

    QEventLoop loop;
    bool error = false;
    KWallet::errorCallback = [&]() {
        if (loop.isRunning()) {
            loop.exit(1);
        } else {
            error = true;
        }
    };

    EwsSettings settings(0);
    QTimer::singleShot(100, this, [&]() {
        settings.setPassword(QString());
        if (wallet) {
            qDebug() << "Wallet is not null";
            loop.exit(1);
            return;
        }
        loop.exit(0);
    });
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        qDebug() << "Test timeout";
        loop.exit(1);
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(2000);

    QVERIFY(error != true);

    QVERIFY(loop.exec() == 0);
}

void UtEwsSettings::writeNullWallet()
{
    KWallet::MyWallet *wallet = nullptr;
    KWallet::openWalletCallback = [](KWallet::MyWallet *) {
        return nullptr;
    };

    QEventLoop loop;
    bool error = false;
    KWallet::errorCallback = [&]() {
        if (loop.isRunning()) {
            loop.exit(1);
        } else {
            error = true;
        }
    };

    EwsSettings settings(0);
    QTimer::singleShot(100, this, [&]() {
        settings.setPassword(QStringLiteral("foo"));
        if (wallet) {
            qDebug() << "Wallet is not null";
            loop.exit(1);
            return;
        }
        loop.exit(0);
    });
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        qDebug() << "Test timeout";
        loop.exit(1);
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(2000);

    QVERIFY(error != true);

    QVERIFY(loop.exec() == 0);
}

void UtEwsSettings::writeTimeout()
{
    KWallet::MyWallet *wallet = nullptr;
    KWallet::openWalletCallback = [&wallet](KWallet::MyWallet *w) {
        wallet = w;
        return w;
    };

    QEventLoop loop;
    bool error = false;
    KWallet::errorCallback = [&]() {
        if (loop.isRunning()) {
            loop.exit(1);
        } else {
            error = true;
        }
    };

    bool hasFolderCalled = false;
    bool createFolderCalled = false;
    bool setFolderCalled = false;
    QString password;
    EwsSettings settings(0);
    QTimer::singleShot(100, this, [&]() {
        settings.setPassword(QStringLiteral("foo"));
        if (!wallet) {
            qDebug() << "Wallet is null";
            loop.exit(1);
            return;
        }
        wallet->hasFolderCallback = [&hasFolderCalled](const QString &) {
            hasFolderCalled = true;
            return false;
        };
        wallet->createFolderCallback = [&createFolderCalled](const QString &) {
            createFolderCalled = true;
            return true;
        };
        wallet->setFolderCallback = [&setFolderCalled](const QString &) {
            setFolderCalled = true;
            return false;
        };
        wallet->writePasswordCallback = [&](const QString &, const QString &p) {
            password = p;
            loop.exit(0);
            return true;
        };
    });
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        qDebug() << "Test timeout";
        loop.exit(2);
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(5000);

    QVERIFY(error != true);

    QVERIFY(loop.exec() == 2);

    QVERIFY(password.isNull());
    QVERIFY(!hasFolderCalled);
    QVERIFY(!setFolderCalled);
    QVERIFY(!createFolderCalled);
}

void UtEwsSettings::writeValidPassword()
{
    KWallet::MyWallet *wallet = nullptr;
    KWallet::openWalletCallback = [&wallet](KWallet::MyWallet *w) {
        wallet = w;
        return w;
    };

    QEventLoop loop;
    bool error = false;
    KWallet::errorCallback = [&]() {
        if (loop.isRunning()) {
            loop.exit(1);
        } else {
            error = true;
        }
    };

    bool hasFolderCalled = false;
    bool createFolderCalled = false;
    bool setFolderCalled = false;
    QString password;
    EwsSettings settings(0);
    QTimer::singleShot(100, this, [&]() {
        settings.setPassword(QStringLiteral("foo"));
        if (!wallet) {
            qDebug() << "Wallet is null";
            loop.exit(1);
            return;
        }
        wallet->hasFolderCallback = [&hasFolderCalled](const QString &) {
            hasFolderCalled = true;
            return false;
        };
        wallet->createFolderCallback = [&createFolderCalled](const QString &) {
            createFolderCalled = true;
            return true;
        };
        wallet->setFolderCallback = [&setFolderCalled](const QString &) {
            setFolderCalled = true;
            return false;
        };
        wallet->writePasswordCallback = [&](const QString &, const QString &p) {
            password = p;
            loop.exit(0);
            return true;
        };
        wallet->doOpen(true);
    });
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        qDebug() << "Test timeout";
        loop.exit(1);
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(2000);

    QVERIFY(error != true);

    QVERIFY(loop.exec() == 0);

    QVERIFY(password == QLatin1String("foo"));
    QVERIFY(hasFolderCalled);
    QVERIFY(setFolderCalled);
    QVERIFY(createFolderCalled);
}

void UtEwsSettings::readValidMap()
{
    KWallet::MyWallet *wallet = nullptr;
    KWallet::openWalletCallback = [&wallet](KWallet::MyWallet *w) {
        wallet = w;
        return w;
    };

    QEventLoop loop;
    bool error = false;
    KWallet::errorCallback = [&]() {
        if (loop.isRunning()) {
            loop.exit(1);
        } else {
            error = true;
        }
    };

    bool hasFolderCalled = false;
    bool setFolderCalled = false;
    QMap<QString, QString> map;
    const QMap<QString, QString> expectedMap = {
        {accessTokenMapKey, QStringLiteral("afoo")},
        {refreshTokenMapKey, QStringLiteral("rfoo")},
    };
    EwsSettings settings(0);
    connect(&settings, &EwsSettings::mapRequestFinished, this, [&](const QMap<QString, QString> &m) {
        map = m;
        loop.exit(0);
    });
    QTimer::singleShot(100, this, [&]() {
        settings.requestMap();
        if (!wallet) {
            qDebug() << "Wallet is null";
            loop.exit(1);
            return;
        }
        wallet->hasFolderCallback = [&hasFolderCalled](const QString &) {
            hasFolderCalled = true;
            return true;
        };
        wallet->createFolderCallback = [](const QString &) {
            return false;
        };
        wallet->setFolderCallback = [&setFolderCalled](const QString &) {
            setFolderCalled = true;
            return true;
        };
        wallet->readMapCallback = [](const QString &, QMap<QString, QString> &map) {
            map[accessTokenMapKey] = QStringLiteral("afoo");
            map[refreshTokenMapKey] = QStringLiteral("rfoo");
            return true;
        };
        wallet->doOpen(true);
    });
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        qDebug() << "Test timeout";
        loop.exit(1);
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(2000);

    QVERIFY(error != true);

    QVERIFY(loop.exec() == 0);

    QVERIFY(map == expectedMap);
    QVERIFY(hasFolderCalled);
    QVERIFY(setFolderCalled);
}

void UtEwsSettings::writeValidMap()
{
    KWallet::MyWallet *wallet = nullptr;
    KWallet::openWalletCallback = [&wallet](KWallet::MyWallet *w) {
        wallet = w;
        return w;
    };

    QEventLoop loop;
    bool error = false;
    KWallet::errorCallback = [&]() {
        if (loop.isRunning()) {
            loop.exit(1);
        } else {
            error = true;
        }
    };

    bool hasFolderCalled = false;
    bool createFolderCalled = false;
    bool setFolderCalled = false;
    const QMap<QString, QString> expectedMap = {{accessTokenMapKey, QStringLiteral("afoo")}, {refreshTokenMapKey, QStringLiteral("rfoo")}};
    QMap<QString, QString> map;
    EwsSettings settings(0);
    QTimer::singleShot(100, this, [&]() {
        settings.setMap(expectedMap);
        if (!wallet) {
            qDebug() << "Wallet is null";
            loop.exit(1);
            return;
        }
        wallet->hasFolderCallback = [&hasFolderCalled](const QString &) {
            hasFolderCalled = true;
            return false;
        };
        wallet->createFolderCallback = [&createFolderCalled](const QString &) {
            createFolderCalled = true;
            return true;
        };
        wallet->setFolderCallback = [&setFolderCalled](const QString &) {
            setFolderCalled = true;
            return false;
        };
        wallet->writeMapCallback = [&](const QString &, const QMap<QString, QString> &m) {
            map = m;
            loop.exit(0);
            return true;
        };
        wallet->doOpen(true);
    });
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        qDebug() << "Test timeout";
        loop.exit(1);
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(2000);

    QVERIFY(error != true);

    QVERIFY(loop.exec() == 0);

    QVERIFY(map == expectedMap);
    QVERIFY(hasFolderCalled);
    QVERIFY(setFolderCalled);
    QVERIFY(createFolderCalled);
}

QTEST_MAIN(UtEwsSettings)

#include "ewssettings_ut.moc"
