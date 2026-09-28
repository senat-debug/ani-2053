#include "NKWindow/NKWindow.h"
#include "NKWindow/NKMain.h"

using namespace nkentseu;

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Fenetre nue";

	NkWindow window(cfg);
	if (!window.IsOpen()) {
		return -1;
	}

	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>())
				window.Close();
		}
	}
	return 0;
}