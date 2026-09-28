// ZoneMorte — exercice du chapitre 4.
// Releve la valeur BRUTE d'un axe de manette au repos pendant dix secondes,
// garde le maximum en valeur absolue, et en deduit une zone morte.
// Brut = GetSnapshot(0).axes[...], AVANT ApplyDeadzone (NkGamepadSystem.h:581).
// Releve dans zone-morte.txt.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkGamepadSystem.h"

#include <windows.h>
#include <cstdio>
#include <cmath>

using namespace nkentseu;

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Zone morte : laissez la manette au repos 10 s";
	cfg.width = 760;
	cfg.height = 260;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	FILE *f = std::fopen("zone-morte.txt", "w");

	NkGamepadSystem &manettes = NkGamepads();
	std::fprintf(f, "manettes branchees      : %u\n", manettes.GetConnectedCount());
	std::fprintf(f, "manette 0 connectee     : %s\n", manettes.IsConnected(0) ? "oui" : "non");
	std::fprintf(f, "zone morte du moteur    : %.3f (valeur par defaut)\n\n", manettes.GetDeadzone());

	const int AXES = 4; // LX, LY, RX, RY
	const char *noms[AXES] = {"LX", "LY", "RX", "RY"};
	float maxi[AXES] = {0.f, 0.f, 0.f, 0.f};   // plus grande valeur absolue vue
	float mini[AXES] = {0.f, 0.f, 0.f, 0.f};   // plus petite (negative) vue
	float plus[AXES] = {0.f, 0.f, 0.f, 0.f};   // plus grande (positive) vue
	unsigned long nonNuls[AXES] = {0, 0, 0, 0};

	const ULONGLONG depart = GetTickCount64();
	unsigned long echantillons = 0;

	while (window.IsOpen() && GetTickCount64() - depart < 10000) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) window.Close();
		}

		if (manettes.IsConnected(0)) {
			const NkGamepadSnapshot &brut = manettes.GetSnapshot(0);
			++echantillons;
			for (int a = 0; a < AXES; ++a) {
				const float v = brut.axes[a];
				if (v != 0.f) ++nonNuls[a];
				if (std::fabs(v) > maxi[a]) maxi[a] = std::fabs(v);
				if (v < mini[a]) mini[a] = v;
				if (v > plus[a]) plus[a] = v;
			}
		}
		Sleep(4); // ~250 echantillons par seconde
	}

	const double secondes = (GetTickCount64() - depart) / 1000.0;
	std::fprintf(f, "duree du releve : %.1f s, echantillons : %lu\n\n", secondes, echantillons);

	if (echantillons == 0) {
		std::fprintf(f, "AUCUN ECHANTILLON : pas de manette branchee sur cette machine.\n");
		std::fprintf(f, "Le programme est pret ; il suffit de brancher une manette et de le relancer.\n");
	} else {
		std::fprintf(f, "%-5s | %-12s | %-12s | %-12s | %s\n",
			"axe", "max |v|", "min", "max", "echantillons non nuls");
		float pire = 0.f;
		for (int a = 0; a < AXES; ++a) {
			std::fprintf(f, "%-5s | %-12.4f | %-12.4f | %-12.4f | %lu\n",
				noms[a], maxi[a], mini[a], plus[a], nonNuls[a]);
			if (maxi[a] > pire) pire = maxi[a];
		}
		std::fprintf(f, "\nplus grande valeur absolue, tous axes : %.4f\n", pire);
		std::fprintf(f, "zone morte proposee (mesure x 1,5)    : %.3f\n", pire * 1.5f);
	}

	std::fclose(f);
	return 0;
}