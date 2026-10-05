// interface.cpp — exercice 10 : l'interface qui ne defile pas.
//
// Une vue n'est pas un objet qu'on dessine : c'est un ETAT du renderer. Elle
// reste en place jusqu'a ce qu'on la change, et elle s'applique donc aussi a
// l'interface dessinee ensuite.
//
// Le monde est une rangee de carres plus large que la fenetre, et la vue avance
// avec dt. La barre du haut, elle, doit rester ou elle est.
//
// VERSION FAUTIVE : il suffit de commenter la ligne ResetView() ci-dessous et
// de mettre kReset a false. La barre se met alors a defiler avec le monde :
//
//     // r.ResetView();                 <- l'appel retire
//     static const bool kReset = false; <- pour le journal
//
#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"

#include "NKCanvas/Core/NkContextDesc.h"
#include "NKCanvas/Core/NkGraphicsApi.h"
#include "NKCanvas/Renderer/Core/NkRenderer2D.h"
#include "NKCanvas/Renderer/Resources/NkFont.h"
#include "NKCanvas/Renderer/Resources/NkSprite.h"
#include "NKCanvas/Renderer/Shapes/NkRectangleShape.h"
#include "NKCanvas/Renderer/Targets/NkRenderWindow.h"
#include "NKTime/NkClock.h"

#include <cstdio>

using namespace nkentseu;
using namespace nkentseu::renderer;

static const bool kReset = true; // sert au journal, pas au dessin

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = kReset ? "AVEC ResetView" : "SANS ResetView";
	cfg.width = 900;
	cfg.height = 320;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) {
		return -1;
	}

	NkContextDesc desc;
	desc.api = NkGraphicsApi::NK_GFX_API_OPENGL;
	NkRenderWindow cible(window, desc);
	if (!cible.IsValid()) {
		return -2;
	}

	renderer::NkFont police; // NkFont existe aussi dans nkentseu : on tranche
	const bool aPolice = police.LoadFromFile(*cible.GetRenderer(), "Resources/Fonts/Antonio-Bold.ttf");

	FILE *journal = std::fopen(kReset ? "journal_avec.txt" : "journal_sans.txt", "w");

	NkClock horloge;
	float32 centreX = 450.f; // le centre de la vue, en coordonnees monde
	float32 temps = 0.f;
	int releves = 0;

	while (window.IsOpen()) {
		float32 dt = horloge.Tick().delta;
		if (dt > 0.1f) {
			dt = 1.f / 60.f;
		}

		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) {
				window.Close();
			}
		}

		// Le monde defile AVEC dt, pas avec un compteur d'images : 120 pixels
		// par seconde, quelle que soit la machine.
		temps += dt;
		centreX += 120.f * dt;

		const float32 largeur = (float32)cible.GetSize().x;
		const float32 hauteur = (float32)cible.GetSize().y;

		cible.Clear(NkColor2D{18, 18, 24, 255});
		NkRenderer2D &r = cible.GetRenderer2D();

		// ---- le monde, vu a travers la vue --------------------------------
		NkView2D vue;
		vue.center = {centreX, hauteur * 0.5f};
		vue.size = {largeur, hauteur};
		r.SetView(vue);

		for (int i = 0; i < 40; ++i) {
			NkRectangleShape bloc({80.f, 80.f});
			bloc.SetPosition({(float32)i * 120.f, 170.f});
			bloc.SetFillColor((i % 2 == 0) ? NkColor2D{70, 110, 160, 255} : NkColor2D{60, 140, 110, 255});
			cible.Draw(bloc);
		}

		// ---- l'interface, en coordonnees ECRAN ----------------------------
		// La ligne qui fait tout : sans elle, la vue du monde s'applique encore
		// et la barre part avec lui.
		r.ResetView();

		NkRectangleShape barre({520.f, 48.f});
		barre.SetPosition({0.f, 0.f});
		barre.SetFillColor(NkColor2D{200, 90, 60, 255});
		cible.Draw(barre);

		if (aPolice) {
			// La position d'un NkText est sa LIGNE DE BASE : a y = 6 le texte
			// monterait hors de la fenetre.
			NkText titre(police, kReset ? "BARRE FIXE (ResetView)" : "BARRE QUI DEFILE (sans ResetView)", 22);
			titre.SetFillColor(NkColor2D{255, 255, 255, 255});
			titre.SetPosition({16.f, 34.f});
			cible.Draw(titre);
		}

		cible.Display();

		// ---- le journal : trois releves, un par seconde --------------------
		if (journal != nullptr && releves < 3 && temps >= (float32)(releves + 1)) {
			++releves;
			// La barre est dessinee a l'origine du repere courant. Avec
			// ResetView, ce repere est l'ecran, donc 0. Sans lui, c'est le
			// monde, et l'ecran la voit en largeur/2 - centre_x.
			const float32 barreX = kReset ? 0.f : (largeur * 0.5f - centreX);
			std::fprintf(journal, "reset : %s, centre_x : %.0f, barre_x : %.0f\n",
						 kReset ? "oui" : "non", centreX, barreX);
			std::fflush(journal);
		}
	}

	if (journal != nullptr) {
		std::fclose(journal);
	}
	return 0;
}