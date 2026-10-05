// alamain.cpp — le meme carre rouge, ecrit SANS coquille.
// Je monte les pieces moi-meme : la fenetre, la cible de rendu, l'horloge, et
// la boucle. Deux verifications, pas une : une fenetre peut echouer a s'ouvrir
// et un contexte graphique peut echouer a se creer.
// L'ordre de la boucle ne se negocie pas : le temps, les evenements, la mise a
// jour, Clear, le dessin, Display.
#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"

#include "NKCanvas/Core/NkContextDesc.h"
#include "NKCanvas/Core/NkGraphicsApi.h"
#include "NKCanvas/Renderer/Shapes/NkRectangleShape.h"
#include "NKCanvas/Renderer/Targets/NkRenderWindow.h"
#include "NKTime/NkClock.h"

using namespace nkentseu;
using namespace nkentseu::renderer;

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "A la main";
	cfg.width = 900;
	cfg.height = 300;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) {
		return -1; // la fenetre ne s'est pas ouverte
	}

	NkContextDesc desc;
	desc.api = NkGraphicsApi::NK_GFX_API_OPENGL;
	NkRenderWindow target(window, desc);
	if (!target.IsValid()) {
		return -2; // la fenetre existe, mais rien ne sait y dessiner
	}

	NkClock clock;
	float32 x = 0.f;
	bool running = true;

	while (running && window.IsOpen()) {
		// 1. le temps
		float32 dt = clock.Tick().delta;
		if (dt > 0.1f) {
			dt = 1.f / 60.f; // retour de veille : on ne rattrape pas
		}

		// 2. les evenements
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) {
				running = false;
				break;
			}
		}

		// 3. la mise a jour
		x += 100.f * dt; // cent pixels par seconde
		if (x > 900.f) {
			x = -60.f;
		}

		// 4. le dessin, entre Clear et Display
		target.Clear(NkColor2D{18, 18, 24, 255});
		NkRectangleShape carre({60.f, 60.f});
		carre.SetPosition({x, 120.f});
		carre.SetFillColor(NkColor2D{200, 60, 60, 255});
		target.Draw(carre);
		target.Display();
	}

	window.Close();
	return 0;
}