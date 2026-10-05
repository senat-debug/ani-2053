// main.cpp — exercice 7 : la planche de sprites.
// Une animation reglee sur un compteur d'images va deux fois plus vite sur une
// machine deux fois plus rapide. Reglee sur le TEMPS, elle a la meme vitesse
// partout. Le plafond, lui, protege du retour de veille : l'horloge rend alors
// plusieurs secondes d'un coup, et l'animation sauterait des cases.
#include <iostream>

int main() {
	// R est lu, mais le calcul n'a besoin que de C : la ligne d'une case se
	// deduit de c / C, et la planche n'est jamais depassee puisqu'on boucle
	// sur F, qui vaut au plus C * R.
	long long C = 0, R = 0, W = 0, H = 0, F = 0, D = 0, P = 0;
	if (!(std::cin >> C >> R >> W >> H >> F >> D >> P)) {
		return 0;
	}

	long long n = 0;
	long long courante = 0;	  // la case affichee, elle commence a 0
	long long accumule = 0;	  // le temps mis de cote, garde d'une image a l'autre
	long long avances = 0, plafonnes = 0;

	if (std::cin >> n) {
		for (long long i = 0; i < n; ++i) {
			long long dt = 0;
			if (!(std::cin >> dt)) {
				break;
			}

			// 1. Le plafond vient AVANT l'accumulation. Un dt egal au plafond
			// n'est pas plafonne : la comparaison est stricte.
			if (dt > P) {
				dt = P;
				++plafonnes;
			}

			// 2. On ajoute, on ne remet jamais a zero : le reste d'une image
			// sert a la suivante, sinon l'animation ralentit.
			accumule += dt;

			// 3. Une image peut faire avancer de plusieurs cases d'un coup.
			const long long pas = accumule / D;
			accumule -= pas * D;
			if (pas > 0) {
				avances += pas;
				// On revient a 0 apres la case F - 1, pas apres C * R - 1 :
				// la planche peut porter plus de cases que l'animation.
				courante = (courante + pas) % F;
			}

			// 4. Les cases se lisent ligne par ligne, de gauche a droite.
			const long long x = (courante % C) * W;
			const long long y = (courante / C) * H;
			std::cout << courante << ' ' << x << ' ' << y << ' ' << W << ' ' << H << '\n';
		}
	}

	std::cout << "AVANCES " << avances << '\n';
	std::cout << "PLAFONNES " << plafonnes << '\n';
	return 0;
}