#pragma once

#ifdef PSP

#include <cstddef>

void pspEnsureSaveDirectory();
void pspRunSaveDialog(int mode);
void pspAutosave();

#endif
