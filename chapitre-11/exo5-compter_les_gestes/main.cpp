#include <iostream>
#include <sstream>
#include <string>

std::string Minutes(long long secondes)
{
    std::string reste = std::to_string(secondes % 60);
    if (reste.size() < 2)
        reste = "0" + reste;
    return std::to_string(secondes / 60) + ":" + reste;
}

int main()
{
    bool enCours = false;
    std::string nom;
    long long debut = 0, minimum = 0, parJour = 0;
    long long gestes = 0, attente = 0;
    long long totalSecondes = 0;

    std::string ligne;
    while (std::getline(std::cin, ligne))
    {
        std::istringstream champs(ligne);
        long long t = 0;
        std::string evenement;
        if (!(champs >> t >> evenement))
            continue;

        if (evenement == "DEBUT")
        {
            champs >> nom >> minimum >> parJour;
            debut = t;
            gestes = 0;
            attente = 0;
            enCours = true;
            continue;
        }

        // Hors de toute tâche, gestes et attentes ne comptent pas.
        if (!enCours)
            continue;

        if (evenement == "FIN")
        {
            long long enTrop = gestes > minimum ? gestes - minimum : 0;
            long long jour = attente * parJour / 1000;
            totalSecondes += jour;

            std::cout << nom << " GESTES " << gestes << " EN_TROP " << enTrop
                      << " DUREE " << t - debut << " ATTENTE " << attente
                      << " JOUR " << Minutes(jour) << '\n';
            enCours = false;
        }
        else if (evenement == "ATTENTE")
        {
            long long duree = 0;
            champs >> duree;
            attente += duree;
        }
        else if (evenement == "CLIC" || evenement == "TOUCHE"
                 || evenement == "MOLETTE" || evenement == "GLISSE")
        {
            ++gestes;
        }
    }

    if (enCours)
        std::cout << nom << " INACHEVEE\n";
    std::cout << "TOTAL_JOUR " << Minutes(totalSecondes) << '\n';
    return 0;
}
