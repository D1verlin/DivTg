/*
This file is part of DivTG, a custom mod for Telegram Desktop.
*/
#pragma once

#include <QString>
#include "rpl/variable.h"

namespace DivTG {

enum class BackdropType {
	None = 0,
	Acrylic = 1,
	Mica = 2,
	MicaAlt = 3
};

class Settings final {
public:
	static Settings &Instance();

	void load();
	void save();

	[[nodiscard]] bool acrylicEnabled() const;
	void setAcrylicEnabled(bool enabled);

	[[nodiscard]] BackdropType backdropType() const;
	void setBackdropType(BackdropType type);

	[[nodiscard]] float windowOpacity() const;
	void setWindowOpacity(float opacity);

	[[nodiscard]] bool checkUpdates() const;
	void setCheckUpdates(bool check);

	rpl::producer<bool> acrylicEnabledChanges() const;
	rpl::producer<BackdropType> backdropTypeChanges() const;
	rpl::producer<float> windowOpacityChanges() const;

private:
	Settings();
	~Settings() = default;

	QString settingsFilePath() const;

	rpl::variable<bool> _acrylicEnabled = true;
	rpl::variable<BackdropType> _backdropType = BackdropType::Acrylic;
	rpl::variable<float> _windowOpacity = 0.85f;
	rpl::variable<bool> _checkUpdates = true;
};

inline Settings &ModSettings() {
	return Settings::Instance();
}

} // namespace DivTG
