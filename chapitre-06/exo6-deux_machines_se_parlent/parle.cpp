// Exercice 6 : deux machines se parlent
//   parle serveur
//   parle client <adresse-du-serveur>     (127.0.0.1 si on ne la donne pas)
#include "NKNetwork/NKNetwork.h"
#include "NKCore/NkAtomic.h"
#include "NKTime/NkChrono.h"
#include <cstdio>
#include <cstring>

using namespace nkentseu;

const uint16 PORT = 7777;
const uint32 MAX_CLIENTS = 8;
const int PAUSE_MS = 50;
// le client attend la connexion au plus 5 s, le serveur un message au plus 2 min
const int TOURS_CONNEXION = 100;
const int TOURS_SERVEUR = 2400;

// onPeerConnected est appele dans le fil reseau du moteur : on y range
// l'identifiant du pair, et la boucle principale le lit
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
	printf("Serveur a l'ecoute sur le port %u, identifiant local %llu\n", PORT,
		   (unsigned long long)reseau.GetLocalPeerId().value);

	NkVector<NkReceiveMsg> recus;
	bool messageRecu = false;
	for (int tour = 0; tour < TOURS_SERVEUR && !messageRecu; tour++) {
		recus.Clear();
		reseau.DrainAll(recus);
		for (const NkReceiveMsg &msg : recus) {
			printf("Message de %llu : %.*s\n", (unsigned long long)msg.from.value, (int)msg.size,
				   (const char *)msg.data);
			messageRecu = true;
		}
		NkChrono::SleepMilliseconds(PAUSE_MS);
	}
	if (!messageRecu) {
		printf("Aucun message recu\n");
	}

	// le temps que l'accuse de reception reparte vers le client
	NkChrono::SleepMilliseconds(500);
	reseau.Shutdown();
	return messageRecu ? 0 : 1;
}

static int Client(const char *adresse) {
	NkConnectionManager reseau;
	BrancherConnexion(reseau);

	NkNetResult r = reseau.Connect(NkAddress(adresse, PORT));
	if (r != NkNetResult::NK_NET_OK) {
		printf("Connexion impossible a %s : %s\n", adresse, net::NkNetResultStr(r));
		return 1;
	}
	printf("Client vers %s:%u, identifiant local %llu\n", adresse, PORT,
		   (unsigned long long)reseau.GetLocalPeerId().value);

	// la poignee de main se fait dans le fil du moteur : on attend qu'elle aboutisse
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
	const char *message = "Bonjour du client";
	r = reseau.SendTo(serveur, (const uint8 *)message, (uint32)strlen(message));
	if (r != NkNetResult::NK_NET_OK) {
		printf("Envoi refuse : %s\n", net::NkNetResultStr(r));
		reseau.Shutdown();
		return 1;
	}
	printf("Message envoye a %llu : %s\n", (unsigned long long)serveur.value, message);

	// laisser au canal fiable le temps d'etre acquitte avant de fermer
	NkChrono::SleepMilliseconds(1000);
	reseau.Shutdown();
	return 0;
}

int main(int argc, char **argv) {
	if (argc < 2 || (strcmp(argv[1], "serveur") != 0 && strcmp(argv[1], "client") != 0)) {
		printf("Usage : parle serveur | parle client <adresse-du-serveur>\n");
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
