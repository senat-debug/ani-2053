// LettreEtPosition — exercice du chapitre 4.
// Pour chaque touche pressee : la LETTRE (ce que la touche produit) et le CODE
// PHYSIQUE (ou la touche se trouve sur le clavier), plus le code natif du
// systeme. On garde les trois cote a cote pour voir lequel suit la disposition.
// Releve dans touches.txt, a cote de l'executable.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkKeyboardEvent.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Lettre et position - tapez des touches";
	cfg.width = 760;
	cfg.height = 300;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	FILE *f = std::fopen("touches.txt", "w");

	// La disposition active, telle que le systeme la declare.
	const HKL hkl = GetKeyboardLayout(0);
	std::fprintf(f, "Disposition active du thread : HKL = 0x%08llX (langue 0x%04X)\n\n",
		(unsigned long long)(uintptr_t)hkl, (unsigned)((uintptr_t)hkl & 0xFFFF));
	std::fprintf(f, "%-14s | %-16s | %-10s | %s\n",
		"NkKey (position)", "NkScancode", "VK natif", "caractere produit");
	std::fprintf(f, "---------------+------------------+------------+------------------\n");

	// On garde la derniere touche pressee pour lui coller son caractere, qui
	// arrive dans un evenement separe (NkTextInputEvent).
	NkKey derniereTouche = NkKey::NK_UNKNOWN;
	NkScancode dernierSc = NkScancode::NK_SC_UNKNOWN;
	uint32 dernierVk = 0;
	bool enAttenteDeTexte = false;

	auto Ecrire = [&](const char *carac) {
		std::fprintf(f, "%-14s | %-16s | 0x%02X (%3u) | %s\n",
			NkKeyToString(derniereTouche), NkScancodeToString(dernierSc),
			dernierVk, dernierVk, carac);
		std::fflush(f);
	};

	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) {
				window.Close();
			} else if (auto *k = ev->As<NkKeyPressEvent>()) {
				if (enAttenteDeTexte) Ecrire("(aucun)"); // touche sans caractere
				derniereTouche = k->GetKey();
				dernierSc = k->GetScancode();
				dernierVk = k->GetNativeKey();
				enAttenteDeTexte = true;
			} else if (auto *t = ev->As<NkTextInputEvent>()) {
				char buf[16];
				const uint32 cp = t->GetCodepoint();
				if (cp >= 32 && cp < 127)
					std::snprintf(buf, sizeof(buf), "'%c'  (U+%04X)", (char)cp, cp);
				else
					std::snprintf(buf, sizeof(buf), "U+%04X", cp);
				Ecrire(buf);
				enAttenteDeTexte = false;
			}
		}
		Sleep(1);
	}

	if (enAttenteDeTexte) Ecrire("(aucun)");
	std::fclose(f);
	return 0;
}