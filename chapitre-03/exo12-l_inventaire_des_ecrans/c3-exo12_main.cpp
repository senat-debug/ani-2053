// InventaireEcrans — exercice 12 du chapitre 3.
// Pour chaque ecran branche : taille, position, facteur d'echelle, et lequel
// porte la fenetre. Puis on deplace la fenetre et on verifie que ca suit.
// Releve dans ecrans.txt, a cote de l'executable.

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"

#include <cstdio>

using namespace nkentseu;

static FILE *gLog = nullptr;

// Le moniteur courant et celui de la liste sont-ils le meme ? On compare la
// position et la taille : l'index de GetCurrentMonitor() n'est pas fiable.
static bool MemeEcran(const NkDisplayInfo &a, const NkDisplayInfo &b) {
	return a.posX == b.posX && a.posY == b.posY && a.width == b.width && a.height == b.height;
}

static void Inventaire(const char *quand, NkWindow &w) {
	const NkVector<NkDisplayInfo> ecrans = w.EnumerateMonitors();
	const NkDisplayInfo courant = w.GetCurrentMonitor();
	const math::NkVec2u pos = w.GetPosition();
	const math::NkVec2u taille = w.GetSize();

	std::fprintf(gLog, "===== %s =====\n", quand);
	std::fprintf(gLog, "fenetre : position %u , %u   taille %u x %u   echelle %.2f\n",
		pos.x, pos.y, taille.x, taille.y, w.GetDpiScale());
	std::fprintf(gLog, "GetMonitorCount() = %u, EnumerateMonitors() en rend %u\n",
		w.GetMonitorCount(), (unsigned)ecrans.Size());

	for (usize i = 0; i < ecrans.Size(); ++i) {
		const NkDisplayInfo &e = ecrans[i];
		std::fprintf(gLog,
			"  ecran %u \"%s\"%s\n"
			"      taille logique %u x %u | physique %u x %u\n"
			"      position %d , %d | echelle %.2f (%.0f x %.0f dpi) | %u Hz%s\n",
			e.index, e.name[0] ? e.name : "(sans nom)", e.isPrimary ? "  [principal]" : "",
			e.width, e.height, e.physWidth, e.physHeight,
			e.posX, e.posY, e.dpiScale, e.dpiX, e.dpiY, e.refreshRate,
			MemeEcran(e, courant) ? "   <== PORTE LA FENETRE" : "");
	}
	std::fprintf(gLog, "  GetCurrentMonitor() : index=%u \"%s\" position %d , %d echelle %.2f\n",
		courant.index, courant.name[0] ? courant.name : "(sans nom)",
		courant.posX, courant.posY, courant.dpiScale);
	std::fprintf(gLog, "  GetDisplaySize()     : %u x %u\n",
		w.GetDisplaySize().x, w.GetDisplaySize().y);
	std::fprintf(gLog, "  GetDisplayPosition() : %u , %u\n\n",
		w.GetDisplayPosition().x, w.GetDisplayPosition().y);
	std::fflush(gLog);
}

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Inventaire des ecrans";
	cfg.width = 640;
	cfg.height = 360;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	gLog = std::fopen("ecrans.txt", "w");
	Inventaire("A l'ouverture", window);

	// On promene la fenetre et on regarde si les valeurs suivent.
	window.SetPosition(40, 40);
	Inventaire("Apres SetPosition(40, 40)", window);

	window.SetPosition(1200, 500);
	Inventaire("Apres SetPosition(1200, 500)", window);

	// Position qui serait sur un second ecran a droite, s'il y en avait un.
	window.SetPosition(2200, 300);
	Inventaire("Apres SetPosition(2200, 300)", window);

	window.SetPosition(300, 200);

	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) window.Close();
			else if (ev->Is<NkWindowMoveEvent>()) Inventaire("Deplacement recu", window);
		}
	}

	std::fclose(gLog);
	return 0;
}