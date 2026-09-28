// DeuxFenetres — exercice 11 du chapitre 3.
// Deux fenetres, et pour chaque clic on dit laquelle l'a recu : l'evenement
// porte l'identifiant de sa fenetre (NkEvent::GetWindowId, NkEvent.h:489).
// On essaie aussi de DESSINER dans les deux, avec une cible de rendu par
// fenetre, pour voir ce qui marche et ce qui manque.
// Releve dans deux-fenetres.txt, a cote de l'executable.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkMouseEvent.h"

#include "NKCanvas/Renderer/Targets/NkRenderWindow.h"
#include "NKCanvas/Core/NkContextDesc.h"
#include "NKCanvas/Core/NkGraphicsApi.h"
#include "NKCanvas/Renderer/Shapes/NkRectangleShape.h"

#include <cstdio>

using namespace nkentseu;
using namespace nkentseu::renderer;

int nkmain(const NkEntryState &state) {
	NkWindowConfig ca;
	ca.title = "Fenetre A";
	ca.width = 520;
	ca.height = 320;
	ca.centered = false;
	ca.x = 80;
	ca.y = 120;

	NkWindowConfig cb = ca;
	cb.title = "Fenetre B";
	cb.x = 680;

	NkWindow A(ca), B(cb);
	if (!A.IsOpen() || !B.IsOpen()) return -1;

	FILE *f = std::fopen("deux-fenetres.txt", "w");
	std::fprintf(f, "Fenetre A : id=%llu\n", (unsigned long long)A.GetId());
	std::fprintf(f, "Fenetre B : id=%llu\n\n", (unsigned long long)B.GetId());

	// Une cible de rendu par fenetre : est-ce que les deux tiennent ?
	NkContextDesc desc;
	desc.api = NkGraphicsApi::NK_GFX_API_OPENGL;
	NkRenderWindow cibleA(A, desc);
	NkRenderWindow cibleB(B, desc);
	std::fprintf(f, "cible de rendu A valide : %s\n", cibleA.IsValid() ? "oui" : "NON");
	std::fprintf(f, "cible de rendu B valide : %s\n", cibleB.IsValid() ? "oui" : "NON");
	std::fprintf(f, "meme renderer pour les deux : %s\n\n",
		(cibleA.GetRenderer() == cibleB.GetRenderer()) ? "oui" : "non (un par fenetre)");
	std::fflush(f);

	unsigned long clics = 0, clicsA = 0, clicsB = 0;
	unsigned long imagesA = 0, imagesB = 0;

	while (A.IsOpen() || B.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			const uint64 id = ev->GetWindowId();
			const char *nom = (id == A.GetId()) ? "A" : (id == B.GetId() ? "B" : "?");

			if (ev->Is<NkWindowCloseEvent>()) {
				if (id == A.GetId() && A.IsOpen()) A.Close();
				if (id == B.GetId() && B.IsOpen()) B.Close();
				std::fprintf(f, "fermeture demandee par la fenetre %s\n", nom);
			} else if (auto *b = ev->As<NkMouseButtonPressEvent>()) {
				++clics;
				if (id == A.GetId()) ++clicsA;
				else if (id == B.GetId()) ++clicsB;
				std::fprintf(f, "clic %-3lu recu par la fenetre %-2s (id=%llu) en %d , %d\n",
					clics, nom, (unsigned long long)id, b->GetX(), b->GetY());
				std::fflush(f);

				char titre[64];
				std::snprintf(titre, sizeof(titre), "Fenetre A - %lu clic(s)", clicsA);
				A.SetTitle(titre);
				std::snprintf(titre, sizeof(titre), "Fenetre B - %lu clic(s)", clicsB);
				B.SetTitle(titre);
			}
		}

		// Dessin : chaque fenetre a sa propre cible, donc son propre Clear.
		if (A.IsOpen() && cibleA.IsValid()) {
			cibleA.Clear(NkColor2D{30, 60, 110, 255});
			NkRectangleShape r({120.f, 80.f});
			r.SetPosition({40.f, 40.f});
			r.SetFillColor(NkColor2D{120, 180, 255, 255});
			cibleA.Draw(r);
			cibleA.Display();
			++imagesA;
		}
		if (B.IsOpen() && cibleB.IsValid()) {
			cibleB.Clear(NkColor2D{110, 50, 30, 255});
			NkRectangleShape r({120.f, 80.f});
			r.SetPosition({40.f, 40.f});
			r.SetFillColor(NkColor2D{255, 170, 120, 255});
			cibleB.Draw(r);
			cibleB.Display();
			++imagesB;
		}
	}

	std::fprintf(f, "\nclics : %lu au total (A : %lu, B : %lu)\n", clics, clicsA, clicsB);
	std::fprintf(f, "images dessinees : A = %lu, B = %lu\n", imagesA, imagesB);
	std::fclose(f);
	return 0;
}