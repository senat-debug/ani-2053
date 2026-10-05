// main.cpp — exercice 2 : ce que forment les sommets.
// Une liste de sommets ne dessine rien par elle-meme : c'est le type de
// primitive qui decide si elle fait des points, des segments ou des triangles.
// Les sommets en trop ne provoquent aucune erreur : ils sont ignores en
// silence, et la forme est incomplete sans que rien ne le dise.
#include <iostream>
#include <string>

int main() {
	long long n = 0;
	long long points = 0, segments = 0, triangles = 0, refuses = 0;

	if (std::cin >> n) {
		for (long long i = 0; i < n; ++i) {
			std::string type;
			long long s = 0;
			if (!(std::cin >> type >> s)) {
				break;
			}

			long long nombre = 0, restants = 0;
			const char *unite = nullptr; // nullptr = type refuse
			long long *bilan = nullptr;  // le total auquel ce type ajoute

			if (type == "POINTS") {
				nombre = s;
				restants = 0;
				unite = "POINTS";
				bilan = &points;
			} else if (type == "LINES") {
				nombre = s / 2;
				restants = s % 2;
				unite = "SEGMENTS";
				bilan = &segments;
			} else if (type == "LINE_STRIP") {
				// Une ligne brisee a besoin de deux sommets pour exister.
				nombre = (s >= 2) ? s - 1 : 0;
				restants = (s >= 2) ? 0 : s;
				unite = "SEGMENTS";
				bilan = &segments;
			} else if (type == "TRIANGLES") {
				nombre = s / 3;
				restants = s % 3;
				unite = "TRIANGLES";
				bilan = &triangles;
			} else if (type == "TRIANGLE_STRIP" || type == "TRIANGLE_FAN") {
				// Une bande ou un eventail de deux sommets ne forme aucun
				// triangle : on rend zero, jamais un nombre negatif.
				nombre = (s >= 3) ? s - 2 : 0;
				restants = (s >= 3) ? 0 : s;
				unite = "TRIANGLES";
				bilan = &triangles;
			}

			if (unite == nullptr) {
				// Tout autre type, QUADS compris : la ligne s'arrete la.
				std::cout << type << ' ' << s << " REFUSE\n";
				++refuses;
				continue;
			}

			std::cout << type << ' ' << s << ' ' << nombre << ' ' << unite << ' ' << restants << '\n';
			*bilan += nombre;
		}
	}

	std::cout << "POINTS " << points << '\n';
	std::cout << "SEGMENTS " << segments << '\n';
	std::cout << "TRIANGLES " << triangles << '\n';
	std::cout << "REFUSES " << refuses << '\n';
	return 0;
}