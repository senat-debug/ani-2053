// GlisserQuiSort — exercice 7 du chapitre 3.
// Un glisser qui commence dans la fenetre et continue dehors, une fois SANS
// capture, une fois AVEC. Le programme compte ce qu'il recoit dans les deux cas.
//
// Touche C : bascule la capture (affichee dans le titre).
// Releve dans glisser.txt, a cote de l'executable.

#include "NKWindow/NKWindow.h"
#include "NKWindow/NKMain.h"
#include "NKEvent/NkMouseEvent.h"
#include "NKEvent/NkKeyboardEvent.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;

struct Glisser {
	unsigned long dedans = 0;   // mouvements recus, souris dans la fenetre
	unsigned long dehors = 0;   // mouvements recus, souris hors de la fenetre
	int dernierX = 0, dernierY = 0;
	bool relacheRecu = false;
	bool relacheDehors = false;
	bool captureTenue = false;  // GetCapture() == notre fenetre, pendant le glisser
};

static void Ecrire(FILE *f, const char *titre, const Glisser &g) {
	std::fprintf(f, "%s\n", titre);
	std::fprintf(f, "  mouvements recus DANS la fenetre  : %lu\n", g.dedans);
	std::fprintf(f, "  mouvements recus HORS la fenetre  : %lu\n", g.dehors);
	std::fprintf(f, "  derniere position recue           : %d , %d\n", g.dernierX, g.dernierY);
	std::fprintf(f, "  GetCapture() == notre fenetre     : %s\n", g.captureTenue ? "oui" : "non");
	std::fprintf(f, "  relachement recu                  : %s%s\n",
		g.relacheRecu ? "oui" : "NON",
		g.relacheRecu ? (g.relacheDehors ? " (souris dehors)" : " (souris dedans)") : "");
	std::fprintf(f, "\n");
}

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Glisser sans capture (C pour basculer)";
	cfg.width = 700;
	cfg.height = 420;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	bool capture = false;
	bool enGlisser = false;
	Glisser sans, avec;

	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) {
				window.Close();
			} else if (auto *k = ev->As<NkKeyPressEvent>()) {
				if (k->GetKey() == NkKey::NK_C) {
					capture = !capture;
					window.SetTitle(capture ? "Glisser AVEC capture (C pour basculer)"
											: "Glisser SANS capture (C pour basculer)");
				}
			} else if (auto *b = ev->As<NkMouseButtonPressEvent>()) {
				if (b->GetButton() == NkMouseButton::NK_MB_LEFT) {
					enGlisser = true;
					if (capture) window.CaptureMouse(true);
					Glisser &g = capture ? avec : sans;
					g.captureTenue = (GetCapture() == window.mData.mHwnd);
				}
			} else if (auto *m = ev->As<NkMouseMoveEvent>()) {
				if (!enGlisser) continue;
				Glisser &g = capture ? avec : sans;
				const math::NkVec2u taille = window.GetSize();
				const int x = m->GetX(), y = m->GetY();
				const bool dehors = (x < 0 || y < 0 || x >= (int)taille.x || y >= (int)taille.y);
				if (dehors) ++g.dehors; else ++g.dedans;
				g.dernierX = x;
				g.dernierY = y;
			} else if (auto *r = ev->As<NkMouseButtonReleaseEvent>()) {
				if (r->GetButton() == NkMouseButton::NK_MB_LEFT && enGlisser) {
					Glisser &g = capture ? avec : sans;
					const math::NkVec2u taille = window.GetSize();
					const int x = r->GetX(), y = r->GetY();
					g.relacheRecu = true;
					g.relacheDehors = (x < 0 || y < 0 || x >= (int)taille.x || y >= (int)taille.y);
					enGlisser = false;
					if (capture) window.CaptureMouse(false);
				}
			}
		}
		Sleep(1);
	}

	FILE *f = std::fopen("glisser.txt", "w");
	Ecrire(f, "A) SANS capture", sans);
	Ecrire(f, "B) AVEC capture", avec);
	std::fclose(f);
	return 0;
}