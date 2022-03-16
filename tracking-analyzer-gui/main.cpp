#include "application.h"
#include "gui/main_window.h"

auto main(int argc, char* argv[]) -> int
{
  analyzer::gui::application application {argc, argv};
  analyzer::gui::main_window window;
  window.show();
  // if (analyzer::gui::application::settings().contains(
  //       analyzer::gui::settings_keys::last_loaded_dataset))
  // {
  //   window.load_dataset(
  //     analyzer::gui::application::settings()
  //       .value(analyzer::gui::settings_keys::last_loaded_dataset)
  //       .toString());
  // }
  return QCoreApplication::exec();
}
