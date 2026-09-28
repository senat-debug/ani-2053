// LesBornes — exercice 3 du chapitre 3.
// Deux fenetres : une avec une taille minimale imposee, une sans. On essaie de
// les reduire en dessous, par l'API du moteur puis par le systeme, et on note
// ce que chacun accepte. Releve dans bornes.txt, a cote de l'executable.

#include "NKWindow/NKWindow.h"
#include "NKWindow/NKMain.h"

#include <windows.h>
#include <cstdio>

using namespace nkentseu;

static void Exterieur(HWND h, long &l, long &ht) {
	RECT r{};
	GetWindowRect(h, &r);
	l = r.right - r.left;
	ht = r.bottom - r.top;
}

// Ce que la fenetre repond quand le systeme lui demande ses bornes.
static void BornesDeclarees(HWND h, long &l, long &ht) {
	MINMAXINFO mmi{};
	mmi.ptMinTrackSize.x = 0;
	mmi.ptMinTrackSize.y = 0;
	SendMessageW(h, WM_GETMINMAXINFO, 0, reinterpret_cast<LPARAM>(&mmi));
	l = mmi.ptMinTrackSize.x;
	ht = mmi.ptMinTrackSize.y;
}

static void Mesurer(FILE *f, const char *quoi, NkWindow &w) {
	HWND h = w.mData.mHwnd;
	long el = 0, eh = 0;
	Exterieur(h, el, eh);
	math::NkVec2u c = w.GetSize();
	std::fprintf(f, "  %-34s client %4u x %-4u | exterieur %4ld x %-4ld\n", quoi, c.x, c.y, el, eh);
}

static void Essais(FILE *f, const char *nom, NkWindow &w) {
	HWND h = w.mData.mHwnd;
	long bl = 0, bh = 0;
	BornesDeclarees(h, bl, bh);

	std::fprintf(f, "%s\n", nom);
	std::fprintf(f, "  bornes declarees au systeme (WM_GETMINMAXINFO) : %ld x %ld\n", bl, bh);
	Mesurer(f, "a l'ouverture", w);

	w.SetSize(1, 1);
	Mesurer(f, "apres SetSize(1,1)", w);

	w.SetSize(200, 150);
	Mesurer(f, "apres SetSize(200,150)", w);

	// Le chemin du systeme, sans passer par le moteur.
	SetWindowPos(h, nullptr, 0, 0, 1, 1, SWP_NOMOVE | SWP_NOZORDER);
	Mesurer(f, "apres SetWindowPos 1x1 (systeme)", w);

	w.SetSize(420, 240);
	std::fprintf(f, "\n");
}

int nkmain(const NkEntryState &state) {
	NkWindowConfig avec;
	avec.title = "Avec minimum 400x300";
	avec.width = 420;
	avec.height = 240;
	avec.minWidth = 400;
	avec.minHeight = 300;
	avec.centered = false;
	avec.x = 60;
	avec.y = 80;

	NkWindowConfig sans;
	sans.title = "Sans minimum";
	sans.width = 420;
	sans.height = 240;
	sans.minWidth = 1;
	sans.minHeight = 1;
	sans.centered = false;
	sans.x = 560;
	sans.y = 80;

	NkWindow f1, f2;
	f1.Create(avec);
	f2.Create(sans);

	FILE *f = std::fopen("bornes.txt", "w");
	std::fprintf(f, "Defauts de NkWindowConfig : minWidth=%u minHeight=%u\n\n",
		NkWindowConfig{}.minWidth, NkWindowConfig{}.minHeight);
	std::fprintf(f, "Facteur d echelle de l ecran (GetDpiScale) : %.2f\n", f1.GetDpiScale());
	std::fprintf(f, "Minimum impose par Windows (SM_CXMIN x SM_CYMIN) : %d x %d\n\n",
		GetSystemMetrics(SM_CXMIN), GetSystemMetrics(SM_CYMIN));
	Essais(f, "FENETRE 1 : minWidth=400 minHeight=300", f1);
	Essais(f, "FENETRE 2 : minWidth=1 minHeight=1 (minimum retire)", f2);
	std::fclose(f);

	bool reste = true;
	while (reste) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (auto *c = ev->As<NkWindowCloseEvent>()) {
				if (f1.IsOpen() && f1.GetId() == c->GetWindowId()) f1.Close();
				if (f2.IsOpen() && f2.GetId() == c->GetWindowId()) f2.Close();
			}
		}
		reste = f1.IsOpen() || f2.IsOpen();
	}
	return 0;
}