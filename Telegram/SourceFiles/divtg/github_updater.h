/*
This file is part of DivTG, a custom mod for Telegram Desktop.
*/
#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include "rpl/variable.h"

namespace DivTG {

struct ReleaseInfo {
	QString tagName;
	QString versionName;
	QString releaseNotes;
	QString htmlUrl;
	QString downloadUrl;
	bool isNewer = false;
};

class GitHubUpdater final : public QObject {
public:
	static GitHubUpdater &Instance();

	void checkForUpdates(bool silent = false);

	rpl::producer<ReleaseInfo> newVersionFound() const;
	rpl::producer<> isLatest() const;
	rpl::producer<QString> checkFailed() const;

private:
	GitHubUpdater();
	~GitHubUpdater() override = default;

	void handleNetworkReply(QNetworkReply *reply, bool silent);
	[[nodiscard]] bool compareVersions(const QString &remoteVersion, const QString &currentVersion) const;

	QNetworkAccessManager _networkManager;
	rpl::event_stream<ReleaseInfo> _newVersionFound;
	rpl::event_stream<> _isLatest;
	rpl::event_stream<QString> _checkFailed;
};

inline GitHubUpdater &Updater() {
	return GitHubUpdater::Instance();
}

} // namespace DivTG
