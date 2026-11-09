#include <iostream>
#include <map>
#include <string>
#include <vector>

struct Methode
{
    std::string nom;
    bool declaree = false;
    bool modifie = false;
    std::vector<int> appels;
};

enum Etat { INCONNU, OUI, NON };

// Le graphe des appels est sans cycle : la récursion se termine toujours.
bool PourraitEtreConst(const std::vector<Methode> &methodes, int i, std::vector<Etat> &etats)
{
    if (etats[i] == INCONNU)
    {
        bool possible = !methodes[i].modifie;
        for (int appelee : methodes[i].appels)
            if (possible && !PourraitEtreConst(methodes, appelee, etats))
                possible = false;
        etats[i] = possible ? OUI : NON;
    }
    return etats[i] == OUI;
}

bool AppeleesToutesConst(const Methode &methode, const std::vector<bool> &estConst)
{
    for (int appelee : methode.appels)
        if (!estConst[appelee])
            return false;
    return true;
}

void AfficherListe(const std::string &titre, const std::vector<Methode> &methodes, const std::vector<int> &indices)
{
    std::cout << titre;
    if (indices.empty())
        std::cout << " AUCUN";
    for (int i : indices)
        std::cout << ' ' << methodes[i].nom;
    std::cout << '\n';
}

int main()
{
    int n = 0;
    std::cin >> n;

    // Les appels peuvent viser une méthode lue plus loin : on garde les noms
    // et on ne les traduit en indices qu'une fois toute la classe lue.
    std::vector<Methode> methodes(n);
    std::vector<std::vector<std::string>> nomsAppeles(n);
    std::map<std::string, int> indiceDe;
    for (int i = 0; i < n; ++i)
    {
        std::string declaree;
        int modifie = 0;
        int k = 0;
        std::cin >> methodes[i].nom >> declaree >> modifie >> k;
        methodes[i].declaree = declaree == "const";
        methodes[i].modifie = modifie == 1;
        nomsAppeles[i].resize(k);
        for (std::string &nom : nomsAppeles[i])
            std::cin >> nom;
        indiceDe[methodes[i].nom] = i;
    }
    for (int i = 0; i < n; ++i)
        for (const std::string &nom : nomsAppeles[i])
            methodes[i].appels.push_back(indiceDe[nom]);

    std::vector<Etat> etats(n, INCONNU);
    std::vector<int> pourrait;
    for (int i = 0; i < n; ++i)
        if (!methodes[i].declaree && PourraitEtreConst(methodes, i, etats))
            pourrait.push_back(i);

    std::vector<bool> estConst(n);
    for (int i = 0; i < n; ++i)
        estConst[i] = methodes[i].declaree;

    // Chaque passe ne voit que l'état laissé par la précédente : on choisit
    // d'abord toute la passe, puis on la déclare d'un coup.
    std::vector<int> racines;
    int passes = 0;
    std::vector<int> restantes = pourrait;
    while (!restantes.empty())
    {
        std::vector<int> corrigees;
        std::vector<int> suivantes;
        for (int i : restantes)
        {
            if (AppeleesToutesConst(methodes[i], estConst))
                corrigees.push_back(i);
            else
                suivantes.push_back(i);
        }
        for (int i : corrigees)
            estConst[i] = true;
        if (passes == 0)
            racines = corrigees;
        ++passes;
        restantes = suivantes;
    }

    AfficherListe("POURRAIT", methodes, pourrait);
    AfficherListe("RACINES", methodes, racines);
    std::cout << "PASSES " << passes << '\n';
    return 0;
}
