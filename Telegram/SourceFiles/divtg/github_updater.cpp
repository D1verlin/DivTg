/*
This file is part of DivTG, a custom mod for Telegram Desktop.
*/
#include "divtg/github_updater.h"
#include "divtg/divtg_config.h"
#include "divtg/divtg_settings.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkRequest>

namespace DivTG {

GitHubUpdater &GitHubUpdater::Instance() {
	static GitHubUpdater instance;
	return instance;
}

GitHubUpdater::GitHubUpdater() {
}

void GitHubUpdater::checkForUpdates(bool silent) {
	QNetworkRequest request(QUrl(ModReleasesApiUrl));
	request.setHeader(QNetworkRequest::UserAgentHeader, QString("%1-Updater/%2").arg(ModName, ModVersion));
	request.setRawHeader("Accept", "application/vnd.github.v3+json");

	auto *reply = _networkManager.get(request);
	connect(reply, &QNetworkReply::finished, [=] {
		handleNetworkReply(reply, silent);
	});
}

void GitHubUpdater::handleNetworkReply(QNetworkReply *reply, bool silent) {
	reply->deleteLater();

	if (reply->error() != QNetworkReply::NoError) {
		if (!silent) {
			_checkFailed.fire(reply->errorString());
		}
		return;
	}

	const auto data = reply->readAll();
	const auto doc = QJsonDocument::fromJson(data);
	if (!doc.isObject()) {
		if (!silent) {
			_checkFailed.fire("Invalid JSON response from GitHub API");
		}
		return;
	}

	const auto root = doc.object();
	ReleaseInfo info;
	info.tagName = root["tag_name"].toString();
	info.versionName = root["name"].toString();
	info.releaseNotes = root["body"].toString();
	info.htmlUrl = root["html_url"].toString();

	// Look for executable asset in assets array
	const auto assets = root["assets"].toArray();
	for (const auto &item : assets) {
		const auto asset = item.toObject();
		const auto name = asset["name"].toString().toLower();
		if (name.endsWith(".exe") || name.endsWith(".zip")) {
			info.downloadUrl = asset["browser_download_url"].toString();
			break;
		}
	}

	// Clean tag string (strip leading 'v')
	QString cleanRemote = info.tagName;
	if (cleanRemote.startsWith('v') || cleanRemote.startsWith('V')) {
		cleanRemote = cleanRemote.mid(1);
	}

	info.isNewer = compareVersions(cleanRemote, ModVersion);

	if (info.isNewer) {
		_newVersionFound.fire(std::move(info));
	} else {
		_isLatest.fire({});
	}
}

bool GitHubUpdater::compareVersions(const QString &remote, const QString &current) const {
	const auto remoteParts = remote.split('.');
	const auto currentParts = current.split('.');

	const auto count = std::max(remoteParts.size(), currentParts.size());
	for (int i = 0; i < count; ++i) {
		const auto r = (i < remoteParts.size()) ? remoteParts[i].toInt() : 0;
		const auto c = (i < currentParts.size()) ? currentParts[i].toInt() : 0;
		if (r > c) return true;
		if (r < c) return false;
	}
	return false;
}

rpl::producer<ReleaseInfo> GitHubUpdater::newVersionFound() const {
	return _newVersionFound.events();
}

rpl::producer<> GitHubUpdater::isLatest() const {
	return _isLatest.events();
}

rpl::producer<QString> GitHubUpdater::checkFailed() const {
	return _checkFailed.events();
}

} // namespace DivTG
