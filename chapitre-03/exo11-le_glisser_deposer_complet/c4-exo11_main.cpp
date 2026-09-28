// SixDepots — exercice du chapitre 4.
// Traite les SIX evenements de depot : entree, survol, sortie, et les trois
// natures de contenu (fichier, texte, image). Retour visuel pendant le survol :
// le fond s'eclaire, un cadre epais apparait, et un marqueur suit le curseur.
//
// Touche S : joue une sequence simulee (entree -> survols -> texte), pour
// verifier les six chemins sans souris. Releve dans depots.txt.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkDropEvent.h"
#include "NKEvent/NkKeyboardEvent.h"

#include "NKCanvas/Renderer/Targets/NkRenderWindow.h"
#include "NKCanvas/Core/NkContextDesc.h"
#include "NKCanvas/Core/NkGraphicsApi.h"
#include "NKCanvas/Renderer/Shapes/NkRectangleShape.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;
using namespace nkentseu::renderer;

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Six depots — deposez un fichier ou du texte (S = sequence simulee)";
	cfg.width = 880;
	cfg.height = 420;
	cfg.centered = true;
	cfg.dropEnabled = true; // sans ca, aucun evenement de depot (3.2.3)

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	NkContextDesc desc;
	desc.api = NkGraphicsApi::NK_GFX_API_OPENGL;
	NkRenderWindow cible(window, desc);
	if (!cible.IsValid()) return -2;

	FILE *f = std::fopen("depots.txt", "w");
	std::fprintf(f, "dropEnabled = %s\n\n", cfg.dropEnabled ? "true" : "false");

	// Etat du retour visuel
	bool survol = false;         // un contenu est au-dessus de la fenetre
	int mx = 0, my = 0;          // derniere position de survol
	char resume[160] = "";       // ce que le survol annonce
	char dernierDepot[200] = "(aucun depot)";
	unsigned long nEnter = 0, nOver = 0, nLeave = 0, nFile = 0, nText = 0, nImage = 0;
	unsigned long trames = 0;

	while (window.IsOpen()) {
		++trames;

		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) { window.Close(); continue; }

			// ---------- 1. ENTREE ----------------------------------------
			if (auto *e = ev->As<NkDropEnterEvent>()) {
				++nEnter;
				survol = true;
				mx = e->data.x; my = e->data.y;
				std::snprintf(resume, sizeof(resume), "%u fichier(s) | texte : %s | image : %s",
					e->data.numFiles, e->data.hasText ? "oui" : "non",
					e->data.hasImage ? "oui" : "non");
				std::fprintf(f, "ENTREE  en %d,%d : %s\n", mx, my, resume);
				std::fflush(f);
			}
			// ---------- 2. SURVOL ----------------------------------------
			else if (auto *o = ev->As<NkDropOverEvent>()) {
				++nOver;
				survol = true;
				mx = o->data.x; my = o->data.y;
				std::fprintf(f, "SURVOL  en %d,%d\n", mx, my);
			}
			// ---------- 3. SORTIE ----------------------------------------
			else if (ev->Is<NkDropLeaveEvent>()) {
				++nLeave;
				survol = false;
				resume[0] = '\0';
				std::fprintf(f, "SORTIE  : le contenu quitte la fenetre\n");
				std::fflush(f);
			}
			// ---------- 4. CONTENU : FICHIERS ----------------------------
			else if (auto *d = ev->As<NkDropFileEvent>()) {
				++nFile;
				survol = false;
				std::snprintf(dernierDepot, sizeof(dernierDepot), "%u fichier(s) en %d,%d",
					d->data.Count(), d->data.x, d->data.y);
				std::fprintf(f, "DEPOT FICHIER : %s\n", dernierDepot);
				for (uint32 i = 0; i < d->data.Count() && i < 5; ++i)
					std::fprintf(f, "    %s\n", d->data.paths[i].CStr());
				std::fflush(f);
			}
			// ---------- 5. CONTENU : TEXTE -------------------------------
			else if (auto *t = ev->As<NkDropTextEvent>()) {
				++nText;
				survol = false;
				std::snprintf(dernierDepot, sizeof(dernierDepot), "texte (%s) : \"%s\"",
					t->data.mimeType.CStr(), t->data.text.CStr());
				std::fprintf(f, "DEPOT TEXTE   : %s\n", dernierDepot);
				std::fflush(f);
			}
			// ---------- 6. CONTENU : IMAGE -------------------------------
			else if (auto *i = ev->As<NkDropImageEvent>()) {
				++nImage;
				survol = false;
				std::snprintf(dernierDepot, sizeof(dernierDepot), "image %ux%u (%s)",
					i->data.width, i->data.height, i->data.mimeType.CStr());
				std::fprintf(f, "DEPOT IMAGE   : %s\n", dernierDepot);
				std::fflush(f);
			}
			// ---------- la sequence simulee ------------------------------
			else if (auto *k = ev->As<NkKeyPressEvent>()) {
				if (k->GetKey() == NkKey::NK_S) {
					std::fprintf(f, "\n--- sequence simulee (evenements deposes par le programme) ---\n");
					NkDropEnterData ed; ed.x = 120; ed.y = 90; ed.numFiles = 2; ed.hasText = true;
					NkDropEnterEvent e1(ed, window.GetId());
					NkEvents().Enqueue_Public(e1, window.GetId());
					for (int p = 0; p < 3; ++p) {
						NkDropOverData od; od.x = 200 + p * 150; od.y = 160 + p * 40;
						NkDropOverEvent e2(od, window.GetId());
						NkEvents().Enqueue_Public(e2, window.GetId());
					}
				} else if (k->GetKey() == NkKey::NK_T) {
					NkDropTextData td; td.x = 400; td.y = 220;
					td.text = "bonjour depuis le glisser-deposer";
					td.mimeType = "text/plain";
					NkDropTextEvent e3(td, window.GetId());
					NkEvents().Enqueue_Public(e3, window.GetId());
				} else if (k->GetKey() == NkKey::NK_L) {
					NkDropLeaveEvent e4(window.GetId());
					NkEvents().Enqueue_Public(e4, window.GetId());
				}
			}
		}

		// ---------- le retour visuel ------------------------------------
		const float L = (float)cible.GetSize().x, H = (float)cible.GetSize().y;
		cible.Clear(survol ? NkColor2D{30, 60, 45, 255} : NkColor2D{26, 28, 34, 255});

		if (survol) {
			// cadre epais, comme dans les vrais outils
			const float e = 10.f;
			NkColor2D vert{90, 200, 140, 255};
			NkRectangleShape haut({L, e}); haut.SetPosition({0, 0}); haut.SetFillColor(vert); cible.Draw(haut);
			NkRectangleShape bas({L, e}); bas.SetPosition({0, H - e}); bas.SetFillColor(vert); cible.Draw(bas);
			NkRectangleShape g({e, H}); g.SetPosition({0, 0}); g.SetFillColor(vert); cible.Draw(g);
			NkRectangleShape d({e, H}); d.SetPosition({L - e, 0}); d.SetFillColor(vert); cible.Draw(d);

			// marqueur sous le curseur
			NkRectangleShape m({28.f, 28.f});
			m.SetPosition({(float)mx - 14.f, (float)my - 14.f});
			m.SetFillColor(NkColor2D{220, 255, 220, 255});
			cible.Draw(m);
		}
		cible.Display();

		char t[220];
		std::snprintf(t, sizeof(t), "%s | entree %lu, survol %lu, sortie %lu | fichier %lu, texte %lu, image %lu | %s",
			survol ? resume : "en attente", nEnter, nOver, nLeave, nFile, nText, nImage, dernierDepot);
		window.SetTitle(t);
	}

	std::fprintf(f, "\n%-16s %lu\n", "ENTREE  recues :", nEnter);
	std::fprintf(f, "%-16s %lu\n", "SURVOL  recus  :", nOver);
	std::fprintf(f, "%-16s %lu\n", "SORTIE  recues :", nLeave);
	std::fprintf(f, "%-16s %lu\n", "FICHIER recus  :", nFile);
	std::fprintf(f, "%-16s %lu\n", "TEXTE   recus  :", nText);
	std::fprintf(f, "%-16s %lu\n", "IMAGE   recus  :", nImage);
	std::fprintf(f, "trames : %lu\n", trames);
	std::fclose(f);
	return 0;
}