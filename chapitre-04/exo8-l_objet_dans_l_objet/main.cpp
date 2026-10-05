// main.cpp — exercice 8 : l'objet dans l'objet.
// Une roue attachee a une voiture suit la voiture sans rien savoir d'elle :
// chaque objet COMPOSE la transformation de son parent avec la sienne. Tourner
// le convoi fait suivre la voiture et la roue sans toucher a leurs lignes.
// Attention : la rotation propre d'un objet ne deplace pas son origine, elle ne
// compte que pour ses enfants. C'est l'angle du PARENT qui tourne le point.
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// Ramene un angle dans 0, 90, 180, 270. En C++, -90 % 360 vaut -90.
static long long Normalise(long long a) {
	a %= 360;
	if (a < 0) {
		a += 360;
	}
	return a;
}

// Cosinus et sinus entiers des quatre angles droits.
static void CosSin(long long a, long long &c, long long &s) {
	c = 1;
	s = 0;
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
}

struct Objet {
		long long x = 0, y = 0;		  // position dans le monde
		long long angle = 0;		  // angle dans le monde, entre 0 et 270
		long long echelle = 1;		  // echelle dans le monde
		long long niveau = 1;		  // 1 pour une racine
};

int main() {
	long long n = 0;
	std::vector<Objet> objets;
	std::unordered_map<std::string, std::size_t> parNom;
	long long profondeur = 0;

	if (std::cin >> n) {
		for (long long i = 0; i < n; ++i) {
			std::string nom, parent;
			long long tx = 0, ty = 0, angle = 0, echelle = 0;
			if (!(std::cin >> nom >> parent >> tx >> ty >> angle >> echelle)) {
				break;
			}

			Objet o;
			const auto trouve = parNom.find(parent);

			if (parent == "-" || trouve == parNom.end()) {
				// Une racine est a sa place, telle quelle.
				o.x = tx;
				o.y = ty;
				o.angle = Normalise(angle);
				o.echelle = echelle;
				o.niveau = 1;
			} else {
				const Objet &p = objets[trouve->second];

				// L'echelle du parent DANS LE MONDE, puis sa rotation DANS LE
				// MONDE : les deux se composent sur toute la chaine.
				const long long ax = tx * p.echelle;
				const long long ay = ty * p.echelle;
				long long c = 1, s = 0;
				CosSin(p.angle, c, s);

				o.x = p.x + ax * c - ay * s;
				o.y = p.y + ax * s + ay * c;
				o.angle = Normalise(p.angle + angle);
				o.echelle = p.echelle * echelle;
				o.niveau = p.niveau + 1;
			}

			if (o.niveau > profondeur) {
				profondeur = o.niveau;
			}

			parNom[nom] = objets.size();
			objets.push_back(o);

			std::cout << nom << ' ' << o.x << ' ' << o.y << ' ' << o.angle << ' ' << o.echelle << '\n';
		}
	}

	std::cout << "PROFONDEUR " << profondeur << '\n';
	return 0;
}