/*
This file is part of DivTG, a custom mod for Telegram Desktop.
*/
#include "divtg/divtg_settings.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include "base/platform/base_platform_info.h"

extern QString cWorkingDir();

namespace DivTG {

Settings &Settings::Instance() {
	static Settings instance;
	return instance;
}

Settings::Settings() {
	load();
}

QString Settings::settingsFilePath() const {
	const auto dir = cWorkingDir();
	if (dir.isEmpty()) {
		return QString();
	}
	return dir + "tdata/divtg_settings.json";
}

void Settings::load() {
	const auto path = settingsFilePath();
	if (path.isEmpty()) {
		return;
	}

	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		return;
	}

	const auto doc = QJsonDocument::fromJson(file.readAll());
	if (!doc.isObject()) {
		return;
	}

	const auto obj = doc.object();
	if (obj.contains("acrylic_enabled")) {
		_acrylicEnabled = obj["acrylic_enabled"].toBool(true);
	}
	if (obj.contains("backdrop_type")) {
		_backdropType = static_cast<BackdropType>(obj["backdrop_type"].toInt(1));
	}
	if (obj.contains("window_opacity")) {
		const auto op = float(obj["window_opacity"].toDouble(0.85));
		_windowOpacity = std::clamp(op, 0.50f, 1.0f);
	}
	if (obj.contains("check_updates")) {
		_checkUpdates = obj["check_updates"].toBool(true);
	}
}

void Settings::save() {
	const auto path = settingsFilePath();
	if (path.isEmpty()) {
		return;
	}

	QJsonObject obj;
	obj["acrylic_enabled"] = _acrylicEnabled.current();
	obj["backdrop_type"] = static_cast<int>(_backdropType.current());
	obj["window_opacity"] = double(_windowOpacity.current());
	obj["check_updates"] = _checkUpdates.current();

	QDir().mkpath(QFileInfo(path).absolutePath());
	QFile file(path);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
	}
}

bool Settings::acrylicEnabled() const {
	return _acrylicEnabled.current();
}

void Settings::setAcrylicEnabled(bool enabled) {
	if (_acrylicEnabled.current() != enabled) {
		_acrylicEnabled = enabled;
		save();
	}
}

BackdropType Settings::backdropType() const {
	return _backdropType.current();
}

void Settings::setBackdropType(BackdropType type) {
	if (_backdropType.current() != type) {
		_backdropType = type;
		save();
	}
}

float Settings::windowOpacity() const {
	return _windowOpacity.current();
}

void Settings::setWindowOpacity(float opacity) {
	const auto clamped = std::clamp(opacity, 0.50f, 1.0f);
	if (_windowOpacity.current() != clamped) {
		_windowOpacity = clamped;
		save();
	}
}

bool Settings::checkUpdates() const {
	return _checkUpdates.current();
}

void Settings::setCheckUpdates(bool check) {
	if (_checkUpdates.current() != check) {
		_checkUpdates = check;
		save();
	}
}

rpl::producer<bool> Settings::acrylicEnabledChanges() const {
	return _acrylicEnabled.changes();
}

rpl::producer<BackdropType> Settings::backdropTypeChanges() const {
	return _backdropType.changes();
}

rpl::producer<float> Settings::windowOpacityChanges() const {
	return _windowOpacity.changes();
}

} // namespace DivTG
