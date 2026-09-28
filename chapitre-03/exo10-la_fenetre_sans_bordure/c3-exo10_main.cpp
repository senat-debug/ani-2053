// BarreATitre — exercice 10 du chapitre 3.
// Fenetre SANS bordure (frame = false) avec une barre de titre faite a la main :
// le titre, trois boutons (reduire, agrandir/restaurer, fermer), le deplacement
// a la souris et le double-clic qui agrandit.
//
// Dessin par NKCanvas (NkRenderWindow), deplacement et redimensionnement par le
// hand-off natif : BeginDragMove() et BeginResize() (NkWindow.h:153 et 160).

#include "NKWindow/NKMain.h"
#include "NKWindow/NKWindow.h"
#include "NKEvent/NkMouseEvent.h"

#include "NKCanvas/Renderer/Targets/NkRenderWindow.h"
#include "NKCanvas/Core/NkContextDesc.h"
#include "NKCanvas/Core/NkGraphicsApi.h"
#include "NKCanvas/Renderer/Shapes/NkRectangleShape.h"
#include "NKCanvas/Renderer/Resources/NkFont.h"
#include "NKCanvas/Renderer/Resources/NkSprite.h" // NkText

#include <cstdio>

using namespace nkentseu;
using namespace nkentseu::renderer;

static const float kBarre = 36.f;  // hauteur de ma barre de titre
static const float kBouton = 46.f; // largeur d'un bouton
static const float kBord = 6.f;    // epaisseur des bords sensibles au redimensionnement

// Quel bouton est sous (x,y) ? 0 = reduire, 1 = agrandir, 2 = fermer, -1 = aucun.
static int BoutonSous(float x, float y, float largeur) {
	if (y < 0.f || y >= kBarre) return -1;
	const float droite = largeur;
	if (x >= droite - kBouton * 1.f) return 2;
	if (x >= droite - kBouton * 2.f) return 1;
	if (x >= droite - kBouton * 3.f) return 0;
	return -1;
}

// Quel bord est sous (x,y) ? Rend true et remplit `edge` si on est sur un bord.
static bool BordSous(float x, float y, float w, float h, NkWindow::NkResizeEdge &edge) {
	const bool g = x < kBord, d = x > w - kBord, ht = y < kBord, b = y > h - kBord;
	if (ht && g) { edge = NkWindow::NkResizeEdge::TopLeft; return true; }
	if (ht && d) { edge = NkWindow::NkResizeEdge::TopRight; return true; }
	if (b && g)  { edge = NkWindow::NkResizeEdge::BottomLeft; return true; }
	if (b && d)  { edge = NkWindow::NkResizeEdge::BottomRight; return true; }
	if (g)  { edge = NkWindow::NkResizeEdge::Left; return true; }
	if (d)  { edge = NkWindow::NkResizeEdge::Right; return true; }
	if (ht) { edge = NkWindow::NkResizeEdge::Top; return true; }
	if (b)  { edge = NkWindow::NkResizeEdge::Bottom; return true; }
	return false;
}

int nkmain(const NkEntryState &state) {
	NkWindowConfig cfg;
	cfg.title = "Ma barre a moi";
	cfg.width = 900;
	cfg.height = 520;
	cfg.centered = true;
	cfg.frame = false; // <- la fenetre nue

	NkWindow window(cfg);
	if (!window.IsOpen()) return -1;

	NkContextDesc desc;
	desc.api = NkGraphicsApi::NK_GFX_API_OPENGL;
	NkRenderWindow target(window, desc);
	if (!target.IsValid()) return -2;

	renderer::NkFont police;
	const bool aPolice = police.LoadFromFile(*target.GetRenderer(), "Resources/Fonts/Antonio-Bold.ttf");

	int survole = -1;      // bouton survole
	float sourisX = 0.f, sourisY = 0.f;
	unsigned long clicsBarre = 0, doublesClics = 0, deplacements = 0, redimensions = 0;

	while (window.IsOpen()) {
		while (NkEvent *ev = NkEvents().PollEvent()) {
			if (ev->Is<NkWindowCloseEvent>()) {
				window.Close();
			} else if (auto *m = ev->As<NkMouseMoveEvent>()) {
				sourisX = (float)m->GetX();
				sourisY = (float)m->GetY();
				survole = BoutonSous(sourisX, sourisY, (float)target.GetSize().x);
				// Le curseur suit les bords : c'est l'exercice 6 qui sert ici.
				NkWindow::NkResizeEdge e;
				if (BordSous(sourisX, sourisY, (float)target.GetSize().x, (float)target.GetSize().y, e)) {
					switch (e) {
						case NkWindow::NkResizeEdge::Left:
						case NkWindow::NkResizeEdge::Right:
							window.SetCursor(NkWindow::NkCursorType::ResizeWE); break;
						case NkWindow::NkResizeEdge::Top:
						case NkWindow::NkResizeEdge::Bottom:
							window.SetCursor(NkWindow::NkCursorType::ResizeNS); break;
						case NkWindow::NkResizeEdge::TopLeft:
						case NkWindow::NkResizeEdge::BottomRight:
							window.SetCursor(NkWindow::NkCursorType::ResizeNWSE); break;
						default:
							window.SetCursor(NkWindow::NkCursorType::ResizeNESW); break;
					}
				} else {
					window.SetCursor(survole >= 0 ? NkWindow::NkCursorType::Hand
												  : NkWindow::NkCursorType::Arrow);
				}
			} else if (auto *b = ev->As<NkMouseButtonPressEvent>()) {
				if (b->GetButton() != NkMouseButton::NK_MB_LEFT) continue;
				const float x = (float)b->GetX(), y = (float)b->GetY();
				const float w = (float)target.GetSize().x, h = (float)target.GetSize().y;

				NkWindow::NkResizeEdge e;
				const int bouton = BoutonSous(x, y, w);
				if (bouton == 0) {
					window.Minimize();
				} else if (bouton == 1) {
					if (window.IsMaximized()) window.Restore(); else window.Maximize();
				} else if (bouton == 2) {
					window.Close();
				} else if (BordSous(x, y, w, h, e)) {
					window.BeginResize(e);   // le systeme prend la main
					++redimensions;
				} else if (y < kBarre) {
					++clicsBarre;
					window.BeginDragMove(); // le systeme deplace la fenetre
					++deplacements;
				}
			} else if (auto *d = ev->As<NkMouseDoubleClickEvent>()) {
				if (d->GetButton() == NkMouseButton::NK_MB_LEFT && (float)d->GetY() < kBarre) {
					++doublesClics;
					if (window.IsMaximized()) window.Restore(); else window.Maximize();
				}
			}
		}

		const float w = (float)target.GetSize().x;
		target.Clear(NkColor2D{28, 30, 36, 255});

		// La barre
		NkRectangleShape barre({w, kBarre});
		barre.SetPosition({0.f, 0.f});
		barre.SetFillColor(NkColor2D{44, 48, 58, 255});
		target.Draw(barre);

		// Les trois boutons
		for (int i = 0; i < 3; ++i) {
			const float bx = w - kBouton * (3 - i);
			NkRectangleShape bt({kBouton, kBarre});
			bt.SetPosition({bx, 0.f});
			const bool actif = (survole == i);
			if (i == 2)
				bt.SetFillColor(actif ? NkColor2D{200, 60, 60, 255} : NkColor2D{44, 48, 58, 255});
			else
				bt.SetFillColor(actif ? NkColor2D{70, 76, 90, 255} : NkColor2D{44, 48, 58, 255});
			target.Draw(bt);

			if (aPolice) {
				const char *glyphe = (i == 0) ? "_" : (i == 1 ? (window.IsMaximized() ? "o" : "[]") : "X");
				NkText g(police, glyphe, 16u);
				g.SetFillColor(NkColor2D::White);
				g.SetPosition({bx + kBouton * 0.5f - 6.f, 8.f});
				target.Draw(g);
			}
		}

		// Le titre
		if (aPolice) {
			NkText t(police, "Ma barre a moi - double-clic pour agrandir", 18u);
			t.SetFillColor(NkColor2D{230, 232, 238, 255});
			t.SetPosition({12.f, 7.f});
			target.Draw(t);
		}

		target.Display();
	}

	FILE *f = std::fopen("barre.txt", "w");
	std::fprintf(f, "police chargee        : %s\n", aPolice ? "oui" : "non");
	std::fprintf(f, "clics dans la barre   : %lu\n", clicsBarre);
	std::fprintf(f, "BeginDragMove appele  : %lu fois\n", deplacements);
	std::fprintf(f, "BeginResize appele    : %lu fois\n", redimensions);
	std::fprintf(f, "doubles-clics barre   : %lu\n", doublesClics);
	std::fclose(f);
	return 0;
}