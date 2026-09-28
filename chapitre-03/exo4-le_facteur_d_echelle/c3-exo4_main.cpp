// FacteurEchelle — exercice 4 du chapitre 3.
// Affiche cote a cote : la taille rendue par la fenetre, celle rendue par la
// cible de rendu, et le facteur d'echelle. Le titre porte les trois valeurs,
// et le detail part dans echelle.txt.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"

#include "NKCanvas/Renderer/Targets/NkRenderWindow.h"
#include "NKCanvas/Core/NkContextDesc.h"
#include "NKCanvas/Core/NkGraphicsApi.h"

#include <cstdio>

using namespace nkentseu;
using namespace nkentseu::renderer;

static void Ligne(FILE *f, const char *quand, NkWindow &w, NkRenderWindow &t) {
	const math::NkVec2u fen = w.GetSize();
	const math::NkVec2u cib = t.GetSize();
	const float echelle = w.GetDpiScale();
	std::fprintf(f, "%-22s | fenetre %4u x %-4u | cible %4u x %-4u | echelle %.2f | ecart %+d x %+d\n",
		quand, fen.x, fen.y, cib.x, cib.y, echelle,
		(int)cib.x - (int)fen.x, (int)cib.y - (int)fen.y);
}

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Facteur d'echelle";
	cfg.width = 1280;
	cfg.height = 720;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	NkContextDesc desc;
	desc.api = NkGraphicsApi::NK_GFX_API_OPENGL;
	NkRenderWindow target(window, desc);
	if (!target.IsValid()) return -2;

	FILE *f = std::fopen("echelle.txt", "w");
	std::fprintf(f, "Demande dans la configuration : %u x %u\n", cfg.width, cfg.height);
	std::fprintf(f, "Ecran : %u x %u, %u moniteur(s)\n\n",
		window.GetDisplaySize().x, window.GetDisplaySize().y, window.GetMonitorCount());

	Ligne(f, "a l'ouverture", window, target);

	window.SetSize(1280, 720);
	target.OnResize(window.GetSize().x, window.GetSize().y);
	Ligne(f, "apres SetSize(1280,720)", window, target);

	window.SetSize(800, 450);
	target.OnResize(window.GetSize().x, window.GetSize().y);
	Ligne(f, "apres SetSize(800,450)", window, target);

	window.Maximize();
	target.OnResize(window.GetSize().x, window.GetSize().y);
	Ligne(f, "apres Maximize()", window, target);

	window.Restore();
	target.OnResize(window.GetSize().x, window.GetSize().y);
	Ligne(f, "apres Restore()", window, target);
	std::fclose(f);

	// Les trois valeurs, cote a cote, dans la barre de titre.
	char titre[160];
	const math::NkVec2u fen = window.GetSize();
	const math::NkVec2u cib = target.GetSize();
	std::snprintf(titre, sizeof(titre), "fenetre %ux%u | cible %ux%u | echelle %.2f",
		fen.x, fen.y, cib.x, cib.y, window.GetDpiScale());
	window.SetTitle(titre);

	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) window.Close();
		}
		target.Clear(NkColor2D{24, 26, 30, 255});
		target.Display();
	}
	return 0;
}