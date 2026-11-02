
#include "NKCanvas/App/NkCanvasApp.h"
#include "NKEvent/NkKeyboardEvent.h"
// fournit WinMain et les autres points d'entree natifs qui appellent nkmain
#include "NKWindow/NKMain.h"
#include <cstdio>

using namespace nkentseu;

const char *const FICHIER_JOURNAL = "crochets.log";
const int32 IMAGES_NOTEES = 3;
const int32 IMAGE_DU_QUIT = 10;

class MonJeu : public renderer::NkCanvasApp {
	public:
		// OnCommandLine passe avant toute fenetre : le journal doit deja etre ouvert
		MonJeu() {
			mJournal = fopen(FICHIER_JOURNAL, "w");
			Config().title = "Les crochets du patron";
		}

		~MonJeu() override {
			if (mJournal != nullptr) {
				fclose(mJournal);
			}
		}

	protected:
		NkOptional<int> OnCommandLine(const NkVector<NkString> &args) override {
			char detail[32];
			snprintf(detail, sizeof(detail), "%u argument(s)", (unsigned)args.Size());
			Noter("OnCommandLine", detail);
			if (mJournal == nullptr) {
				// sans journal la mesure n'a pas lieu : inutile d'ouvrir la fenetre
				return NkOptional<int>(1);
			}
			return NkOptional<int>();
		}

		bool OnInit() override {
			Noter("OnInit");
			return true;
		}

		void OnUpdate(float32 deltaTime) override {
			++mImage;
			if (mImage >= IMAGE_DU_QUIT) {
				Quit();
			}
			char detail[32];
			snprintf(detail, sizeof(detail), "dt=%.4f", deltaTime);
			Noter("OnUpdate", detail, ImageNotee());
		}

		void OnRender(renderer::NkRenderWindow &target) override {
			(void)target;
			Noter("OnRender", "", ImageNotee());
		}

		void OnShutdown() override {
			Noter("OnShutdown");
		}

		bool OnEvent(const NkEvent &event) override {
			Noter("OnEvent", event.GetName(), ImageNotee());
			return false;
		}

		bool OnPointer(const NkPointer &pointer) override {
			char detail[48];
			snprintf(detail, sizeof(detail), "phase %d en %.0f,%.0f", (int)pointer.phase, pointer.x, pointer.y);
			Noter("OnPointer", detail);
			return false;
		}

		bool OnKeyPress(const NkKeyPressEvent &event) override {
			Noter("OnKeyPress", NkKeyToString(event.GetKey()));
			return false;
		}

		void OnLayout(const renderer::NkLayoutInfo &layout) override {
			char detail[48];
			snprintf(detail, sizeof(detail), "%ux%u densite %.2f", layout.width, layout.height, layout.density);
			Noter("OnLayout", detail);
		}

		bool OnCloseRequested() override {
			Noter("OnCloseRequested");
			return true;
		}

		void OnPause() override {
			Noter("OnPause");
		}

		void OnResume() override {
			Noter("OnResume");
		}

	private:
		// les premieres images, et celle ou Quit() a ete demande
		bool ImageNotee() const {
			return mImage <= IMAGES_NOTEES || mImage >= IMAGE_DU_QUIT;
		}

		void Noter(const char *crochet, const char *detail = "", bool ecrire = true) {
			++mAppels;
			if (!ecrire || mJournal == nullptr) {
				return;
			}
			fprintf(mJournal, "%4u  %-16s image %2d  Time()=%.4f  %s\n", mAppels, crochet, mImage, Time(), detail);
			// flush a chaque ligne : si le programme tombe, le journal dit ou
			fflush(mJournal);
		}

		FILE *mJournal = nullptr;
		uint32 mAppels = 0;
		int32 mImage = 0; ///< nombre d'OnUpdate deja commences
};

int nkmain(const NkEntryState &state) {
	return renderer::NkCanvasApp::Run<MonJeu>(state);
}
