/*
This file is part of DivTG, a custom mod for Telegram Desktop.
*/
#include "divtg/acrylic_manager.h"

#ifdef Q_OS_WIN

#include <QtCore/QOperatingSystemVersion>
#include <QtGui/QPalette>
#include <QtGui/QColor>
#include "platform/win/windows_dlls.h"
#include "ui/ui_utility.h"

namespace DivTG {
namespace {

// Undocumented Windows 10 Accent API
enum ACCENT_STATE {
	ACCENT_DISABLED = 0,
	ACCENT_ENABLE_GRADIENT = 1,
	ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
	ACCENT_ENABLE_BLURBEHIND = 3,
	ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
	ACCENT_INVALID_STATE = 5
};

struct ACCENT_POLICY {
	ACCENT_STATE AccentState;
	DWORD AccentFlags;
	DWORD GradientColor;
	DWORD AnimationId;
};

// Windows 11 DWM Backdrop constants
constexpr DWORD kDWMWA_MICA_EFFECT = 1029;
constexpr DWORD kDWMWA_SYSTEMBACKDROP_TYPE = 38;

constexpr DWORD kDWMSBT_AUTO = 0;
constexpr DWORD kDWMSBT_NONE = 1;
constexpr DWORD kDWMSBT_MAINWINDOW = 2;       // Mica
constexpr DWORD kDWMSBT_TRANSIENTWINDOW = 3;  // Acrylic
constexpr DWORD kDWMSBT_TABBEDWINDOW = 4;     // Mica Alt

} // namespace

AcrylicManager &AcrylicManager::Instance() {
	static AcrylicManager instance;
	return instance;
}

AcrylicManager::AcrylicManager() {
	const auto version = QOperatingSystemVersion::current();
	_buildNumber = version.microVersion();
}

int AcrylicManager::osBuildNumber() const {
	return _buildNumber;
}

bool AcrylicManager::isWin11() const {
	return _buildNumber >= 22000;
}

bool AcrylicManager::isWin10() const {
	return _buildNumber >= 17134 && _buildNumber < 22000;
}

bool AcrylicManager::isSupported() const {
	return _buildNumber >= 17134;
}

void AcrylicManager::attachToWindow(HWND hwnd, QWidget *widget) {
	_hwnd = hwnd;
	_widget = widget;
	_lifetime.destroy();

	if (!_hwnd || !_widget) {
		return;
	}

	// Listen to setting changes reactively
	ModSettings().acrylicEnabledChanges(
	) | rpl::on_next([=](bool) {
		updateBackdrop();
	}, _lifetime);

	ModSettings().backdropTypeChanges(
	) | rpl::on_next([=](BackdropType) {
		updateBackdrop();
	}, _lifetime);

	ModSettings().windowOpacityChanges(
	) | rpl::on_next([=](float) {
		updateBackdrop();
	}, _lifetime);

	updateBackdrop();
}

void AcrylicManager::updateBackdrop() {
	if (!_hwnd || !_widget) {
		return;
	}

	const auto enabled = ModSettings().acrylicEnabled();
	const auto type = ModSettings().backdropType();

	if (!enabled || type == BackdropType::None || !isSupported()) {
		disableBackdrop(_hwnd);
		_widget->setAttribute(Qt::WA_TranslucentBackground, false);
		_widget->update();
		return;
	}

	_widget->setAttribute(Qt::WA_TranslucentBackground, true);

	if (isWin11()) {
		applyBackdropWin11(_hwnd, type);
	} else if (isWin10()) {
		// Windows 10 Acrylic BlurBehind
		const auto alpha = uint32_t(std::clamp(ModSettings().windowOpacity() * 255.0f, 0.0f, 255.0f));
		// Semi-transparent dark/light tint (ABGR format)
		const uint32_t tintColor = (alpha << 24) | 0x00202020;
		applyBackdropWin10(_hwnd, true, tintColor);
	}

	_widget->update();
}

void AcrylicManager::applyBackdropWin11(HWND hwnd, BackdropType type) {
	if (!hwnd) {
		return;
	}

	// Windows 11 Build 22621+ (22H2 and newer): System Backdrop Type
	if (_buildNumber >= 22621) {
		DWORD backdrop = kDWMSBT_TRANSIENTWINDOW; // Default to Acrylic
		switch (type) {
		case BackdropType::Mica:
			backdrop = kDWMSBT_MAINWINDOW;
			break;
		case BackdropType::MicaAlt:
			backdrop = kDWMSBT_TABBEDWINDOW;
			break;
		case BackdropType::Acrylic:
		default:
			backdrop = kDWMSBT_TRANSIENTWINDOW;
			break;
		}

		DwmSetWindowAttribute(
			hwnd,
			kDWMWA_SYSTEMBACKDROP_TYPE,
			&backdrop,
			sizeof(backdrop));
	} else if (_buildNumber >= 22000) {
		// Windows 11 Build 22000 (21H2): Mica Effect
		BOOL mica = (type == BackdropType::Mica || type == BackdropType::MicaAlt) ? TRUE : FALSE;
		DwmSetWindowAttribute(
			hwnd,
			kDWMWA_MICA_EFFECT,
			&mica,
			sizeof(mica));

		if (type == BackdropType::Acrylic) {
			const auto alpha = uint32_t(std::clamp(ModSettings().windowOpacity() * 255.0f, 0.0f, 255.0f));
			const uint32_t tintColor = (alpha << 24) | 0x00202020;
			applyBackdropWin10(hwnd, true, tintColor);
		}
	}
}

void AcrylicManager::applyBackdropWin10(HWND hwnd, bool enable, uint32_t tintColor) {
	if (!hwnd || !Platform::Dlls::SetWindowCompositionAttribute) {
		return;
	}

	ACCENT_POLICY policy = {};
	if (enable) {
		policy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
		policy.AccentFlags = 2; // draw all borders
		policy.GradientColor = tintColor;
	} else {
		policy.AccentState = ACCENT_DISABLED;
	}

	Platform::Dlls::WINDOWCOMPOSITIONATTRIBDATA data = {
		static_cast<Platform::Dlls::WINDOWCOMPOSITIONATTRIB>(19), // WCA_ACCENT_POLICY
		&policy,
		sizeof(policy)
	};

	Platform::Dlls::SetWindowCompositionAttribute(hwnd, &data);
}

void AcrylicManager::disableBackdrop(HWND hwnd) {
	if (!hwnd) {
		return;
	}

	if (_buildNumber >= 22621) {
		DWORD backdrop = kDWMSBT_NONE;
		DwmSetWindowAttribute(
			hwnd,
			kDWMWA_SYSTEMBACKDROP_TYPE,
			&backdrop,
			sizeof(backdrop));
	} else if (_buildNumber >= 22000) {
		BOOL mica = FALSE;
		DwmSetWindowAttribute(
			hwnd,
			kDWMWA_MICA_EFFECT,
			&mica,
			sizeof(mica));
	}

	if (Platform::Dlls::SetWindowCompositionAttribute) {
		applyBackdropWin10(hwnd, false, 0);
	}
}

} // namespace DivTG

#endif // Q_OS_WIN
