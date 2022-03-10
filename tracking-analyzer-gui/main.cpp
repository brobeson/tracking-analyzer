#include "application.h"
#include "gui/main_window.h"

auto main(int argc, char* argv[]) -> int
{
  analyzer::gui::application application {argc, argv};
  analyzer::gui::main_window window;
  window.show();
  return QCoreApplication::exec();
}
