#include <iostream>
#include <vector>
using namespace std;

int main() {
    long long D, C;
    int n;
    cin >> D >> C >> n;
    vector<long long> t(n);
    for (int i = 0; i < n; i++) cin >> t[i];

    long long voixMax = 0, retardMax = 0, coupes = 0;
    long long finCharge = 0; 
    int debut = 0;           

    for (int i = 0; i < n; i++) {
       
        while (debut <= i && t[debut] + D <= t[i]) debut++;
        long long voix = i - debut + 1;

        
        long long depart = t[i];
        if (i > 0 && finCharge > depart) depart = finCharge;
        finCharge = depart + C;
        long long retard = finCharge - t[i];

        
        bool coupe = (i + 1 < n && t[i + 1] < t[i] + D);

        cout << t[i] << " " << voix << " " << retard << " " << (coupe ? "COUPE" : "ENTIER") << "\n";

        if (voix > voixMax) voixMax = voix;
        if (retard > retardMax) retardMax = retard;
        if (coupe) coupes++;
    }

    cout << "VOIX_MAX " << voixMax << "\n";
    cout << "RETARD_MAX " << retardMax << "\n";
    cout << "COUPES " << coupes << "\n";
    return 0;
}
