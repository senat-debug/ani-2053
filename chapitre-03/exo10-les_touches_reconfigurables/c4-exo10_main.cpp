// MesTouches — exercice du chapitre 4.
// Les trois actions du jeu sont ecrites dans touches.cfg, chargees au
// demarrage, et l'utilisateur peut les changer dans le programme (F2).
// Le fichier est ecrit avec NkInputCode::ToString / relu avec FromString
// (NkEventDispatcher.h:476-500), donc le joueur peut aussi l'editer a la main.
//
// Le jeu : amener le carre sur la cible, puis appuyer sur "Valider".
// Releve dans partie.txt.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkKeyboardEvent.h"

#include "NKCanvas/Renderer/Targets/NkRenderWindow.h"
#include "NKCanvas/Core/NkContextDesc.h"
#include "NKCanvas/Core/NkGraphicsApi.h"
#include "NKCanvas/Renderer/Shapes/NkRectangleShape.h"

#include <windows.h>
#include <cstdio>
#include <cstring>

using namespace nkentseu;
using namespace nkentseu::renderer;

static const char *kFichier = "touches.cfg";
static const int kNbActions = 3;
static const char *kActions[kNbActions] = {"Gauche", "Droite", "Valider"};

struct Config {
	NkInputCode codes[kNbActions];
};

static Config Defauts() {
	Config c;
	c.codes[0] = NkInputCode::Key(NkKey::NK_LEFT);
	c.codes[1] = NkInputCode::Key(NkKey::NK_RIGHT);
	c.codes[2] = NkInputCode::Key(NkKey::NK_SPACE);
	return c;
}

static void Sauver(const Config &c, FILE *journal) {
	FILE *f = std::fopen(kFichier, "w");
	if (!f) return;
	std::fprintf(f, "# touches.cfg — une action par ligne : Action=Appareil:Entree\n");
	for (int i = 0; i < kNbActions; ++i)
		std::fprintf(f, "%s=%s\n", kActions[i], c.codes[i].ToString().CStr());
	std::fclose(f);
	if (journal) std::fprintf(journal, "config ecrite dans %s\n", kFichier);
}

static bool Charger(Config &c, FILE *journal) {
	FILE *f = std::fopen(kFichier, "r");
	if (!f) return false;
	char ligne[128];
	int lues = 0;
	while (std::fgets(ligne, sizeof(ligne), f)) {
		if (ligne[0] == '#' || ligne[0] == '\n') continue;
		char *egal = std::strchr(ligne, '=');
		if (!egal) continue;
		*egal = '\0';
		char *valeur = egal + 1;
		char *fin = valeur + std::strlen(valeur);
		while (fin > valeur && (fin[-1] == '\n' || fin[-1] == '\r' || fin[-1] == ' ')) *--fin = '\0';
		for (int i = 0; i < kNbActions; ++i) {
			if (std::strcmp(ligne, kActions[i]) == 0) {
				bool valide = false;
				NkInputCode code = NkInputCode::FromString(valeur, &valide);
				if (valide) { c.codes[i] = code; ++lues; }
				else if (journal) std::fprintf(journal, "ligne ignoree : %s=%s (inconnu)\n", ligne, valeur);
			}
		}
	}
	std::fclose(f);
	if (journal) std::fprintf(journal, "%d action(s) relue(s) depuis %s\n", lues, kFichier);
	return lues > 0;
}

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Mes touches (F2 : changer)";
	cfg.width = 900;
	cfg.height = 340;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	NkContextDesc desc;
	desc.api = NkGraphicsApi::NK_GFX_API_OPENGL;
	NkRenderWindow cible(window, desc);
	if (!cible.IsValid()) return -2;

	FILE *journal = std::fopen("partie.txt", "w");

	Config conf = Defauts();
	if (!Charger(conf, journal)) {
		std::fprintf(journal, "pas de %s lisible : on ecrit les valeurs par defaut\n", kFichier);
		Sauver(conf, journal);
	}
	for (int i = 0; i < kNbActions; ++i)
		std::fprintf(journal, "  %-8s -> %s\n", kActions[i], conf.codes[i].ToString().CStr());
	std::fprintf(journal, "\n");
	std::fflush(journal);

	float x = 100.f, cibleX = 640.f;
	int points = 0;
	int enReglage = -1;   // -1 = on joue ; 0..2 = on attend la touche de l'action i
	unsigned long trames = 0;

	auto MajTitre = [&]() {
		char t[192];
		if (enReglage >= 0)
			std::snprintf(t, sizeof(t), "Appuyez sur la touche pour : %s", kActions[enReglage]);
		else
			std::snprintf(t, sizeof(t), "%d point(s) | %s=%s  %s=%s  %s=%s | F2 pour changer",
				points,
				kActions[0], conf.codes[0].ToString().CStr(),
				kActions[1], conf.codes[1].ToString().CStr(),
				kActions[2], conf.codes[2].ToString().CStr());
		window.SetTitle(t);
	};
	MajTitre();

	while (window.IsOpen()) {
		++trames;

		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) { window.Close(); continue; }
			auto *k = ev->As<NkKeyPressEvent>();
			if (!k) continue;

			// --- reglage des touches ------------------------------------
			if (enReglage >= 0) {
				if (k->GetKey() == NkKey::NK_ESCAPE) { enReglage = -1; MajTitre(); continue; }
				conf.codes[enReglage] = NkInputCode::Key(k->GetKey());
				std::fprintf(journal, "%-8s reglee sur %s\n",
					kActions[enReglage], conf.codes[enReglage].ToString().CStr());
				++enReglage;
				if (enReglage >= kNbActions) {
					enReglage = -1;
					Sauver(conf, journal);
					std::fprintf(journal, "\n");
				}
				std::fflush(journal);
				MajTitre();
				continue;
			}

			if (k->GetKey() == NkKey::NK_F2) { enReglage = 0; MajTitre(); continue; }

			// --- action "Valider" : on compare au code de la config -----
			if (conf.codes[2].device == NkInputDevice::NK_KEYBOARD &&
				(uint32)k->GetKey() == conf.codes[2].code) {
				if (x > cibleX - 60.f && x < cibleX + 60.f) {
					++points;
					cibleX = (cibleX > 450.f) ? 200.f : 700.f;
					std::fprintf(journal, "trame %-5lu point %d (valide avec %s)\n",
						trames, points, conf.codes[2].ToString().CStr());
					std::fflush(journal);
					MajTitre();
				}
			}
		}

		// --- deplacement : etat, lu depuis la config --------------------
		if (enReglage < 0) {
			const NkKeyboardInputState &clavier = NkEvents().GetInputState().GetKeyboard();
			if (conf.codes[0].device == NkInputDevice::NK_KEYBOARD &&
				clavier.IsKeyPressed((NkKey)conf.codes[0].code)) x -= 6.f;
			if (conf.codes[1].device == NkInputDevice::NK_KEYBOARD &&
				clavier.IsKeyPressed((NkKey)conf.codes[1].code)) x += 6.f;
		}

		const float largeur = (float)cible.GetSize().x;
		if (largeur > 100.f) {
			if (x < 10.f) x = 10.f;
			if (x > largeur - 60.f) x = largeur - 60.f;
		}

		cible.Clear(enReglage >= 0 ? NkColor2D{50, 40, 20, 255} : NkColor2D{26, 28, 34, 255});
		NkRectangleShape zone({120.f, 120.f});
		zone.SetPosition({cibleX - 60.f, 110.f});
		zone.SetFillColor(NkColor2D{60, 90, 60, 255});
		cible.Draw(zone);
		NkRectangleShape carre({50.f, 50.f});
		carre.SetPosition({x, 145.f});
		carre.SetFillColor(NkColor2D{130, 200, 255, 255});
		cible.Draw(carre);
		cible.Display();
	}

	std::fprintf(journal, "\npoints marques : %d\n", points);
	std::fprintf(journal, "config finale :\n");
	for (int i = 0; i < kNbActions; ++i)
		std::fprintf(journal, "  %-8s -> %s\n", kActions[i], conf.codes[i].ToString().CStr());
	std::fclose(journal);
	return 0;
}