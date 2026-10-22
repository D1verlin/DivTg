/*
This file is part of DivTG, a custom mod for Telegram Desktop.
*/
#pragma once

#ifdef Q_OS_WIN

#include <windows.h>
#include <dwmapi.h>
#include <QWidget>
#include "divtg/divtg_settings.h"
#include "rpl/lifetime.h"

namespace DivTG {

class AcrylicManager final {
public:
	static AcrylicManager &Instance();

	void attachToWindow(HWND hwnd, QWidget *widget);
	void updateBackdrop();
	void refreshPalette();

	[[nodiscard]] bool isSupported() const;
	[[nodiscard]] bool isWin11() const;
	[[nodiscard]] bool isWin10() const;
	[[nodiscard]] int osBuildNumber() const;

private:
	AcrylicManager();
	~AcrylicManager() = default;

	void applyBackdropWin11(HWND hwnd, BackdropType type);
	void applyBackdropWin10(HWND hwnd, bool enable, uint32_t tintColor);
	void disableBackdrop(HWND hwnd);

	HWND _hwnd = nullptr;
	QWidget *_widget = nullptr;
	int _buildNumber = 0;
	rpl::lifetime _lifetime;
};

inline AcrylicManager &Acrylic() {
	return AcrylicManager::Instance();
}

} // namespace DivTG

#endif // Q_OS_WIN
