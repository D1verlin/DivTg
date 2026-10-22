# DivTG — Кастомный мод Telegram Desktop

**DivTG** — это модифицированная версия официального клиента [Telegram Desktop](https://github.com/telegramdesktop/tdesktop) на базе релиза **v7.3.3**.

---

## ✨ Реализованные особенности

1. **Эффект Windows Acrylic & Mica**:
   - Нативная поддержка размытия и прозрачности фона окна для **Windows 11** (DWM System Backdrop: `Acrylic` и `Mica`) и **Windows 10** (`SetWindowCompositionAttribute` с `ACCENT_ENABLE_ACRYLICBLURBEHIND`).
   - Настраиваемый уровень непрозрачности (50% – 95%).
   - Сохранение 100% четкости и контрастности текста, пузырей сообщений и медиа-контента.
   - Переключатель эффекта в меню: *Настройки → Продвинутые настройки → DivTG: Windows Acrylic / Mica transparency*.

2. **Брендинг и изоляция сессий**:
   - Отображаемое имя: **DivTG**, имя бинарника: **`DivTG.exe`**.
   - **Изолированный каталог данных**: `%APPDATA%\DivTG\tdata`.
     *DivTG может работать одновременно с официальным клиентом Telegram Desktop без конфликтов блокировки данных и баз SQLite.*
   - **Портативный режим**: при размещении папки `tdata` рядом с `DivTG.exe` все данные хранятся локально.
   - Метаданные исполняемого файла Windows (`DivTG Desktop`, `DivTG Team`).

3. **Собственная иконка приложения**:
   - Комплект иконок во всех системных разрешениях (16x16, 24x24, 32x32, 48x48, 64x64, 128x128, 256x256, 512x512).
   - Стилизованный мастер-логотип с неоновым градиентом и геометрией бумажного самолета.
   - Скрипт автоматической перегенерации иконок: `scripts/generate_icons.py`.

4. **Система обновлений**:
   - Официальный автоапдейтер tdesktop отключен (клиент никогда не перезапишет мод стандартным Telegram).
   - Интеграция проверки новых версий мода через **GitHub Releases API**.

---

## 📂 Архитектура и структура кода

Все модификации мода изолированы в отдельном модуле, что сводит к минимуму вероятность конфликтов при выходе новых версий Telegram Desktop:

```
Telegram/
  ├── SourceFiles/
  │   ├── divtg/                      <-- Автономный модуль мода DivTG
  │   │   ├── divtg_config.h          # Версии, метаданные и URL репозитория
  │   │   ├── divtg_settings.h/.cpp   # Хранилище настроек мода (JSON в tdata)
  │   │   ├── acrylic_manager.h/.cpp  # DWM & Windows 10/11 Backdrop Engine
  │   │   └── github_updater.h/.cpp   # Проверка обновлений через GitHub API
  │   ├── core/
  │   │   └── version.h               # Константы AppName, AppFile, AppId
  │   ├── platform/win/
  │   │   └── main_window_win.cpp     # Хук инициализации AcrylicManager
  │   └── window/
  │       └── main_window.cpp         # Альфа-канал палитры фона окна
  ├── Resources/
  │   ├── art/                        # Иконки PNG и многослойный icon256.ico
  │   └── winrc/Telegram.rc           # Ресурсы версии Windows (DivTG Desktop)
  └── CMakeLists.txt                  # Сборка исполняемого файла DivTG.exe
scripts/
  ├── generate_icons.py               # Генератор иконок всех размеров
  ├── sync_upstream.ps1               # Автоматизация подтягивания обновлений
  └── build_win.ps1                   # Скрипт локальной сборки под Windows
.github/workflows/
  └── build-divtg.yml                 # GitHub Actions CI для сборки .exe в облаке
```

---

## 🔄 Как подвязывать новые обновления Telegram

Когда команда Telegram выпускает новую версию (например, `v7.4.0`):

### Автоматический способ (рекомендуется):
Запустите скрипт синхронизации из корня репозитория:
```powershell
.\scripts\sync_upstream.ps1 -UpstreamTag v7.4.0
```

Скрипт автоматически:
1. Загрузит свежие теги с `https://github.com/telegramdesktop/tdesktop.git`.
2. Создаст резервную ветку текущего состояния.
3. Выполнит `git rebase` наших коммитов DivTG поверх нового тега.

### Ручной способ:
```powershell
# 1. Получить свежие теги официального tdesktop
git fetch upstream --tags

# 2. Накатить наши коммиты на новый релизный тег
git checkout main
git rebase v7.4.0
```
Так как почти вся логика DivTG вынесена в `Telegram/SourceFiles/divtg/`, перебазирование проходит чисто и бесконфликтно.

---

## 🔨 Сборка проекта

### Вариант 1: Автоматическая сборка в облаке (GitHub Actions)
В репозитории уже настроен рабочий пайплайн `.github/workflows/build-divtg.yml`.
1. Опубликуйте свой форк на GitHub.
2. При любом пуше в ветку `main` или создании тега (например, `git tag v1.0.0 && git push --tags`) GitHub Actions автоматически соберет проект.
3. Скачайте готовый архив `DivTG-Windows-x64.zip` во вкладке **Actions** или в **Releases**.

### Вариант 2: Локальная сборка на Windows (VS 2022)
1. Откройте **x64 Native Tools Command Prompt for VS 2022**.
2. Перейдите в папку проекта:
   ```cmd
   cd C:\Users\Lenovo\Desktop\Projects\DivTG
   ```
3. Запустите скрипт сборки:
   ```powershell
   powershell -ExecutionPolicy Bypass -File .\scripts\build_win.ps1 -Configuration Release
   ```
4. Собранный файл будет находиться в `out\Release\DivTG.exe`.

---

## 🎨 Кастомизация иконки

Если вы захотите заменить стартовый дизайн иконки на собственный:
1. Отредактируйте функцию рисования в `scripts/generate_icons.py` или поместите свой квадратный файл `master_logo.png` (1024x1024) в корень проекта.
2. Запустите генератор:
   ```powershell
   python scripts/generate_icons.py
   ```
3. Все необходимые системные иконки (`.ico`, HiDPI `.png`) обновятся автоматически.
