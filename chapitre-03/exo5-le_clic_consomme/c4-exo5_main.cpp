// PanneauQuiConsomme — exercice du chapitre 4.
// Un panneau occupe le coin haut-gauche de la fenetre (240 x 160). Un clic
// dans ce coin est CONSOMME par le premier gestionnaire : le second, celui du
// reste de la fenetre, n'est pas appele.
// Le tri se fait avec NkEventDispatcher : Dispatch() sort tout de suite si
// l'evenement est deja marque traite (NkEventDispatcher.h:124-126), et il le
// marque quand le gestionnaire rend true (ligne 132-134).
// Releve dans panneau.txt, a cote de l'executable.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkMouseEvent.h"
#include "NKEvent/NkEventDispatcher.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;

static const int kPanneauL = 240; // largeur du panneau
static const int kPanneauH = 160; // hauteur du panneau

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Panneau en haut a gauche : 240 x 160";
	cfg.width = 800;
	cfg.height = 460;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	FILE *f = std::fopen("panneau.txt", "w");
	std::fprintf(f, "Panneau : x < %d et y < %d\n\n", kPanneauL, kPanneauH);

	unsigned long clics = 0, prisParPanneau = 0, prisParFond = 0, secondAppele = 0;

	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) { window.Close(); continue; }
			if (!ev->Is<NkMouseButtonPressEvent>()) continue;

			++clics;
			NkEventDispatcher d(ev);
			bool secondEstEntre = false;

			// --- 1er gestionnaire : le panneau ---------------------------
			const bool consomme = d.Dispatch<NkMouseButtonPressEvent>(
				[&](NkMouseButtonPressEvent &e) -> bool {
					const bool dansLePanneau = (e.GetX() < kPanneauL && e.GetY() < kPanneauH);
					std::fprintf(f, "clic %-3lu (%4d,%4d) | PANNEAU   : %s\n",
						clics, e.GetX(), e.GetY(),
						dansLePanneau ? "c'est pour moi, je CONSOMME (true)"
									  : "pas chez moi, je laisse passer (false)");
					if (dansLePanneau) ++prisParPanneau;
					return dansLePanneau; // true = consomme
				});

			// --- 2e gestionnaire : le reste de la fenetre -----------------
			d.Dispatch<NkMouseButtonPressEvent>(
				[&](NkMouseButtonPressEvent &e) -> bool {
					secondEstEntre = true;
					++secondAppele;
					++prisParFond;
					std::fprintf(f, "%-22s | FOND      : appele, je traite le clic (%d,%d)\n",
						"", e.GetX(), e.GetY());
					return true;
				});

			if (consomme && !secondEstEntre)
				std::fprintf(f, "%-22s | FOND      : PAS APPELE (evenement deja marque traite)\n", "");
			std::fprintf(f, "\n");
			std::fflush(f);
		}
		Sleep(1);
	}

	std::fprintf(f, "clics recus            : %lu\n", clics);
	std::fprintf(f, "consommes par le panneau : %lu\n", prisParPanneau);
	std::fprintf(f, "traites par le fond      : %lu\n", prisParFond);
	std::fprintf(f, "appels du 2e gestionnaire: %lu (doit valoir le nombre de clics hors panneau)\n", secondAppele);
	std::fclose(f);
	return 0;
}