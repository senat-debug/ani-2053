#include <iostream>
#include <string>
#include <vector>

int main()
{
    // Ce que l'utilisateur croit pouvoir défaire, opération par opération.
    std::vector<std::string> pile;
    // Ce que l'outil sait réellement défaire : les traits encore présents.
    std::vector<int> traits;

    int numeroTrait = 0;
    int operations = 0, annulables = 0;
    int annules = 0, justes = 0, riens = 0, trahis = 0;

    std::string op;
    while (std::cin >> op)
    {
        if (op == "ANNULE")
        {
            std::string attendu = "AUCUNE";
            if (!pile.empty())
            {
                attendu = pile.back();
                pile.pop_back();
            }

            std::string obtenu = "RIEN";
            if (!traits.empty())
            {
                obtenu = "TRAIT " + std::to_string(traits.back());
                traits.pop_back();
            }

            std::string verdict;
            if (obtenu == attendu || (attendu == "AUCUNE" && obtenu == "RIEN"))
            {
                verdict = "JUSTE";
                ++justes;
            }
            else if (obtenu == "RIEN")
            {
                verdict = "RIEN";
                ++riens;
            }
            else
            {
                verdict = "TRAHI";
                ++trahis;
            }

            std::cout << "ANNULE " << ++annules << " ATTENDU " << attendu
                      << " OBTENU " << obtenu << ' ' << verdict << '\n';
            continue;
        }

        if (op == "TRAIT")
        {
            traits.push_back(++numeroTrait);
            pile.push_back("TRAIT " + std::to_string(numeroTrait));
            ++annulables;
        }
        else if (op == "EFFACE")
        {
            // L'outil oublie les traits effacés : un ANNULE ne les rendra pas.
            traits.clear();
            pile.push_back(op);
        }
        else if (op == "RANGE")
        {
            pile.push_back(op);
        }
        else if (op == "DEPOSE" || op == "DEPLACE" || op == "SUPPRIME" || op == "MIROIR")
        {
            std::string nom;
            std::cin >> nom;
            pile.push_back(op + ' ' + nom);
        }
        else
        {
            continue;
        }
        ++operations;
    }

    std::cout << "JUSTES " << justes << '\n';
    std::cout << "RIENS " << riens << '\n';
    std::cout << "TRAHIS " << trahis << '\n';
    std::cout << "ANNULABLES " << annulables << " SUR " << operations << '\n';
    return 0;
}
