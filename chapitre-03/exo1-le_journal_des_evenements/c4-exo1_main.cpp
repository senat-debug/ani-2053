// JournalEvenements — exercice du chapitre 4.
// Ecrit dans le journal chaque evenement recu, avec sa famille (categorie) et
// son type, puis compte combien d'evenements chaque seconde produit.
// Releve dans journal.txt, a cote de l'executable.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkMouseEvent.h"
#include "NKEvent/NkKeyboardEvent.h"
#include "NKEvent/NkDropEvent.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;

// Les familles portees par un evenement, en clair.
static void Familles(uint32 flags, char *out, size_t taille) {
	struct { uint32 bit; const char *nom; } table[] = {
		{NkEventCategory::NK_CAT_APPLICATION, "APPLICATION"},
		{NkEventCategory::NK_CAT_INPUT, "INPUT"},
		{NkEventCategory::NK_CAT_KEYBOARD, "KEYBOARD"},
		{NkEventCategory::NK_CAT_MOUSE, "MOUSE"},
		{NkEventCategory::NK_CAT_WINDOW, "WINDOW"},
		{NkEventCategory::NK_CAT_GRAPHICS, "GRAPHICS"},
		{NkEventCategory::NK_CAT_TOUCH, "TOUCH"},
		{NkEventCategory::NK_CAT_GAMEPAD, "GAMEPAD"},
		{NkEventCategory::NK_CAT_CUSTOM, "CUSTOM"},
		{NkEventCategory::NK_CAT_TRANSFER, "TRANSFER"},
		{NkEventCategory::NK_CAT_GENERIC_HID, "HID"},
		{NkEventCategory::NK_CAT_DROP, "DROP"},
		{NkEventCategory::NK_CAT_SYSTEM, "SYSTEM"},
	};
	out[0] = '\0';
	for (auto &e : table) {
		if (flags & e.bit) {
			if (out[0]) strncat(out, "|", taille - strlen(out) - 1);
			strncat(out, e.nom, taille - strlen(out) - 1);
		}
	}
	if (!out[0]) strncpy(out, "(aucune)", taille);
}

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Journal des evenements - bougez, tapez, redimensionnez, deposez";
	cfg.width = 820;
	cfg.height = 420;
	cfg.centered = true;
	cfg.dropEnabled = true; // sans ca, aucun evenement de depot n'arrive (3.2.3)

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	FILE *f = std::fopen("journal.txt", "w");
	std::fprintf(f, "%-10s | %-28s | %-22s | %s\n", "ms", "type", "familles", "detail");

	const ULONGLONG depart = GetTickCount64();
	unsigned long total = 0;
	unsigned long parSeconde[64] = {};   // compteur par seconde, 64 s au plus
	unsigned long parType[512] = {};     // compteur par type d'evenement

	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			const ULONGLONG ms = GetTickCount64() - depart;
			const unsigned seconde = (unsigned)(ms / 1000);
			++total;
			if (seconde < 64) ++parSeconde[seconde];
			const unsigned t = (unsigned)ev->GetType();
			if (t < 512) ++parType[t];

			char fam[160];
			Familles(ev->GetCategoryFlags(), fam, sizeof(fam));

			char detail[128] = "";
			if (auto *m = ev->As<NkMouseMoveEvent>())
				std::snprintf(detail, sizeof(detail), "x=%d y=%d", m->GetX(), m->GetY());
			else if (auto *k = ev->As<NkKeyPressEvent>())
				std::snprintf(detail, sizeof(detail), "touche=%d ctrl=%d", (int)k->GetKey(), k->HasCtrl() ? 1 : 0);
			else if (auto *ti = ev->As<NkTextInputEvent>())
				std::snprintf(detail, sizeof(detail), "codepoint=%u", ti->GetCodepoint());
			else if (auto *r = ev->As<NkWindowResizeEvent>())
				std::snprintf(detail, sizeof(detail), "%u x %u", r->GetWidth(), r->GetHeight());
			else if (auto *d = ev->As<NkDropFileEvent>())
				std::snprintf(detail, sizeof(detail), "%u fichier(s)", (unsigned)d->data.Count());

			std::fprintf(f, "%-10llu | %-28s | %-22s | %s\n",
				(unsigned long long)ms, ev->GetName(), fam, detail);

			if (ev->Is<NkWindowCloseEvent>()) window.Close();
		}
		Sleep(1);
	}

	const ULONGLONG duree = GetTickCount64() - depart;
	std::fprintf(f, "\n===== COMPTES =====\n");
	std::fprintf(f, "duree de la seance : %.1f s\n", duree / 1000.0);
	std::fprintf(f, "evenements recus   : %lu\n", total);
	std::fprintf(f, "moyenne            : %.1f par seconde\n\n", total / (duree / 1000.0));

	std::fprintf(f, "par seconde :\n");
	unsigned long maxi = 0;
	for (unsigned s = 0; s < 64 && s * 1000 < duree; ++s) {
		std::fprintf(f, "  seconde %-2u : %lu\n", s, parSeconde[s]);
		if (parSeconde[s] > maxi) maxi = parSeconde[s];
	}
	std::fprintf(f, "  pointe : %lu evenements en une seconde\n\n", maxi);

	std::fprintf(f, "par type :\n");
	for (unsigned t = 0; t < 512; ++t)
		if (parType[t])
			std::fprintf(f, "  %-28s %lu\n", NkEventType::ToString((NkEventType::Value)t).CStr(), parType[t]);
	std::fclose(f);
	return 0;
}