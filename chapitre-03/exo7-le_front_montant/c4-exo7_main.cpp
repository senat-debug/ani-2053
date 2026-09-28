// SautEtFront — exercice du chapitre 4.
// Trois personnages sautent a l'espace :
//   A. etat NU        : if (IsKeyPressed(SPACE)) sauter();      -> sauts multiples
//   B. etat + FRONT   : on ne saute qu'au passage relache -> enfonce
//   C. par EVENEMENT  : NkKeyPressEvent (les repetitions sont ignorees)
// Meme physique pour les trois. Releve dans saut.txt.

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

struct Personnage {
	float y = 0.f;       // hauteur au-dessus du sol, en pixels
	float vy = 0.f;      // vitesse verticale
	unsigned long sauts = 0;
	unsigned long trameEnLAir = 0;

	void Sauter() {
		vy = 9.f;        // impulsion, sans aucune verification
		++sauts;
	}
	void Avancer() {
		vy -= 0.45f;     // gravite
		y += vy;
		if (y < 0.f) { y = 0.f; vy = 0.f; }
		if (y > 0.f) ++trameEnLAir;
	}
	bool AuSol() const { return y <= 0.f; }
};

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Saut a l'espace : etat nu / front / evenement";
	cfg.width = 900;
	cfg.height = 420;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	NkContextDesc desc;
	desc.api = NkGraphicsApi::NK_GFX_API_OPENGL;
	NkRenderWindow cible(window, desc);
	if (!cible.IsValid()) return -2;

	Personnage nu, front, evenement;
	bool espaceAvant = false;   // l'etat de la touche a la trame precedente
	unsigned long trames = 0, tramesEspaceEnfonce = 0;

	while (window.IsOpen()) {
		++trames;

		// --- C : par evenement ------------------------------------------
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) { window.Close(); continue; }
			if (auto *k = ev->As<NkKeyPressEvent>()) {
				if (k->GetKey() == NkKey::NK_SPACE) evenement.Sauter();
			}
			// NkKeyRepeatEvent volontairement ignore : une pression = un saut.
		}

		const bool espace = NkEvents().GetInputState().GetKeyboard().IsKeyPressed(NkKey::NK_SPACE);
		if (espace) ++tramesEspaceEnfonce;

		// --- A : etat nu, le defaut a montrer ---------------------------
		if (espace) nu.Sauter();

		// --- B : etat + detection de front ------------------------------
		if (espace && !espaceAvant) front.Sauter();
		espaceAvant = espace;

		nu.Avancer();
		front.Avancer();
		evenement.Avancer();

		// --- dessin ------------------------------------------------------
		const float hauteur = (float)cible.GetSize().y;
		const float sol = hauteur - 70.f;
		cible.Clear(NkColor2D{26, 28, 34, 255});
		struct { const Personnage *p; float x; NkColor2D c; } trio[3] = {
			{&nu, 120.f, NkColor2D{255, 120, 120, 255}},
			{&front, 420.f, NkColor2D{255, 200, 120, 255}},
			{&evenement, 720.f, NkColor2D{150, 230, 150, 255}},
		};
		for (auto &t : trio) {
			NkRectangleShape r({52.f, 52.f});
			r.SetPosition({t.x, sol - t.p->y});
			r.SetFillColor(t.c);
			cible.Draw(r);
		}
		cible.Display();
	}

	FILE *f = std::fopen("saut.txt", "w");
	std::fprintf(f, "trames                      : %lu\n", trames);
	std::fprintf(f, "trames avec espace enfonce  : %lu\n\n", tramesEspaceEnfonce);
	std::fprintf(f, "%-26s | %-10s | %s\n", "methode", "sauts", "trames en l'air");
	std::fprintf(f, "A. etat nu                 | %-10lu | %lu\n", nu.sauts, nu.trameEnLAir);
	std::fprintf(f, "B. etat + front            | %-10lu | %lu\n", front.sauts, front.trameEnLAir);
	std::fprintf(f, "C. evenement               | %-10lu | %lu\n", evenement.sauts, evenement.trameEnLAir);
	std::fclose(f);
	return 0;
}