#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <Windows.h>
#include <TlHelp32.h>

#include <string>
#include <unordered_map>
#include <vector>

using namespace geode::prelude;

namespace {
    // PID -> original priority class, so we can put everything back afterwards.
    std::unordered_map<DWORD, DWORD> g_originalPriorities;
    DWORD g_originalGDPriority = 0;
    bool g_applied = false;

    bool isEnabled() {
        return Mod::get()->getSettingValue<bool>("enabled");
    }

    std::vector<std::wstring> getTargetNames() {
        std::vector<std::wstring> names;
        auto raw = Mod::get()->getSettingValue<std::string>("processes");
        for (auto& part : utils::string::split(raw, ",")) {
            auto name = utils::string::trim(part);
            if (!name.empty()) names.push_back(utils::string::utf8ToWide(name));
        }
        return names;
    }

    void applyPriorities() {
        if (g_applied) return;

        auto targets = getTargetNames();
        auto self = GetCurrentProcessId();

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            log::warn("Could not list processes (error {})", GetLastError());
            return;
        }

        PROCESSENTRY32W entry{};
        entry.dwSize = sizeof(entry);
        int lowered = 0;

        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (entry.th32ProcessID == self) continue;

                bool match = false;
                for (auto const& target : targets) {
                    if (_wcsicmp(entry.szExeFile, target.c_str()) == 0) {
                        match = true;
                        break;
                    }
                }
                if (!match) continue;

                HANDLE proc = OpenProcess(
                    PROCESS_SET_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION,
                    FALSE, entry.th32ProcessID
                );
                if (!proc) continue;

                DWORD original = GetPriorityClass(proc);
                if (original != 0 && SetPriorityClass(proc, IDLE_PRIORITY_CLASS)) {
                    g_originalPriorities[entry.th32ProcessID] = original;
                    ++lowered;
                }
                CloseHandle(proc);
            } while (Process32NextW(snapshot, &entry));
        }
        CloseHandle(snapshot);

        if (Mod::get()->getSettingValue<bool>("boost-gd")) {
            HANDLE gd = GetCurrentProcess();
            g_originalGDPriority = GetPriorityClass(gd);
            if (!SetPriorityClass(gd, HIGH_PRIORITY_CLASS)) g_originalGDPriority = 0;
        }

        g_applied = true;
        log::info("Lowered priority of {} process(es)", lowered);
    }

    void restorePriorities() {
        if (!g_applied) return;

        for (auto const& [pid, priority] : g_originalPriorities) {
            HANDLE proc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
            if (!proc) continue; // the program was closed in the meantime
            SetPriorityClass(proc, priority);
            CloseHandle(proc);
        }
        g_originalPriorities.clear();

        if (g_originalGDPriority != 0) {
            SetPriorityClass(GetCurrentProcess(), g_originalGDPriority);
            g_originalGDPriority = 0;
        }

        g_applied = false;
        log::info("Restored original priorities");
    }
}

class $modify(OptimizerMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return true;

        auto button = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png"),
            this, menu_selector(OptimizerMenuLayer::onOptimizerButton)
        );
        button->setID("toggle-button"_spr);
        menu->addChild(button);
        menu->updateLayout();

        return true;
    }

    void onOptimizerButton(CCObject*) {
        bool enabled = isEnabled();
        auto text = enabled
            ? "GD Optimizer is currently <cg>enabled</c>.\nWould you like to <cr>disable</c> it?"
            : "GD Optimizer is currently <cr>disabled</c>.\nWould you like to <cg>enable</c> it?";

        createQuickPopup(
            "GD Optimizer", text, "Cancel", enabled ? "Disable" : "Enable",
            [enabled](FLAlertLayer*, bool confirmed) {
                if (!confirmed) return;
                // Saved inside the callback (the original saved before the user answered).
                Mod::get()->setSettingValue<bool>("enabled", !enabled);
                if (enabled) restorePriorities();
            }
        );
    }
};

class $modify(PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        if (isEnabled()) applyPriorities();
        return true;
    }

    void onQuit() {
        if (Mod::get()->getSettingValue<bool>("restore-on-exit")) restorePriorities();
        PlayLayer::onQuit();
    }
};

// Never leave other programs stuck at low priority after closing the game.
$on_game(Exiting) {
    restorePriorities();
}
