// Exercice 1 : la reserve
#include "NKThreading/NkThread.h"
#include "NKThreading/NkThreadPool.h"
#include "NKTime/NkChrono.h"
#include <cstdio>

using namespace nkentseu;
using namespace nkentseu::threading;

const int NB_TACHES = 200;
const int TAILLE_COURTE = 1000;
const int TAILLE_LONGUE = 100 * TAILLE_COURTE;
const int NB_MESURES = 3;

// rempli au demarrage : le compilateur ne connait pas les valeurs et doit vraiment additionner
static int entiers[TAILLE_LONGUE];

// chaque tache ecrit dans sa propre case, il n'y a donc rien a proteger
static long long resultats[NB_TACHES];

static void Tache(int numero, int taille) {
	long long somme = 0;
	for (int i = 0; i < taille; i++) {
		somme += entiers[i];
	}
	resultats[numero] = somme;
}

static long long Total() {
	long long total = 0;
	for (int i = 0; i < NB_TACHES; i++) {
		total += resultats[i];
	}
	return total;
}

static float64 MesurerReserve(NkThreadPool &reserve, int taille) {
	NkChrono chrono;
	for (int i = 0; i < NB_TACHES; i++) {
		reserve.Enqueue([i, taille]() { Tache(i, taille); });
	}
	reserve.Join();
	return chrono.Elapsed().milliseconds;
}

static float64 MesurerFils(int taille) {
	NkChrono chrono;
	NkThread fils[NB_TACHES];
	for (int i = 0; i < NB_TACHES; i++) {
		fils[i] = NkThread([i, taille](void *) { Tache(i, taille); });
	}
	for (int i = 0; i < NB_TACHES; i++) {
		fils[i].Join();
	}
	return chrono.Elapsed().milliseconds;
}

int main() {
	for (int i = 0; i < TAILLE_LONGUE; i++) {
		entiers[i] = i % 1000;
	}

	NkThreadPool reserve;
	// printf plutot que logger.Info : en Release la console du logger ne montre que les erreurs
	printf("Ouvriers de la reserve : %u\n", reserve.GetNumWorkers());

	for (int m = 1; m <= NB_MESURES; m++) {
		float64 ms = MesurerReserve(reserve, TAILLE_COURTE);
		printf("Reserve, mesure %d : %.3f ms (total %lld)\n", m, ms, Total());
	}
	for (int m = 1; m <= NB_MESURES; m++) {
		float64 ms = MesurerFils(TAILLE_COURTE);
		printf("Fils, mesure %d : %.3f ms (total %lld)\n", m, ms, Total());
	}

	float64 msReserve = MesurerReserve(reserve, TAILLE_LONGUE);
	printf("Tache longue, reserve : %.3f ms (total %lld)\n", msReserve, Total());
	float64 msFils = MesurerFils(TAILLE_LONGUE);
	printf("Tache longue, fils : %.3f ms (total %lld)\n", msFils, Total());

	return 0;
}
