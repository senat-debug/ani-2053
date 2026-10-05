// main.cpp — exercice 3 : le pivot.
// L'origine est le point autour duquel l'objet tourne. Laissee au coin, elle
// fait tourner le rectangle AUTOUR DE CE COIN, et il part la ou on ne l'attend
// pas. Posee au centre, le rectangle tourne sur lui-meme.
// L'axe y descend vers le bas de l'ecran : un angle positif tourne donc dans
// le sens des aiguilles d'une montre.
#include <iostream>
#include <string>

int main() {
	long long n = 0;
	long long refuses = 0;

	if (std::cin >> n) {
		for (long long i = 0; i < n; ++i) {
			std::string nom;
			long long w = 0, h = 0, px = 0, py = 0, ox = 0, oy = 0, sx = 0, sy = 0, angle = 0;
			if (!(std::cin >> nom >> w >> h >> px >> py >> ox >> oy >> sx >> sy >> angle)) {
				break;
			}

			// En C++, -90 % 360 vaut -90 : on ramene le reste dans les positifs
			// AVANT de choisir c et s.
			long long a = angle % 360;
			if (a < 0) {
				a += 360;
			}

			if (a % 90 != 0) {
				std::cout << nom << " ANGLE REFUSE\n";
				++refuses;
				continue;
			}

			long long c = 1, s = 0;
			if (a == 90) {
				c = 0;
				s = 1;
			} else if (a == 180) {
				c = -1;
				s = 0;
			} else if (a == 270) {
				c = 0;
				s = -1;
			}

			// Haut-gauche, haut-droit, bas-droit, bas-gauche.
			const long long locaux[4][2] = {{0, 0}, {w, 0}, {w, h}, {0, h}};
			long long xs[4] = {0, 0, 0, 0};
			long long ys[4] = {0, 0, 0, 0};

			for (int k = 0; k < 4; ++k) {
				// On retire l'origine, PUIS on applique l'echelle. Une echelle
				// negative retourne l'objet comme dans un miroir : la formule
				// s'applique telle quelle, sans cas particulier.
				const long long ax = (locaux[k][0] - ox) * sx;
				const long long ay = (locaux[k][1] - oy) * sy;
				xs[k] = px + ax * c - ay * s;
				ys[k] = py + ax * s + ay * c;
			}

			std::cout << nom << " COINS";
			for (int k = 0; k < 4; ++k) {
				std::cout << ' ' << xs[k] << ' ' << ys[k];
			}
			std::cout << '\n';

			// La boite se calcule APRES la rotation, sur les quatre coins, et
			// jamais a partir de w et h.
			long long minx = xs[0], maxx = xs[0], miny = ys[0], maxy = ys[0];
			for (int k = 1; k < 4; ++k) {
				if (xs[k] < minx) minx = xs[k];
				if (xs[k] > maxx) maxx = xs[k];
				if (ys[k] < miny) miny = ys[k];
				if (ys[k] > maxy) maxy = ys[k];
			}
			std::cout << nom << " BOITE " << minx << ' ' << miny << ' ' << maxx << ' ' << maxy << '\n';
		}
	}

	std::cout << "REFUSES " << refuses << '\n';
	return 0;
}