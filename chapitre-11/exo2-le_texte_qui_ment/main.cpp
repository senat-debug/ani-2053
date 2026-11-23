#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

int main()
{
    int h = 0;
    std::cin >> h;

    std::vector<std::string> hashes(h > 0 ? h : 0);
    std::map<std::string, int> indiceDuCommit;
    std::set<std::string> presentes;

    // Pour chaque fonction, l'indice du dernier commit qui a réellement
    // changé sa présence. Entre deux changements, son état ne bouge pas.
    std::map<std::string, int> dernierChangement;

    for (int i = 0; i < h; ++i)
    {
        std::string action;
        int k = 0;
        std::cin >> hashes[i] >> action >> k;
        indiceDuCommit[hashes[i]] = i;

        for (int j = 0; j < k; ++j)
        {
            std::string fonction;
            std::cin >> fonction;

            bool change = action == "AJOUTE" ? presentes.insert(fonction).second
                                             : presentes.erase(fonction) > 0;
            if (change)
                dernierChangement[fonction] = i;
        }
    }

    int a = 0;
    std::cin >> a;

    int mensonges = 0;
    for (int n = 1; n <= a; ++n)
    {
        std::string hash, etat, fonction;
        std::cin >> hash >> etat >> fonction;

        bool presente = presentes.count(fonction) > 0;
        bool vraie = etat == "DISPONIBLE" ? presente : !presente;
        if (vraie)
        {
            std::cout << "ANNONCE " << n << " VRAI\n";
            continue;
        }

        // Fausse au dernier commit : elle l'est depuis le dernier changement,
        // mais on ne remonte pas avant le commit où elle a été écrite.
        int ecrite = indiceDuCommit[hash];
        auto trouve = dernierChangement.find(fonction);
        int depuis = ecrite;
        if (trouve != dernierChangement.end() && trouve->second > ecrite)
            depuis = trouve->second;

        std::cout << "ANNONCE " << n << " MENT " << hashes[depuis] << '\n';
        ++mensonges;
    }

    std::cout << "MENSONGES " << mensonges << '\n';
    return 0;
}
