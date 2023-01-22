#include <nfd.h>

#include "logger/logger.h"
#include "window_manager/window_mgr.h"

#include "frontend.h"
#include "main_menu/main_menu.h"
#include "status_bar/status_bar.h"
#include "play_control/play_control.h"

#include "waveform_window/waveform_window.h"
#include "console/console.h"

#include "imgui_example/imgui_example.h"

namespace hpaslt {

void frontendInit() {
  NFD_Init();
  logger->coreLogger->debug("NativeFileDialog initialized.");
}

void frontendTerminate() {
  NFD_Quit();
  logger->coreLogger->debug("NativeFileDialog terminated.");
}

void registerAllImGuiObjs() {
  hpaslt::logger->uiLogger->debug("Frontend entry point.");

  // Main Menu.
  hpaslt::WindowManager::getSingleton().lock()->setMainMenuBar(
      std::make_shared<MainMenu>());

  hpaslt::WindowManager::getSingleton().lock()->setPlayControlBar(
      std::make_shared<PlayControl>());

  // Status bar.
  hpaslt::WindowManager::getSingleton().lock()->setMainStatusBar(
      std::make_shared<StatusBar>());

  /* -------------------------- Views ------------------------- */

  hpaslt::WindowManager::getSingleton().lock()->pushRenderObject(
      std::make_shared<WaveformWindow>());

  hpaslt::WindowManager::getSingleton().lock()->pushRenderObject(
      std::make_shared<Console>());

  /* -------------------------- Debug ------------------------- */

  // ImGui Example.
  hpaslt::WindowManager::getSingleton().lock()->pushRenderObject(
      std::make_shared<ImGuiExample>());
}

}  // namespace hpaslt