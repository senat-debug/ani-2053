// SeptDroits — exercice 2 du chapitre 3.
// Cree sept fenetres, une par droit desactive, et releve ce que le systeme
// en a fait. Le releve part dans sept_droits.txt, a cote de l'executable.

#include "NKWindow/NKWindow.h"
#include "NKWindow/NKMain.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;

static const char *kDroits[7] = {
	"frame", "resizable", "minimizable", "movable", "closable", "maximizable", "canFullscreen"};

static void Desactiver(NkWindowConfig &cfg, int i) {
	switch (i) {
		case 0: cfg.frame = false; break;
		case 1: cfg.resizable = false; break;
		case 2: cfg.minimizable = false; break;
		case 3: cfg.movable = false; break;
		case 4: cfg.closable = false; break;
		case 5: cfg.maximizable = false; break;
		case 6: cfg.canFullscreen = false; break;
	}
}

static const char *OuiNon(bool b) { return b ? "oui" : "non"; }

static const char *EtatMenu(HMENU menu, UINT item) {
	UINT s = GetMenuState(menu, item, MF_BYCOMMAND);
	if (s == static_cast<UINT>(-1)) return "absent";
	return (s & (MF_DISABLED | MF_GRAYED)) ? "grise" : "actif";
}

int nkmain(const NkEntryState &state) {
	NkWindow fenetres[7];
	char titres[7][64];

	for (int i = 0; i < 7; ++i) {
		NkWindowConfig cfg;
		std::snprintf(titres[i], sizeof(titres[i]), "%d - sans %s", i + 1, kDroits[i]);
		cfg.title = titres[i];
		cfg.width = 420;
		cfg.height = 240;
		cfg.centered = false;
		cfg.x = 40 + (i % 4) * 450;
		cfg.y = 60 + (i / 4) * 320;
		Desactiver(cfg, i);
		fenetres[i].Create(cfg);
	}

	FILE *f = std::fopen("sept_droits.txt", "w");
	std::fprintf(f, "droit | POPUP | CAPTION | THICKFRAME | MINIMIZEBOX | MAXIMIZEBOX | Fermer | Deplacer | bandeau\n");
	for (int i = 0; i < 7; ++i) {
		HWND h = fenetres[i].mData.mHwnd;
		LONG_PTR st = GetWindowLongPtrW(h, GWL_STYLE);
		HMENU menu = GetSystemMenu(h, FALSE);
		RECT rw{}, rc{};
		GetWindowRect(h, &rw);
		GetClientRect(h, &rc);
		const long bandeau = (rw.bottom - rw.top) - (rc.bottom - rc.top);
		std::fprintf(f, "%-14s | %-5s | %-7s | %-10s | %-11s | %-11s | %-6s | %-8s | %ld px\n",
			kDroits[i],
			OuiNon(st & WS_POPUP), OuiNon(st & WS_CAPTION), OuiNon(st & WS_THICKFRAME),
			OuiNon(st & WS_MINIMIZEBOX), OuiNon(st & WS_MAXIMIZEBOX),
			EtatMenu(menu, SC_CLOSE), EtatMenu(menu, SC_MOVE), bandeau);
	}

	fenetres[1].SetSize(700, 400);
	math::NkVec2u t = fenetres[1].GetSize();
	std::fprintf(f, "\nSetSize(700,400) sur la fenetre sans resizable : %u x %u\n", t.x, t.y);

	fenetres[5].Maximize();
	std::fprintf(f, "Maximize() sur la fenetre sans maximizable : IsMaximized=%s\n",
		OuiNon(fenetres[5].IsMaximized()));
	std::fclose(f);

	bool reste = true;
	while (reste) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (auto *c = ev->As<NkWindowCloseEvent>()) {
				for (int i = 0; i < 7; ++i) {
					if (fenetres[i].IsOpen() && fenetres[i].GetId() == c->GetWindowId())
						fenetres[i].Close();
				}
			}
		}
		reste = false;
		for (int i = 0; i < 7; ++i) {
			if (fenetres[i].IsOpen()) reste = true;
		}
	}
	return 0;
}