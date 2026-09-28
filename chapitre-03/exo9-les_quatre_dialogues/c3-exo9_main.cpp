// QuatreDialogues — exercice 9 du chapitre 3.
// Ouvre les quatre dialogues natifs l'un apres l'autre et traite l'annulation
// dans chacun : on ne se sert de `path` QUE si `confirmed` est vrai.
// Le programme doit survivre a quatre fermetures sans rien choisir, et rendre 0.
// Releve dans dialogues.txt, a cote de l'executable.

#include "NKWindow/NKWindow.h"
#include "NKWindow/NKMain.h"

#include <cstdio>

using namespace nkentseu;

static FILE *gLog = nullptr;

// Le traitement correct : tester confirmed d'abord, path ensuite.
static void Traiter(const char *nom, const NkDialogResult &r) {
	std::fprintf(gLog, "%-18s confirmed=%-5s path=\"%s\" (%u octets)\n",
		nom, r.confirmed ? "true" : "false", r.path.CStr(), (unsigned)r.path.Size());

	if (!r.confirmed) {
		std::fprintf(gLog, "%-18s -> annule : je ne touche pas a path, je continue\n\n", "");
		return;
	}
	if (r.path.Size() == 0) {
		std::fprintf(gLog, "%-18s -> confirme mais chemin vide : je refuse aussi\n\n", "");
		return;
	}
	std::fprintf(gLog, "%-18s -> je peux me servir du chemin\n\n", "");
}

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Quatre dialogues";
	cfg.width = 560;
	cfg.height = 220;
	cfg.centered = true;

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	gLog = std::fopen("dialogues.txt", "w");
	std::fprintf(gLog, "Quatre dialogues natifs, fermes sans rien choisir\n\n");

	// 1 — ouvrir un fichier
	std::fprintf(gLog, "1) OpenFileDialog\n");
	NkDialogResult r1 = NkDialogs::OpenFileDialog("*.png;*.jpg", "Ouvrir une image");
	Traiter("OpenFileDialog", r1);
	std::fflush(gLog);

	// 2 — enregistrer un fichier
	std::fprintf(gLog, "2) SaveFileDialog\n");
	NkDialogResult r2 = NkDialogs::SaveFileDialog("nkref", "Enregistrer la planche");
	Traiter("SaveFileDialog", r2);
	std::fflush(gLog);

	// 3 — choisir un dossier
	std::fprintf(gLog, "3) OpenFolderDialog\n");
	NkDialogResult r3 = NkDialogs::OpenFolderDialog("Choisir un dossier");
	Traiter("OpenFolderDialog", r3);
	std::fflush(gLog);

	// 4 — boite de message : elle ne rend rien, l'annulation est la fermeture
	std::fprintf(gLog, "4) OpenMessageBox (ne rend aucun resultat)\n");
	NkDialogs::OpenMessageBox("Fichier introuvable", "Erreur", 2);
	std::fprintf(gLog, "%-18s -> revenu de la boite, le programme continue\n\n", "");
	std::fflush(gLog);

	std::fprintf(gLog, "Les quatre dialogues sont passes, la fenetre est %s\n",
		window.IsOpen() ? "toujours ouverte" : "fermee");

	// Le programme tourne encore : preuve qu'il n'a pas plante.
	unsigned long tours = 0;
	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) window.Close();
		}
		++tours;
	}

	std::fprintf(gLog, "Tours de boucle apres les dialogues : %lu\n", tours);
	std::fprintf(gLog, "Sortie normale, code 0\n");
	std::fclose(gLog);
	return 0;
}