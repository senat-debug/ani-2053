
#include "NKNetwork/NKNetwork.h"
#include "NKCore/NkAtomic.h"
#include "NKTime/NkChrono.h"
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace nkentseu;

const uint16 PORT = 7777;
const uint32 MAX_CLIENTS = 8;
const int PAUSE_MS = 50;
const int TOURS_CONNEXION = 100;
const int TOURS_SERVEUR = 2400;

// bornes et precisions communes a l'ecriture et a la relecture
const float32 POS_MIN = -500.f;
const float32 POS_MAX = 500.f;
const float32 POS_PRECISION = 0.01f; // le centimetre
const float32 ANGLE_MIN = 0.f;
const float32 ANGLE_MAX = 360.f;
const float32 ANGLE_PRECISION = 0.5f; // le demi-degre
const int32 SANTE_MIN = 0;
const int32 SANTE_MAX = 100;

const uint32 TAILLE_TAMPON = 64;

struct EtatJoueur {
	bool enVie;
	uint8 numero;
	float32 x, y;
	float32 angle;
	int32 sante;
};

static void EcrireComplet(NkBitWriter &w, const EtatJoueur &e) {
	w.WriteU8(e.enVie ? 1 : 0);
	w.WriteU8(e.numero);
	w.WriteF32(e.x);
	w.WriteF32(e.y);
	w.WriteF32(e.angle);
	w.WriteI32(e.sante);
}

static void EcrireQuantifie(NkBitWriter &w, const EtatJoueur &e) {
	w.WriteBool(e.enVie);
	w.WriteU8(e.numero);
	w.WriteF32Q(e.x, POS_MIN, POS_MAX, POS_PRECISION);
	w.WriteF32Q(e.y, POS_MIN, POS_MAX, POS_PRECISION);
	w.WriteF32Q(e.angle, ANGLE_MIN, ANGLE_MAX, ANGLE_PRECISION);
	w.WriteInt(e.sante, SANTE_MIN, SANTE_MAX);
}

// meme ordre que l'ecriture, champ par champ
static EtatJoueur LireComplet(NkBitReader &r) {
	EtatJoueur e;
	e.enVie = r.ReadU8() != 0;
	e.numero = r.ReadU8();
	e.x = r.ReadF32();
	e.y = r.ReadF32();
	e.angle = r.ReadF32();
	e.sante = r.ReadI32();
	return e;
}

static EtatJoueur LireQuantifie(NkBitReader &r) {
	EtatJoueur e;
	e.enVie = r.ReadBool();
	e.numero = r.ReadU8();
	e.x = r.ReadF32Q(POS_MIN, POS_MAX, POS_PRECISION);
	e.y = r.ReadF32Q(POS_MIN, POS_MAX, POS_PRECISION);
	e.angle = r.ReadF32Q(ANGLE_MIN, ANGLE_MAX, ANGLE_PRECISION);
	e.sante = r.ReadInt(SANTE_MIN, SANTE_MAX);
	return e;
}

static void Afficher(const char *titre, const EtatJoueur &e) {
	printf("%s : en vie %d, numero %u, x %.4f m, y %.4f m, angle %.4f deg, sante %d\n", titre,
		   e.enVie ? 1 : 0, e.numero, e.x, e.y, e.angle, e.sante);
}

static NkAtomicUInt64 pairConnecte{0};

static void BrancherConnexion(NkConnectionManager &reseau) {
	reseau.onPeerConnected = [](NkPeerId pair) {
		printf("Pair connecte : %llu\n", (unsigned long long)pair.value);
		pairConnecte.Store(pair.value);
	};
}

static int Serveur() {
	NkConnectionManager reseau;
	BrancherConnexion(reseau);

	NkNetResult r = reseau.StartServer(PORT, MAX_CLIENTS);
	if (r != NkNetResult::NK_NET_OK) {
		printf("Le serveur ne demarre pas : %s\n", net::NkNetResultStr(r));
		return 1;
	}
	printf("Serveur a l'ecoute sur le port %u\n", PORT);

	// le canal fiable ordonne de SendTo livre le paquet complet avant le quantifie
	EtatJoueur complet{};
	EtatJoueur quantifie{};
	int paquetsRecus = 0;
	bool relectureSaine = true;
	NkVector<NkReceiveMsg> recus;
	for (int tour = 0; tour < TOURS_SERVEUR && paquetsRecus < 2; tour++) {
		recus.Clear();
		reseau.DrainAll(recus);
		for (const NkReceiveMsg &msg : recus) {
			if (paquetsRecus >= 2) {
				break;
			}
			NkBitReader lecteur(msg.data, msg.size);
			if (paquetsRecus == 0) {
				complet = LireComplet(lecteur);
				printf("Paquet complet recu : %u octets\n", msg.size);
				Afficher("Relu complet  ", complet);
			} else {
				quantifie = LireQuantifie(lecteur);
				printf("Paquet quantifie recu : %u octets\n", msg.size);
				Afficher("Relu quantifie", quantifie);
			}
			if (lecteur.IsOverflowed()) {
				printf("Paquet tronque : la relecture a depasse les %u octets recus\n", msg.size);
				relectureSaine = false;
			}
			paquetsRecus++;
		}
		NkChrono::SleepMilliseconds(PAUSE_MS);
	}

	if (paquetsRecus < 2) {
		printf("Paquets recus : %d sur 2\n", paquetsRecus);
	} else if (relectureSaine) {
		// le paquet complet rend les valeurs envoyees au bit pres : il sert de reference
		printf("Ecart en vie  : %d\n", complet.enVie != quantifie.enVie ? 1 : 0);
		printf("Ecart numero  : %d\n", abs((int)complet.numero - (int)quantifie.numero));
		printf("Ecart x       : %.4f m\n", fabsf(complet.x - quantifie.x));
		printf("Ecart y       : %.4f m\n", fabsf(complet.y - quantifie.y));
		printf("Ecart angle   : %.4f deg\n", fabsf(complet.angle - quantifie.angle));
		printf("Ecart sante   : %d\n", abs(complet.sante - quantifie.sante));
	}

	NkChrono::SleepMilliseconds(500);
	reseau.Shutdown();
	return paquetsRecus == 2 && relectureSaine ? 0 : 1;
}

static bool Envoyer(NkConnectionManager &reseau, NkPeerId serveur, const char *nom,
					const NkBitWriter &w, const uint8 *tampon) {
	printf("Paquet %s : %u octets, %u bits\n", nom, w.BytesWritten(), w.BitsWritten());
	if (w.IsOverflowed()) {
		printf("Paquet %s deborde du tampon de %u octets\n", nom, TAILLE_TAMPON);
		return false;
	}
	NkNetResult r = reseau.SendTo(serveur, tampon, w.BytesWritten());
	if (r != NkNetResult::NK_NET_OK) {
		printf("Envoi du paquet %s refuse : %s\n", nom, net::NkNetResultStr(r));
		return false;
	}
	return true;
}

static int Client(const char *adresse) {
	const EtatJoueur joueur = {true, 42, 123.4567f, -87.6543f, 271.3f, 73};
	Afficher("Envoye        ", joueur);

	uint8 tamponComplet[TAILLE_TAMPON] = {};
	NkBitWriter complet(tamponComplet, TAILLE_TAMPON);
	EcrireComplet(complet, joueur);

	uint8 tamponQuantifie[TAILLE_TAMPON] = {};
	NkBitWriter quantifie(tamponQuantifie, TAILLE_TAMPON);
	EcrireQuantifie(quantifie, joueur);

	NkConnectionManager reseau;
	BrancherConnexion(reseau);

	NkNetResult r = reseau.Connect(NkAddress(adresse, PORT));
	if (r != NkNetResult::NK_NET_OK) {
		printf("Connexion impossible a %s : %s\n", adresse, net::NkNetResultStr(r));
		return 1;
	}

	for (int tour = 0; tour < TOURS_CONNEXION && pairConnecte.Load() == 0; tour++) {
		NkChrono::SleepMilliseconds(PAUSE_MS);
	}
	if (pairConnecte.Load() == 0) {
		printf("Pas de reponse du serveur %s\n", adresse);
		reseau.Shutdown();
		return 1;
	}

	NkPeerId serveur;
	serveur.value = pairConnecte.Load();
	bool envoye = Envoyer(reseau, serveur, "complet", complet, tamponComplet) &&
				  Envoyer(reseau, serveur, "quantifie", quantifie, tamponQuantifie);

	// laisser au canal fiable le temps d'etre acquitte avant de fermer
	NkChrono::SleepMilliseconds(1000);
	reseau.Shutdown();
	return envoye ? 0 : 1;
}

int main(int argc, char **argv) {
	if (argc < 2 || (strcmp(argv[1], "serveur") != 0 && strcmp(argv[1], "client") != 0)) {
		printf("Usage : octets serveur | octets client <adresse-du-serveur>\n");
		return 1;
	}

	if (NkSocket::PlatformInit() != NkNetResult::NK_NET_OK) {
		printf("Le reseau ne s'initialise pas\n");
		return 1;
	}

	int code;
	if (strcmp(argv[1], "serveur") == 0) {
		code = Serveur();
	} else {
		code = Client(argc > 2 ? argv[2] : "127.0.0.1");
	}

	NkSocket::PlatformShutdown();
	return code;
}
