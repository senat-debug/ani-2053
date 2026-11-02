// Exercice 6 : le singleton et le test
//
// La regle : un personnage saute s'il est au sol et que la barre d'espace est
// enfoncee. Elle existe ici en deux versions :
//   SauterDirect  lit NkInput, le singleton du kit : on ne peut pas lui dire
//                 que la touche est enfoncee, donc on ne peut pas la tester ;
//   Sauter        recoit l'etat de la touche en parametre, et se teste.
//
// Le banc tourne dans OnCommandLine, avant toute fenetre. Il ecrit banc.log a
// cote de l'executable lance et rend le nombre d'echecs comme code de sortie.
#include "NKCanvas/App/NkCanvasApp.h"
#include "NKEvent/NkEventDispatcher.h"
#include "NKEvent/NkKeyboardEvent.h"
// fournit WinMain et les autres points d'entree natifs qui appellent nkmain
#include "NKWindow/NKMain.h"
#include <cstdio>

using namespace nkentseu;

const float32 VITESSE_SAUT = 6.f;

struct Personnage {
		bool auSol = true;
		float32 vitesseY = 0.f;
};

// --- La version directe, telle qu'on l'ecrit d'habitude ----------------------

void SauterDirect(Personnage &perso) {
	if (perso.auSol && NkInput.IsKeyDown(NkKey::NK_SPACE)) {
		perso.vitesseY = VITESSE_SAUT;
		perso.auSol = false;
	}
}

// --- La version testable -----------------------------------------------------
// La regle ne lit plus rien de global : c'est l'appelant qui lit le clavier,
// une fois par image, et qui passe ce qu'il a lu :
//     Sauter(mJoueur, NkInput.IsKeyDown(NkKey::NK_SPACE));

bool DoitSauter(bool auSol, bool espaceEnfoncee) {
	return auSol && espaceEnfoncee;
}

void Sauter(Personnage &perso, bool espaceEnfoncee) {
	if (DoitSauter(perso.auSol, espaceEnfoncee)) {
		perso.vitesseY = VITESSE_SAUT;
		perso.auSol = false;
	}
}

// --- Le banc -----------------------------------------------------------------

struct CasDeSaut {
		const char *nom;
		bool auSol;
		bool espaceEnfoncee;
		bool doitSauter;
};

const CasDeSaut CAS[] = {
	{"au sol, espace enfoncee", true, true, true},
	{"au sol, espace relachee", true, false, false},
	{"en l'air, espace enfoncee", false, true, false},
	{"en l'air, espace relachee", false, false, false},
};

static bool ASaute(const Personnage &avant, const Personnage &apres) {
	return avant.auSol && !apres.auSol && apres.vitesseY == VITESSE_SAUT;
}

static int LancerBanc(FILE *journal) {
	int echecs = 0;

	fprintf(journal, "Version testable : Sauter(perso, espaceEnfoncee)\n");
	for (const CasDeSaut &cas : CAS) {
		Personnage perso;
		perso.auSol = cas.auSol;
		const Personnage avant = perso;
		Sauter(perso, cas.espaceEnfoncee);

		const bool saute = ASaute(avant, perso);
		const bool ok = saute == cas.doitSauter;
		if (!ok) {
			++echecs;
		}
		fprintf(journal, "  %-6s %-28s attendu %-11s obtenu %s\n", ok ? "OK" : "ECHEC", cas.nom,
				cas.doitSauter ? "saute" : "reste", saute ? "saute" : "reste");
	}

	// La version directe au meme endroit. On ne choisit que auSol : l'etat de
	// la barre d'espace, c'est NkInput qui le donne.
	fprintf(journal, "\nVersion directe : SauterDirect(perso), lue dans NkInput\n");
	fprintf(journal, "  NkInput voit : espace %s, derniere touche %s, souris %s en %d,%d\n",
			NkInput.IsKeyDown(NkKey::NK_SPACE) ? "enfoncee" : "relachee", NkKeyToString(NkInput.LastKey()),
			NkInput.IsAnyMouseDown() ? "enfoncee" : "relachee", NkInput.MouseX(), NkInput.MouseY());
	for (bool auSol : {true, false}) {
		Personnage perso;
		perso.auSol = auSol;
		const Personnage avant = perso;
		SauterDirect(perso);
		fprintf(journal, "  %-28s obtenu %s\n", auSol ? "au sol" : "en l'air", ASaute(avant, perso) ? "saute" : "reste");
	}
	fprintf(journal, "  (le cas \"au sol, espace enfoncee\" ne peut pas lui etre pose)\n");

	fprintf(journal, "\n%d echec(s) sur %d cas, code de sortie %d\n", echecs, (int)(sizeof(CAS) / sizeof(CAS[0])),
			echecs);
	return echecs;
}

class BancSaut : public renderer::NkCanvasApp {
	protected:
		NkOptional<int> OnCommandLine(const NkVector<NkString> &args) override {
			(void)args;
			FILE *journal = fopen("banc.log", "w");
			if (journal == nullptr) {
				// sans journal, aucun resultat lisible : on le dit par un code a part
				return NkOptional<int>(100);
			}
			const int echecs = LancerBanc(journal);
			fclose(journal);
			return NkOptional<int>(echecs);
		}
};

int nkmain(const NkEntryState &state) {
	return renderer::NkCanvasApp::Run<BancSaut>(state);
}
