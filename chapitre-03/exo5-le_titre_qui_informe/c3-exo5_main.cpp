// TitreQuiInforme — exercice 5 du chapitre 3.
// Le titre porte le nom du document, un asterisque s'il est modifie, et la
// taille courante de la fenetre. Il n'est reecrit QUE quand une de ces trois
// valeurs change, jamais a chaque tour de boucle.
//
// Frappe d'une touche ou d'un caractere  -> document modifie
// Ctrl+S                                 -> document enregistre
// Le compte des tours de boucle et des appels a SetTitle part dans titre.txt.

#include "NKWindow/NKWindow.h"
#include "NKWindow/NKMain.h"
#include "NKEvent/NkKeyboardEvent.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;

struct Etat {
	const char *document = "brouillon.txt";
	bool modifie = false;
	uint32 largeur = 0;
	uint32 hauteur = 0;

	bool operator!=(const Etat &o) const {
		return modifie != o.modifie || largeur != o.largeur || hauteur != o.hauteur;
	}
};

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "brouillon.txt";
	cfg.width = 900;
	cfg.height = 500;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	Etat etat;
	etat.largeur = window.GetSize().x;
	etat.hauteur = window.GetSize().y;

	Etat affiche;      // ce qui est REELLEMENT ecrit dans la barre de titre
	affiche.largeur = 0; // force la premiere ecriture
	affiche.hauteur = 0;

	FILE *f = std::fopen("titre.txt", "w");
	std::fprintf(f, "Titres reellement poses dans la barre :\n");

	unsigned long tours = 0;
	unsigned long appels = 0;

	while (window.IsOpen()) {
		++tours;

		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) {
				window.Close();
			} else if (auto *r = ev->As<NkWindowResizeEvent>()) {
				etat.largeur = r->GetWidth();
				etat.hauteur = r->GetHeight();
			} else if (auto *k = ev->As<NkKeyPressEvent>()) {
				if (k->GetKey() == NkKey::NK_S && k->HasCtrl())
					etat.modifie = false;   // « enregistre »
				else
					etat.modifie = true;
			} else if (ev->Is<NkTextInputEvent>()) {
				etat.modifie = true;
			}
		}

		// Le bon moment : quand l'etat a change, et seulement la.
		if (etat != affiche) {
			char titre[128];
			std::snprintf(titre, sizeof(titre), "%s%s - %u x %u",
				etat.document, etat.modifie ? " *" : "", etat.largeur, etat.hauteur);
			window.SetTitle(titre);
			affiche = etat;
			++appels;
			std::fprintf(f, "  %lu. tour %-7lu %s\n", appels, tours, titre);
		}

		Sleep(1); // sans rendu, on evite de bruler un coeur pour rien
	}

	std::fprintf(f, "\nTours de boucle : %lu\n", tours);
	std::fprintf(f, "Appels a SetTitle : %lu\n", appels);
	if (tours > 0)
		std::fprintf(f, "Soit 1 appel pour %.0f tours\n", (double)tours / (appels ? appels : 1));
	std::fclose(f);
	return 0;
}