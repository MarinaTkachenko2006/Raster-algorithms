#include "dialog_windows.h"

void FileDialogState::reset() noexcept { ready = false; ok = false; path.clear(); }

void SDLCALL onFileDialogResult(void* userdata, const char* const* filelist, int /*filter*/) {
    auto* st = static_cast<FileDialogState*>(userdata);

    // Диалог закрыт, результат готов к обработке главным циклом
    st->pending = false;
    st->ready = true;

    // Файл не выбран, т.е. пользователь нажал "отмена"
    if (!filelist || !filelist[0]) { st->ok = false; return; }

    // Файл выбран
    st->ok = true;
    st->path = filelist[0];
}
