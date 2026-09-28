// TroisCarres — exercice du chapitre 4.
// Le meme carre avance a la fleche droite, de trois facons :
//   A. par EVENEMENTS        : NkKeyPressEvent / NkKeyRepeatEvent
//   B. par ETAT              : GetInputState().GetKeyboard().IsKeyPressed(...)
//   C. par ACTION NOMMEE     : NkActionManager, "AvancerADroite"
// Les trois carres sont dessines l'un sous l'autre ; le releve part dans
// trois-carres.txt.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkKeyboardEvent.h"

#include "NKCanvas/Renderer/Targets/NkRenderWindow.h"
#include "NKCanvas/Core/NkContextDesc.h"
#include "NKCanvas/Core/NkGraphicsApi.h"
#include "NKCanvas/Renderer/Shapes/NkRectangleShape.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;
using namespace nkentseu::renderer;

static const float kPas = 6.f; // le meme pas pour les trois

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Trois carres, une seule fleche droite";
	cfg.width = 900;
	cfg.height = 380;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	NkContextDesc desc;
	desc.api = NkGraphicsApi::NK_GFX_API_OPENGL;
	NkRenderWindow cible(window, desc);
	if (!cible.IsValid()) return -2;

	float xEvenement = 20.f, xEtat = 20.f, xAction = 20.f;
	unsigned long pasEvenement = 0, pasEtat = 0, pasAction = 0;
	unsigned long trames = 0;

	// --- C : l'action nommee, declaree une fois -------------------------
	// Le code du jeu ne parle plus de touche : il parle de "AvancerADroite".
	NkActionManager &actions = NkEvents().GetActionManager();
	actions.CreateAction("AvancerADroite",
		[&](const NkString &nom, const NkInputCode &code, bool enfoncee, bool repetition) {
			if (enfoncee) {
				xAction += kPas;
				++pasAction;
			}
		});
	// Deux sources pour la MEME action : la fleche droite et la touche D.
	actions.AddCommand(NkActionCommand("AvancerADroite", NkInputCode::Key(NkKey::NK_RIGHT)));
	actions.AddCommand(NkActionCommand("AvancerADroite", NkInputCode::Key(NkKey::NK_D)));

	while (window.IsOpen()) {
		++trames;

		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) { window.Close(); continue; }

			// --- A : par evenements -------------------------------------
			if (auto *k = ev->As<NkKeyPressEvent>()) {
				if (k->GetKey() == NkKey::NK_RIGHT) { xEvenement += kPas; ++pasEvenement; }
			} else if (auto *r = ev->As<NkKeyRepeatEvent>()) {
				if (r->GetKey() == NkKey::NK_RIGHT) { xEvenement += kPas; ++pasEvenement; }
			}
		}

		// --- B : par interrogation d'etat, une fois par trame ------------
		if (NkEvents().GetInputState().GetKeyboard().IsKeyPressed(NkKey::NK_RIGHT)) {
			xEtat += kPas;
			++pasEtat;
		}

		// --- dessin ------------------------------------------------------
		const float largeur = (float)cible.GetSize().x;
		cible.Clear(NkColor2D{26, 28, 34, 255});
		struct { float x; float y; NkColor2D c; } carres[3] = {
			{xEvenement, 40.f,  NkColor2D{120, 180, 255, 255}},
			{xEtat,      160.f, NkColor2D{255, 190, 110, 255}},
			{xAction,    280.f, NkColor2D{150, 230, 150, 255}},
		};
		for (auto &c : carres) {
			NkRectangleShape r({48.f, 48.f});
			r.SetPosition({c.x, c.y});
			r.SetFillColor(c.c);
			cible.Draw(r);
		}
		cible.Display();

		// On arrete les carres au bord, sans rien casser.
		if (largeur > 100.f) {
			if (xEvenement > largeur - 60.f) xEvenement = largeur - 60.f;
			if (xEtat > largeur - 60.f) xEtat = largeur - 60.f;
			if (xAction > largeur - 60.f) xAction = largeur - 60.f;
		}
	}

	FILE *f = std::fopen("trois-carres.txt", "w");
	std::fprintf(f, "trames dessinees : %lu\n\n", trames);
	std::fprintf(f, "%-22s | %-10s | %s\n", "methode", "pas faits", "x final");
	std::fprintf(f, "A. evenements          | %-10lu | %.0f\n", pasEvenement, xEvenement);
	std::fprintf(f, "B. etat (par trame)    | %-10lu | %.0f\n", pasEtat, xEtat);
	std::fprintf(f, "C. action nommee       | %-10lu | %.0f\n", pasAction, xAction);
	std::fclose(f);
	return 0;
}