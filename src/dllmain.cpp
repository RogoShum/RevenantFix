#include "core.hpp"

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
  return revenantfix::internal::HandleDllProcessEvent(module, reason);
}
