// SeptCurseurs — exercice 6 du chapitre 3.
// Une seule fenetre, sept zones verticales. A chaque NkMouseMoveEvent, on
// regarde dans quelle zone est la souris et on pose la forme correspondante.
// Le programme note AUSSI la forme reellement affichee par le systeme
// (GetCursorInfo), pour comparer ce qui est demande et ce qui est obtenu.
//
// Touche U : on arrete de poser le curseur a chaque mouvement ; un seul appel
//            a ete fait au demarrage. On observe ce qui se passe ensuite.
// Releve dans curseurs.txt, a cote de l'executable.

#include "NKWindow/NKWindow.h"
#include "NKWindow/NKMain.h"
#include "NKEvent/NkMouseEvent.h"
#include "NKEvent/NkKeyboardEvent.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;

static const char *kNoms[7] = {
	"Arrow", "TextInput", "Hand", "ResizeNS", "ResizeWE", "ResizeNWSE", "ResizeNESW"};

static const NkWindow::NkCursorType kTypes[7] = {
	NkWindow::NkCursorType::Arrow, NkWindow::NkCursorType::TextInput, NkWindow::NkCursorType::Hand,
	NkWindow::NkCursorType::ResizeNS, NkWindow::NkCursorType::ResizeWE,
	NkWindow::NkCursorType::ResizeNWSE, NkWindow::NkCursorType::ResizeNESW};

static HCURSOR gNatifs[7];

static void ChargerNatifs() {
	gNatifs[0] = LoadCursorW(nullptr, IDC_ARROW);
	gNatifs[1] = LoadCursorW(nullptr, IDC_IBEAM);
	gNatifs[2] = LoadCursorW(nullptr, IDC_HAND);
	gNatifs[3] = LoadCursorW(nullptr, IDC_SIZENS);
	gNatifs[4] = LoadCursorW(nullptr, IDC_SIZEWE);
	gNatifs[5] = LoadCursorW(nullptr, IDC_SIZENWSE);
	gNatifs[6] = LoadCursorW(nullptr, IDC_SIZENESW);
}

// Quelle forme le systeme affiche-t-il en ce moment ?
static const char *FormeAffichee() {
	CURSORINFO ci{};
	ci.cbSize = sizeof(ci);
	if (!GetCursorInfo(&ci)) return "inconnu (GetCursorInfo a echoue)";
	if (ci.hCursor == nullptr) return "aucun (cache)";
	for (int i = 0; i < 7; ++i) {
		if (ci.hCursor == gNatifs[i]) return kNoms[i];
	}
	return "autre curseur systeme";
}

int nkmain(const NkEntryState &state) {
	ChargerNatifs();

	NkWindowConfig cfg;
	cfg.title = "Sept zones - survolez de gauche a droite (U = un seul appel)";
	cfg.width = 1120;
	cfg.height = 300;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	// L'appel unique du depart : la main.
	window.SetCursor(NkWindow::NkCursorType::Hand);

	bool poserAChaqueMouvement = true;
	const char *obtenu[7] = {"pas survolee", "pas survolee", "pas survolee", "pas survolee",
							 "pas survolee", "pas survolee", "pas survolee"};
	const char *obtenuUnique[7] = {"pas survolee", "pas survolee", "pas survolee", "pas survolee",
								   "pas survolee", "pas survolee", "pas survolee"};
	unsigned long mouvements = 0;
	unsigned long appels = 0;
	int zonePrecedente = -1;

	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) {
				window.Close();
			} else if (auto *k = ev->As<NkKeyPressEvent>()) {
				if (k->GetKey() == NkKey::NK_U) {
					poserAChaqueMouvement = false;
					window.SetCursor(NkWindow::NkCursorType::Hand); // l'unique appel
					++appels;
				}
			} else if (auto *m = ev->As<NkMouseMoveEvent>()) {
				++mouvements;
				const uint32 largeur = window.GetSize().x;
				const uint32 bande = largeur / 7;
				int zone = bande ? (int)(m->GetX() / bande) : 0;
				if (zone < 0) zone = 0;
				if (zone > 6) zone = 6;

				// Ce que le systeme affiche AVANT qu'on ne demande quoi que ce
				// soit : c'est le resultat de la demande precedente.
				if (zone == zonePrecedente) {
					if (poserAChaqueMouvement)
						obtenu[zone] = FormeAffichee();
					else
						obtenuUnique[zone] = FormeAffichee();
				}
				zonePrecedente = zone;

				if (poserAChaqueMouvement) {
					window.SetCursor(kTypes[zone]);
					++appels;
				}
			}
		}
		Sleep(1);
	}

	FILE *f = std::fopen("curseurs.txt", "w");
	std::fprintf(f, "Mouvements recus : %lu | appels a SetCursor : %lu\n\n", mouvements, appels);
	std::fprintf(f, "A) On pose le curseur a chaque mouvement\n");
	std::fprintf(f, "%-12s | %-12s | %s\n", "zone", "demande", "obtenu");
	for (int i = 0; i < 7; ++i)
		std::fprintf(f, "zone %d       | %-12s | %s\n", i + 1, kNoms[i], obtenu[i]);

	std::fprintf(f, "\nB) Un seul appel au demarrage (Hand), plus rien ensuite\n");
	std::fprintf(f, "%-12s | %-12s | %s\n", "zone", "demande", "obtenu");
	for (int i = 0; i < 7; ++i)
		std::fprintf(f, "zone %d       | %-12s | %s\n", i + 1, "Hand (une fois)", obtenuUnique[i]);
	std::fclose(f);
	return 0;
}