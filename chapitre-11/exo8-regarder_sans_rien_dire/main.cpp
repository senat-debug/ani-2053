#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

struct Contournement
{
    std::string nom;
    long long fois = 0;
    long long premier = 0;
};

const long long FIN_SEANCE = 20 * 60;

long long Secondes(const std::string& instant)
{
    return std::stoll(instant.substr(0, 2)) * 60 + std::stoll(instant.substr(3, 2));
}

int main()
{
    std::string mot;
    std::size_t n = 0;
    std::cin >> mot >> n;

    // Rangés dans l'ordre de leur première ligne en séance, pour que le tri
    // stable départage les dernières égalités.
    std::vector<Contournement> observes;
    std::map<std::string, std::size_t> position;
    long long horsSeance = 0;

    for (std::size_t i = 0; i < n; ++i)
    {
        std::string instant, nom;
        std::cin >> instant >> nom;
        long long t = Secondes(instant);
        if (t > FIN_SEANCE)
        {
            ++horsSeance;
            continue;
        }

        auto trouve = position.find(nom);
        if (trouve == position.end())
        {
            position[nom] = observes.size();
            observes.push_back({nom, 1, t});
            continue;
        }
        Contournement& contournement = observes[trouve->second];
        ++contournement.fois;
        contournement.premier = std::min(contournement.premier, t);
    }

    std::size_t m = 0;
    std::cin >> mot >> m;
    std::vector<std::string> cites(m);
    for (std::string& nom : cites)
        std::cin >> nom;
    std::set<std::string> dits(cites.begin(), cites.end());

    std::stable_sort(observes.begin(), observes.end(),
                     [](const Contournement& gauche, const Contournement& droite) {
                         if (gauche.fois != droite.fois)
                             return gauche.fois > droite.fois;
                         return gauche.premier < droite.premier;
                     });

    long long nonDits = 0, foisNonDites = 0, foisTotal = 0;
    for (const Contournement& contournement : observes)
    {
        bool dit = dits.count(contournement.nom) > 0;
        if (!dit)
        {
            ++nonDits;
            foisNonDites += contournement.fois;
        }
        foisTotal += contournement.fois;
        std::cout << contournement.nom << ' ' << contournement.fois
                  << (dit ? " DIT" : " NON_DIT") << '\n';
    }

    std::cout << "NON_DITS " << nonDits << '\n';
    std::cout << "PART_NON_DITE " << (foisTotal > 0 ? 100 * foisNonDites / foisTotal : 0) << '\n';

    std::string sansVu;
    for (const std::string& nom : cites)
        if (position.count(nom) == 0)
            sansVu += ' ' + nom;
    std::cout << "DIT_SANS_VU" << (sansVu.empty() ? " AUCUN" : sansVu) << '\n';
    std::cout << "HORS_SEANCE " << horsSeance << '\n';
    return 0;
}
