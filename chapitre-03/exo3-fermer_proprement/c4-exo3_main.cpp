// UnSeulChemin — exercice du chapitre 4.
// La fenetre ne se ferme QUE sur NkWindowCloseEvent : il n'y a qu'un seul
// appel a window.Close() dans tout le programme.
// Les trois declencheurs passent par ce meme chemin :
//   1. le bouton du systeme (la croix)
//   2. le raccourci du gestionnaire de fenetres (Alt+F4)
//   3. ma propre touche (Echap), qui ne ferme pas : elle DEPOSE un
//      NkWindowCloseEvent dans la file, comme le ferait le systeme.
// Les deux premieres demandes sont refusees pour montrer qu'on decide, la
// troisieme ferme. Releve dans fermeture.txt.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkKeyboardEvent.h"
#include "NKEvent/NkMouseEvent.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Un seul chemin de fermeture (Echap, Alt+F4, la croix)";
	cfg.width = 720;
	cfg.height = 300;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	FILE *f = std::fopen("fermeture.txt", "w");

	unsigned long demandes = 0;   // combien de NkWindowCloseEvent recus
	unsigned long fermetures = 0; // combien de fois Close() est appele
	unsigned long autres = 0;     // evenements qui NE ferment pas
	const char *cause = "systeme"; // mise a "Echap" juste avant notre depot

	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {

			// ---- LE SEUL CHEMIN DE FERMETURE ------------------------------
			if (auto *c = ev->As<NkWindowCloseEvent>()) {
				++demandes;
				std::fprintf(f, "demande %lu : NkWindowCloseEvent (forced=%s) venue de %s\n",
					demandes, c->IsForced() ? "true" : "false", cause);
				if (demandes < 3) {
					std::fprintf(f, "            -> refusee, la fenetre reste ouverte\n");
				} else {
					window.Close();          // <== unique appel a Close()
					++fermetures;
					std::fprintf(f, "            -> acceptee, Close() appele (%lu fois au total)\n",
						fermetures);
				}
				cause = "systeme";
				std::fflush(f);
				continue;
			}

			// ---- MA TOUCHE : elle ne ferme pas, elle DEMANDE --------------
			if (auto *k = ev->As<NkKeyPressEvent>()) {
				if (k->GetKey() == NkKey::NK_ESCAPE) {
					cause = "Echap (depose par le programme)";
					NkWindowCloseEvent demande(false, window.GetId());
					NkEvents().Enqueue_Public(demande, window.GetId());
					std::fprintf(f, "touche Echap : je depose un NkWindowCloseEvent dans la file\n");
					std::fflush(f);
					continue;
				}
			}

			// ---- TOUT LE RESTE NE FERME RIEN ------------------------------
			++autres;
		}
		Sleep(1);
	}

	std::fprintf(f, "\ndemandes de fermeture recues : %lu\n", demandes);
	std::fprintf(f, "appels a Close()             : %lu\n", fermetures);
	std::fprintf(f, "autres evenements recus      : %lu (aucun n'a ferme la fenetre)\n", autres);
	std::fclose(f);
	return 0;
}