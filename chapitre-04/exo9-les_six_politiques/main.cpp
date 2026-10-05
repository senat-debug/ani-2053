// main.cpp — exercice 9 : les six politiques de redimensionnement.
// Quand la fenetre change de taille, le moteur doit choisir, et ce choix change
// LE JEU, pas seulement l'affichage : suivre la fenetre donne au joueur au
// grand ecran un champ de vision plus large, etirer est equitable et laid, les
// bandes noires sont equitables et honnetes.
// FIT_LETTERBOX retrecit le VIEWPORT, FIT_CROP retrecit le MONDE : ce ne sont
// pas les memes colonnes qui changent.
#include <iostream>

// Arrondi a l'entier le plus proche, une moitie monte. a >= 0 et b > 0.
static long long Arrondi(long long a, long long b) {
	return (2 * a + b) / (2 * b);
}

struct Politique {
		const char *nom = "";
		long long vx = 0, vy = 0, vw = 0, vh = 0; // le viewport
		long long mw = 0, mh = 0;				  // le monde visible dedans
};

int main() {
	long long RW = 0, RH = 0, AW = 0, AH = 0, W = 0, H = 0;
	if (!(std::cin >> RW >> RH >> AW >> AH >> W >> H)) {
		return 0;
	}

	// Sans reference, il n'y a rien a ajuster : quatre politiques sur six font
	// alors comme FOLLOW_WINDOW.
	const bool reference = (RW > 0 && RH > 0);

	Politique p[6];
	p[0].nom = "FOLLOW_WINDOW";
	p[1].nom = "STRETCH";
	p[2].nom = "FIT_LETTERBOX";
	p[3].nom = "INTEGER_SCALE";
	p[4].nom = "FIT_CROP";
	p[5].nom = "MANUAL";

	// 1. FOLLOW_WINDOW : un pixel reste un pixel, on voit plus de monde.
	p[0].vw = W;
	p[0].vh = H;
	p[0].mw = W;
	p[0].mh = H;

	// 6. MANUAL : le moteur ne touche a rien.
	p[5].vw = AW;
	p[5].vh = AH;
	p[5].mw = AW;
	p[5].mh = AH;

	if (!reference) {
		for (int i = 1; i <= 4; ++i) {
			p[i] = p[0];
		}
		p[1].nom = "STRETCH";
		p[2].nom = "FIT_LETTERBOX";
		p[3].nom = "INTEGER_SCALE";
		p[4].nom = "FIT_CROP";
	} else {
		// 2. STRETCH : la scene remplit tout, quitte a se deformer.
		p[1].vw = W;
		p[1].vh = H;
		p[1].mw = RW;
		p[1].mh = RH;

		// 3. FIT_LETTERBOX : rapport conserve, des bandes sur deux cotes. On
		// compare les rapports EN MULTIPLIANT, jamais en divisant.
		long long lvw = 0, lvh = 0;
		if (W * RH <= H * RW) {
			lvw = W;					  // la largeur limite
			lvh = Arrondi(RH * W, RW);
		} else {
			lvh = H;					  // la hauteur limite
			lvw = Arrondi(RW * H, RH);
		}
		p[2].vw = lvw;
		p[2].vh = lvh;
		p[2].vx = (W - lvw) / 2;
		p[2].vy = (H - lvh) / 2;
		p[2].mw = RW;
		p[2].mh = RH;

		// 4. INTEGER_SCALE : comme letterbox, mais l'agrandissement est un
		// entier. S'il serait nul, on retombe exactement sur letterbox.
		if (W >= RW && H >= RH) {
			const long long kx = W / RW;
			const long long ky = H / RH;
			const long long k = (kx < ky) ? kx : ky;
			p[3].vw = RW * k;
			p[3].vh = RH * k;
			p[3].vx = (W - p[3].vw) / 2;
			p[3].vy = (H - p[3].vh) / 2;
		} else {
			p[3].vx = p[2].vx;
			p[3].vy = p[2].vy;
			p[3].vw = p[2].vw;
			p[3].vh = p[2].vh;
		}
		p[3].mw = RW;
		p[3].mh = RH;

		// 5. FIT_CROP : le viewport prend toute la fenetre, c'est le MONDE qui
		// retrecit, et les bords sont perdus.
		p[4].vw = W;
		p[4].vh = H;
		if (W * RH > H * RW) {
			p[4].mw = RW;				  // fenetre plus large : on perd en haut et en bas
			p[4].mh = Arrondi(RW * H, W);
		} else {
			p[4].mw = Arrondi(RH * W, H); // on perd a gauche et a droite
			p[4].mh = RH;
		}
	}

	long long bandes = 0;
	for (int i = 0; i < 6; ++i) {
		// Un viewport plus GRAND que la fenetre n'a pas de bandes : seul plus
		// etroit ou plus bas compte, et la politique ne compte qu'une fois.
		if (p[i].vw < W || p[i].vh < H) {
			++bandes;
		}
		std::cout << p[i].nom << ' ' << p[i].vx << ' ' << p[i].vy << ' ' << p[i].vw << ' '
				  << p[i].vh << ' ' << p[i].mw << ' ' << p[i].mh << '\n';
	}

	const bool deformation = reference && (W * RH != H * RW);
	std::cout << "BANDES " << bandes << '\n';
	std::cout << "DEFORMATION " << (deformation ? "OUI" : "NON") << '\n';
	return 0;
}