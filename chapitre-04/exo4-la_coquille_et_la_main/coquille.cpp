// coquille.cpp — le meme carre rouge, ecrit AVEC la coquille NkCanvasApp.
// Je ne tiens ni la fenetre, ni la boucle, ni le Clear, ni le Display : je
// remplis trois cases. Le mouvement passe par dt, donc la vitesse ne depend
// pas de la machine.
#include "NKCanvas/App/NkCanvasApp.h"
#include "NKCanvas/Renderer/Shapes/NkRectangleShape.h"
#include "NKWindow/NKMain.h"

using namespace nkentseu;
using namespace nkentseu::renderer;

class Carre : public NkCanvasApp {
	public:
		// La configuration se pose ICI : dans OnInit, la fenetre existe deja.
		Carre() {
			Config().title = "Coquille";
			Config().width = 900;
			Config().height = 300;
			Config().clearColor = NkColor2D{18, 18, 24, 255};
		}

	protected:
		void OnUpdate(float32 dt) override {
			mX += 100.f * dt; // cent pixels par seconde
			if (mX > 900.f) {
				mX = -60.f;
			}
		}

		void OnRender(NkRenderWindow &target) override {
			NkRectangleShape carre({60.f, 60.f});
			carre.SetPosition({mX, 120.f});
			carre.SetFillColor(NkColor2D{200, 60, 60, 255});
			target.Draw(carre);
		}

	private:
		float32 mX = 0.f;
};

int nkmain(const NkEntryState &state) {
	return NkCanvasApp::Run<Carre>(state);
}