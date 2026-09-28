// PointeurGarde — exercice du chapitre 4.
// On garde EXPRES le pointeur rendu par PollEvent d'une trame a l'autre, et on
// affiche ce qu'il montre, avec son ADRESSE. La trame dure 60 ms pour que
// plusieurs evenements s'accumulent dans la file.
// Apres 6 s, on refait la meme chose avec PollEventCopy, qui rend une copie
// dont la duree de vie nous appartient.
// Releve dans pointeur.txt, a cote de l'executable.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkMouseEvent.h"
#include "NKEvent/NkKeyboardEvent.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;

// Ce que montre un evenement, en une ligne.
static void Decrire(const NkEvent *ev, char *out, size_t n) {
	if (!ev) { std::snprintf(out, n, "(nul)"); return; }
	if (auto *m = ev->As<const NkMouseMoveEvent>())
		std::snprintf(out, n, "%s x=%d y=%d", ev->GetName(), m->GetX(), m->GetY());
	else if (auto *k = ev->As<const NkKeyPressEvent>())
		std::snprintf(out, n, "%s touche=%d", ev->GetName(), (int)k->GetKey());
	else
		std::snprintf(out, n, "%s", ev->GetName());
}

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Pointeur garde d'une trame a l'autre";
	cfg.width = 760;
	cfg.height = 300;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	FILE *f = std::fopen("pointeur.txt", "w");

	NkEvent *garde = nullptr;        // DEFECTUEUX : pointeur garde entre trames
	char gardeAlors[96] = "(rien)";  // ce qu'il montrait au moment de la prise
	NkEventPtr copie;                // CORRIGE : une copie a nous
	char copieAlors[96] = "(rien)";

	const ULONGLONG depart = GetTickCount64();
	unsigned long trame = 0;
	bool corrige = false;

	while (window.IsOpen()) {
		++trame;
		if (!corrige && GetTickCount64() - depart > 6000) {
			corrige = true;
			garde = nullptr;
			std::fprintf(f, "\n########## ON PASSE A PollEventCopy ##########\n\n");
		}

		unsigned n = 0, reutilisations = 0;
		bool pris = false;

		if (!corrige) {
			while (NkEvent *ev = NkEvents().PollEvent()) {
				++n;
				if (ev == garde) ++reutilisations; // meme adresse que ce qu'on garde !
				if (ev->Is<NkWindowCloseEvent>()) window.Close();
				if (!pris && garde == nullptr && ev->Is<NkMouseMoveEvent>()) {
					garde = ev;                     // <== on garde le pointeur brut
					Decrire(garde, gardeAlors, sizeof(gardeAlors));
					pris = true;
				}
			}
			char maintenant[96];
			Decrire(garde, maintenant, sizeof(maintenant));
			if (garde) {
				std::fprintf(f, "trame %-3lu | %2u ev. | garde @%p pris comme [%s] | il montre MAINTENANT [%s]%s\n",
					trame, n, (void *)garde, gardeAlors, maintenant,
					reutilisations ? "  <== ADRESSE REUTILISEE cette trame" : "");
				std::fflush(f);
			}
		} else {
			while (NkEventPtr ev = NkEvents().PollEventCopy()) {
				++n;
				if (ev->Is<NkWindowCloseEvent>()) window.Close();
				if (!pris && copie.Get() == nullptr && ev->Is<NkMouseMoveEvent>()) {
					Decrire(ev.Get(), copieAlors, sizeof(copieAlors));
					copie = traits::NkMove(ev);     // <== on garde la copie
					pris = true;
				}
			}
			char maintenant[96];
			Decrire(copie.Get(), maintenant, sizeof(maintenant));
			if (copie.Get()) {
				std::fprintf(f, "trame %-3lu | %2u ev. | copie @%p prise comme [%s] | elle montre MAINTENANT [%s]\n",
					trame, n, (void *)copie.Get(), copieAlors, maintenant);
				std::fflush(f);
			}
		}

		Sleep(60); // trame longue expres : la file a le temps de se remplir
	}

	std::fclose(f);
	return 0;
}