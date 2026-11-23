#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// Ordre des actions à l'intérieur d'un même tour. Les captures passent
// après toutes les actions, une fois le compteur avancé.
enum Rang { PAN, ZOOM, DEPOT, CLIC, MOVE, RANGE, GRILLE, CAPTURE };

struct Action
{
    long long n = 0;
    Rang rang = PAN;
    std::string texte;
};

const std::size_t MAX_ELEMENTS = 8;

std::vector<std::string> Decouper(const std::string& valeur, char separateur)
{
    std::vector<std::string> morceaux;
    std::size_t debut = 0;
    while (true)
    {
        std::size_t fin = valeur.find(separateur, debut);
        if (fin == std::string::npos)
        {
            morceaux.push_back(valeur.substr(debut));
            return morceaux;
        }
        morceaux.push_back(valeur.substr(debut, fin - debut));
        debut = fin + 1;
    }
}

// Un entier écrit en entier : signe éventuel, chiffres, rien d'autre.
bool LireEntier(const std::string& texte, long long& valeur)
{
    if (texte.empty() || std::isspace(static_cast<unsigned char>(texte[0])))
        return false;
    char* fin = nullptr;
    errno = 0;
    valeur = std::strtoll(texte.c_str(), &fin, 10);
    return errno == 0 && fin != texte.c_str() && *fin == '\0';
}

int main()
{
    std::vector<Action> actions;
    bool sortiePresente = false;
    long long sortie = 0;
    long long ignores = 0;

    // Une action simple, de la forme n:a:b... : au moins `champsMinimum`
    // champs, dont les suivants sont recopiés tels quels.
    auto Simple = [&](const std::string& valeur, Rang rang, const std::string& nom,
                      std::size_t champsMinimum) {
        std::vector<std::string> champs = Decouper(valeur, ':');
        Action action;
        if (champs.size() < champsMinimum || !LireEntier(champs[0], action.n))
        {
            ++ignores;
            return;
        }
        action.rang = rang;
        action.texte = nom;
        for (std::size_t i = 1; i < champsMinimum; ++i)
            action.texte += " " + champs[i];
        actions.push_back(action);
    };

    std::string ligne;
    while (std::getline(std::cin, ligne))
    {
        if (!ligne.empty() && ligne.back() == '\r')
            ligne.pop_back();
        std::size_t egal = ligne.find('=');
        if (egal == std::string::npos)
            continue;
        std::string nom = ligne.substr(0, egal);
        std::string valeur = ligne.substr(egal + 1);

        if (nom == "NK_AGENT_SHOT")
        {
            std::size_t gardes = 0;
            for (const std::string& element : Decouper(valeur, ','))
            {
                Action capture;
                if (gardes == MAX_ELEMENTS || !LireEntier(element, capture.n))
                {
                    ++ignores;
                    continue;
                }
                capture.rang = CAPTURE;
                capture.texte = "CAPTURE";
                actions.push_back(capture);
                ++gardes;
            }
        }
        else if (nom == "NK_AGENT_DROP")
        {
            std::size_t gardes = 0;
            for (const std::string& element : Decouper(valeur, ';'))
            {
                // Le chemin commence après le troisième deux-points et peut
                // lui-même en contenir : on ne découpe pas plus loin.
                std::size_t a = element.find(':');
                std::size_t b = a == std::string::npos ? a : element.find(':', a + 1);
                std::size_t c = b == std::string::npos ? b : element.find(':', b + 1);

                Action depot;
                if (gardes == MAX_ELEMENTS || c == std::string::npos
                    || !LireEntier(element.substr(0, a), depot.n))
                {
                    ++ignores;
                    continue;
                }
                depot.rang = DEPOT;
                depot.texte = "DEPOT " + element.substr(c + 1) + " "
                              + element.substr(a + 1, b - a - 1) + " "
                              + element.substr(b + 1, c - b - 1);
                actions.push_back(depot);
                ++gardes;
            }
        }
        else if (nom == "NK_AGENT_CLICK")
        {
            std::size_t avant = actions.size();
            for (const std::string& element : Decouper(valeur, ';'))
            {
                if (actions.size() - avant == MAX_ELEMENTS)
                    ++ignores;
                else
                    Simple(element, CLIC, "CLIC", 3);
            }
        }
        else if (nom == "NK_AGENT_EXIT")
        {
            if (LireEntier(valeur, sortie))
                sortiePresente = true;
            else
                ++ignores;
        }
        else if (nom == "NK_AGENT_PACK")
        {
            Simple(valeur, RANGE, "RANGE", 1);
        }
        else if (nom == "NK_AGENT_PAN")
        {
            Simple(valeur, PAN, "PAN", 3);
        }
        else if (nom == "NK_AGENT_ZOOM")
        {
            Simple(valeur, ZOOM, "ZOOM", 4);
        }
        else if (nom == "NK_AGENT_MOVE")
        {
            Simple(valeur, MOVE, "MOVE", 3);
        }
        else if (nom == "NK_AGENT_GRID")
        {
            Simple(valeur, GRILLE, "GRILLE", 2);
        }
    }

    // Une action part au tour n, une capture au tour n - 1 : c'est le
    // compteur passé à t + 1 qui la déclenche.
    auto Tour = [](const Action& action) {
        return action.rang == CAPTURE ? action.n - 1 : action.n;
    };
    std::stable_sort(actions.begin(), actions.end(),
                     [&](const Action& gauche, const Action& droite) {
                         if (Tour(gauche) != Tour(droite))
                             return Tour(gauche) < Tour(droite);
                         return gauche.rang < droite.rang;
                     });

    // La boucle s'arrête au tour où t + 1 atteint la sortie.
    bool arret = sortiePresente && sortie > 0;
    long long dernierTour = arret ? sortie - 1 : 0;

    long long jamais = 0;
    for (const Action& action : actions)
    {
        if (action.n <= 0 || (arret && Tour(action) > dernierTour))
        {
            ++jamais;
            continue;
        }
        std::cout << "TOUR " << Tour(action) << ' ' << action.texte << '\n';
    }

    if (arret)
        std::cout << "ARRET " << dernierTour << '\n';
    else
        std::cout << "SANS_ARRET\n";
    std::cout << "JAMAIS " << jamais << '\n';
    std::cout << "IGNORES " << ignores << '\n';
    return 0;
}
