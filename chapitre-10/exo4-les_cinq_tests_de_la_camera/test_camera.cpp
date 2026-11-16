#include "NkRef/NkRefView.h"

#include <cmath>
#include <cstdio>

using nkref::float32;
using nkref::NkRefView;
using nkref::NkVec2f;

namespace {

	const NkVec2f kViewport{800.0f, 600.0f};

	bool Proche(float32 a, float32 b, float32 tolerance) {
		return std::fabs(a - b) <= tolerance;
	}

	bool Proche(const NkVec2f &a, const NkVec2f &b, float32 tolerance) {
		return Proche(a.x, b.x, tolerance) && Proche(a.y, b.y, tolerance);
	}

	// Une vue déjà zoomée et déplacée : avec la vue d'origine, centre nul et
	// zoom 1, une erreur sur le centre ou sur le zoom passerait inaperçue.
	NkRefView VueDeplacee() {
		NkRefView vue;
		vue.ZoomAtPixel(3.5f, {620.0f, 140.0f}, kViewport);
		vue.PanByPixels(-75.0f, 40.0f);
		return vue;
	}

	bool aller_retour() {
		const NkRefView vues[] = {NkRefView{}, VueDeplacee()};
		const NkVec2f pixels[] = {{0.0f, 0.0f}, {400.0f, 300.0f}, {123.0f, 456.0f}, {799.0f, 1.0f}};
		for (const NkRefView &vue : vues) {
			for (const NkVec2f &p : pixels) {
				const NkVec2f retour = vue.WorldToPixel(vue.PixelToWorld(p, kViewport), kViewport);
				if (!Proche(retour, p, 0.001f))
					return false;
			}
		}
		return true;
	}

	bool point_fixe() {
		// Le facteur 1000 se fait borner par kZoomMax : le point doit tenir quand même.
		const float32 facteurs[] = {2.0f, 0.5f, 1.25f, 1000.0f};
		const NkVec2f pixels[] = {{200.0f, 150.0f}, {610.0f, 520.0f}, {400.0f, 300.0f}};
		for (float32 f : facteurs) {
			for (const NkVec2f &p : pixels) {
				NkRefView vue = VueDeplacee();
				const NkVec2f avant = vue.PixelToWorld(p, kViewport);
				vue.ZoomAtPixel(f, p, kViewport);
				if (!Proche(vue.PixelToWorld(p, kViewport), avant, 0.001f))
					return false;
			}
		}
		return true;
	}

	bool bornes() {
		NkRefView vue;
		vue.ZoomAtPixel(1000.0f, {250.0f, 100.0f}, kViewport);
		if (vue.zoom < NkRefView::kZoomMin || vue.zoom > NkRefView::kZoomMax)
			return false;
		vue.ZoomAtPixel(0.00001f, {250.0f, 100.0f}, kViewport);
		return vue.zoom >= NkRefView::kZoomMin && vue.zoom <= NkRefView::kZoomMax;
	}

	bool grille() {
		const float32 minimums[] = {8.0f, 24.0f, 100.0f};
		for (float32 m : minimums) {
			NkRefView vue;
			vue.ZoomAtPixel(1000.0f, {400.0f, 300.0f}, kViewport);
			float32 precedent = vue.GridSpacing(m);
			// On descend par petits pas jusqu'à buter sur kZoomMin.
			while (vue.zoom > NkRefView::kZoomMin) {
				vue.ZoomAtPixel(0.9f, {400.0f, 300.0f}, kViewport);
				const float32 pas = vue.GridSpacing(m);
				if (pas < precedent)
					return false;
				precedent = pas;
			}
		}
		return true;
	}

	bool origine() {
		NkRefView vue;
		vue.ZoomAtPixel(4.0f, {120.0f, 80.0f}, kViewport);
		vue.PanByPixels(250.0f, -130.0f);
		vue.Reset();
		return Proche(vue.center, NkVec2f{0.0f, 0.0f}, 1e-6f) && Proche(vue.zoom, 1.0f, 1e-6f);
	}

	void Affiche(const char *nom, bool reussi) {
		std::printf("TEST %s : %s\n", nom, reussi ? "OK" : "ECHEC");
	}

} // namespace

int main() {
	Affiche("aller_retour", aller_retour());
	Affiche("point_fixe", point_fixe());
	Affiche("bornes", bornes());
	Affiche("grille", grille());
	Affiche("origine", origine());
	return 0;
}
