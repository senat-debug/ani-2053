// fenetre.cpp — le plus petit programme NKCanvas : une fenetre, une couleur,
// et une fermeture propre. Tout le reste est tenu par la coquille.
#include "NKCanvas/App/NkCanvasApp.h"
#include "NKWindow/NKMain.h"

using namespace nkentseu;
using namespace nkentseu::renderer;

class Fenetre : public NkCanvasApp {
	public:
		// La configuration se pose ICI : dans OnInit, la fenetre existe deja.
		Fenetre() {
			Config().title = "La fenetre nue";
			Config().width = 960;
			Config().height = 540;
			Config().clearColor = NkColor2D{18, 18, 24, 255};
		}
};

int nkmain(const NkEntryState &state) {
	return NkCanvasApp::Run<Fenetre>(state);
}