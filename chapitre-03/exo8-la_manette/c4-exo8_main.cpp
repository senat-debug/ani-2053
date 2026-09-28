// ClavierEtManette — exercice du chapitre 4.
// Le meme carre est pilote au clavier ET a la manette, SANS ecrire deux fois
// la logique : un seul axe nomme "Horizontal", plusieurs sources branchees
// dessus. Le code du jeu lit une valeur entre -1 et +1, et ne sait pas d'ou
// elle vient.
// Debranchement : sur NkGamepadDisconnectEvent, le jeu se MET EN PAUSE.
// (Touche X : simule un debranchement, en deposant l'evenement soi-meme.)
// Releve dans clavier-manette.txt.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkKeyboardEvent.h"
#include "NKEvent/NkGamepadEvent.h"

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
	cfg.title = "Clavier et manette, une seule logique";
	cfg.width = 900;
	cfg.height = 360;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	NkContextDesc desc;
	desc.api = NkGraphicsApi::NK_GFX_API_OPENGL;
	NkRenderWindow cible(window, desc);
	if (!cible.IsValid()) return -2;

	FILE *f = std::fopen("clavier-manette.txt", "w");

	float axeH = 0.f;          // la seule entree que le jeu connait
	char sourceDuDernier[32] = "aucune";

	// --- UN SEUL axe nomme, PLUSIEURS sources ---------------------------
	NkAxisManager &axes = NkEvents().GetAxisManager();
	axes.CreateAxis("Horizontal",
		[&](const NkString &nom, const NkInputCode &code, float valeur) {
			// Appele une fois par commande et par trame. On garde la source
			// qui pousse le plus fort : clavier ou manette, peu importe.
			if (valeur > 0.001f || valeur < -0.001f) {
				if ((valeur < 0 ? -valeur : valeur) > (axeH < 0 ? -axeH : axeH)) {
					axeH = valeur;
					std::snprintf(sourceDuDernier, sizeof(sourceDuDernier), "%s",
						code.device == NkInputDevice::NK_KEYBOARD ? "clavier" : "manette");
				}
			}
		});
	axes.AddCommand(NkAxisCommand("Horizontal", NkInputCode::Key(NkKey::NK_RIGHT), +1.f));
	axes.AddCommand(NkAxisCommand("Horizontal", NkInputCode::Key(NkKey::NK_LEFT), -1.f));
	axes.AddCommand(NkAxisCommand("Horizontal", NkInputCode::Key(NkKey::NK_D), +1.f));
	axes.AddCommand(NkAxisCommand("Horizontal", NkInputCode::Key(NkKey::NK_A), -1.f));
	axes.AddCommand(NkAxisCommand("Horizontal", NkInputCode::GamepadAxis(NkGamepadAxis::NK_GP_AXIS_LX), +1.f));

	float x = 420.f;
	bool enPause = false;
	unsigned long trames = 0, tramesEnPause = 0, tramesQuiBougent = 0;
	unsigned long debranchements = 0, rebranchements = 0;

	std::fprintf(f, "manettes vues au demarrage : %u\n\n",
		NkEvents().GetInputState().GetGamepads().GetSlot(0) &&
		NkEvents().GetInputState().GetGamepads().GetSlot(0)->IsConnected() ? 1u : 0u);

	while (window.IsOpen()) {
		++trames;
		axeH = 0.f; // remis a zero : les commandes vont le remplir cette trame

		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) { window.Close(); continue; }

			// --- le debranchement met le jeu en pause --------------------
			if (auto *d = ev->As<NkGamepadDisconnectEvent>()) {
				enPause = true;
				++debranchements;
				std::fprintf(f, "trame %-5lu manette %u DEBRANCHEE -> PAUSE (x = %.0f)\n",
					trames, d->GetGamepadIndex(), x);
				window.SetTitle("PAUSE - manette debranchee (Entree pour reprendre)");
				std::fflush(f);
			} else if (auto *c = ev->As<NkGamepadConnectEvent>()) {
				enPause = false;
				++rebranchements;
				std::fprintf(f, "trame %-5lu manette %u rebranchee -> reprise\n",
					trames, c->GetGamepadIndex());
				window.SetTitle("Clavier et manette, une seule logique");
				std::fflush(f);
			} else if (auto *k = ev->As<NkKeyPressEvent>()) {
				if (k->GetKey() == NkKey::NK_X) {
					// Simulation d'un debranchement, faute de manette reelle.
					NkGamepadDisconnectEvent faux(0, window.GetId());
					NkEvents().Enqueue_Public(faux, window.GetId());
				} else if (k->GetKey() == NkKey::NK_ENTER && enPause) {
					enPause = false;
					std::fprintf(f, "trame %-5lu reprise demandee au clavier (x = %.0f)\n", trames, x);
					window.SetTitle("Clavier et manette, une seule logique");
					std::fflush(f);
				}
			}
		}

		// --- LA LOGIQUE, ecrite UNE SEULE FOIS --------------------------
		if (!enPause) {
			if (axeH != 0.f) ++tramesQuiBougent;
			x += axeH * 7.f;
		} else {
			++tramesEnPause;
		}

		const float largeur = (float)cible.GetSize().x;
		if (largeur > 100.f) {
			if (x < 10.f) x = 10.f;
			if (x > largeur - 70.f) x = largeur - 70.f;
		}

		cible.Clear(enPause ? NkColor2D{60, 30, 30, 255} : NkColor2D{26, 28, 34, 255});
		NkRectangleShape r({56.f, 56.f});
		r.SetPosition({x, 150.f});
		r.SetFillColor(enPause ? NkColor2D{120, 120, 120, 255} : NkColor2D{130, 200, 255, 255});
		cible.Draw(r);
		cible.Display();
	}

	std::fprintf(f, "\ntrames                 : %lu\n", trames);
	std::fprintf(f, "trames ou le carre bouge : %lu\n", tramesQuiBougent);
	std::fprintf(f, "trames en pause          : %lu\n", tramesEnPause);
	std::fprintf(f, "debranchements recus     : %lu\n", debranchements);
	std::fprintf(f, "rebranchements recus     : %lu\n", rebranchements);
	std::fprintf(f, "derniere source vue      : %s\n", sourceDuDernier);
	std::fprintf(f, "x final                  : %.0f\n", x);
	std::fclose(f);
	return 0;
}