#include <bits/stdc++.h>
using namespace std;

const int ALFABETO = 26;
const int NULO = -1;

struct Nodo {
    array<int, ALFABETO> hijos;  
    int fin_de_palabra;           
    int pasan;                  
    Nodo() : fin_de_palabra(0), pasan(0) { hijos.fill(NULO); }
};

class TriePersistente {
    vector<Nodo> nodos;          
    vector<int> raices;          
public:
    int copiar(int id) {
        nodos.push_back(id == NULO ? Nodo() : nodos[id]);
        return (int)nodos.size() - 1;
    }

    int copiar_version(int v) {
        validar_version(v);
        raices.push_back(raices[v]);
        return (int)raices.size() - 1;
    }

    static int indice(char c) { return c - 'a'; }

    void validar_palabra(const string& palabra) const {
        for (char c : palabra)
            if (c < 'a' || c > 'z') throw invalid_argument("solo se admiten letras a-z: " + palabra);
    }
    void validar_version(int v) const {
        if (v < 0 || v >= (int)raices.size()) throw out_of_range("version inexistente");
    }

    int buscar(int v, const string& palabra) const {
        int actual = raices[v];
        for (char c : palabra) {
            actual = nodos[actual].hijos[indice(c)];
            if (actual == NULO) return NULO;
        }
        return actual;
    }


    TriePersistente() {
        nodos.push_back(Nodo());  
        raices.push_back(0);     
    }

    int insertar(int v, const string& palabra) {
        validar_version(v); validar_palabra(palabra);
        int nueva_raiz = copiar(raices[v]);
        nodos[nueva_raiz].pasan++;
        int actual = nueva_raiz;
        for (char c : palabra) {
            int hijo_viejo = nodos[actual].hijos[indice(c)];
            int hijo_nuevo = copiar(hijo_viejo);      
            nodos[hijo_nuevo].pasan++;
            nodos[actual].hijos[indice(c)] = hijo_nuevo; 
            actual = hijo_nuevo;
        }
        nodos[actual].fin_de_palabra++;
        raices.push_back(nueva_raiz);
        return (int)raices.size() - 1;
    }

    int borrar(int v, const string& palabra) {
        validar_version(v); validar_palabra(palabra);
        int fin = buscar(v, palabra);
        if (fin == NULO || nodos[fin].fin_de_palabra == 0) {
            raices.push_back(raices[v]);
            return (int)raices.size() - 1;
        }
        int nueva_raiz = copiar(raices[v]);
        nodos[nueva_raiz].pasan--;
        int actual = nueva_raiz;
        for (char c : palabra) {
            int hijo_viejo = nodos[actual].hijos[indice(c)];
            if (nodos[hijo_viejo].pasan == 1) {
                // Ninguna otra palabra usa esta rama en la versión nueva: se poda aquí.
                nodos[actual].hijos[indice(c)] = NULO;
                raices.push_back(nueva_raiz);
                return (int)raices.size() - 1;
            }
            int hijo_nuevo = copiar(hijo_viejo);
            nodos[hijo_nuevo].pasan--;
            nodos[actual].hijos[indice(c)] = hijo_nuevo;
            actual = hijo_nuevo;
        }
        nodos[actual].fin_de_palabra--;
        raices.push_back(nueva_raiz);
        return (int)raices.size() - 1;
    }

    bool existe(int v, const string& palabra) const {
        validar_version(v); validar_palabra(palabra);
        int fin = buscar(v, palabra);
        return fin != NULO && nodos[fin].fin_de_palabra > 0;
    }

    int contar(int v, const string& palabra) const {
        validar_version(v); validar_palabra(palabra);
        int fin = buscar(v, palabra);
        return fin == NULO ? 0 : nodos[fin].fin_de_palabra;
    }

    // Total de palabras en la versión v, contando duplicados: O(1), está en la raíz.
    int num_palabras(int v) const {
        validar_version(v);
        return nodos[raices[v]].pasan;
    }

    int num_palabras_with_prefix(int v, const string& prefijo) const {
        validar_version(v); validar_palabra(prefijo);
        int nodo = buscar(v, prefijo);
        return nodo == NULO ? 0 : nodos[nodo].pasan;
    }

    int num_versiones() const { return (int)raices.size(); }
    int num_nodos() const { return (int)nodos.size(); }
};

int main() {
    
    /*
    TriePersistente trie;

    auto mostrar = [&](int v) {
        cout << "v" << v << ": num_palabras=" << trie.num_palabras(v)
             << " | casa:" << trie.contar(v, "casa")
             << " cama:" << trie.contar(v, "casco")
             << " carro:" << trie.contar(v, "carro") << "\n";
             << " cosa:" << trie.contar(v, "cosa") << "\n";
    };

    int v0 = 0;                                  // trie vacío
    int v1 = trie.insertar(v0, "casa");
    int v2 = trie.insertar(v1, "casco");
    int nodos_antes = trie.num_nodos();
    int v3 = trie.insertar(v2, "casa");          // duplicado
    cout << "nodos creados al insertar 'casa' otra vez: " << trie.num_nodos() - nodos_antes << "\n"; // 5 = L+1

    int v4 = trie.borrar(v3, "casa");            // quita una copia
    int v5 = trie.borrar(v4, "casa");            // quita la última copia
    int v6 = trie.insertar(v2, "cama");          // rama desde v2 (persistencia total)
    int v7 = trie.borrar(v6, "perro");           // no existe: versión idéntica

    for (int v : {v0, v1, v2, v3, v4, v5, v6, v7}) mostrar(v);

    cout << boolalpha;
    cout << "existe(v5,'casa')=" << trie.existe(v5, "casa") << "\n";   // false
    cout << "existe(v3,'casa')=" << trie.existe(v3, "casa") << "\n";   // true: v3 no cambió
    cout << "existe(v6,'cas')="  << trie.existe(v6, "cas")  << "\n";   // false: solo es prefijo
    cout << "existe(v6,'cama')=" << trie.existe(v6, "cama") << "\n";   // true
    cout << "existe(v3,'cama')=" << trie.existe(v3, "cama") << "\n";   // false: otra rama
    cout << "versiones=" << trie.num_versiones() << " nodos totales=" << trie.num_nodos() << "\n";
    */
    //


    TriePersistente trie;


    int v_actual = 0;                                  
    
    int num_op = 0;
    int type_op = 0;
    string palabra;

    cin>>num_op;
    for(int i=0;i<num_op;i++){
        cin>>type_op>>palabra;
        if(type_op==1){
            v_actual = trie.insertar(v_actual, palabra);
        }
        else if(type_op==2){
            v_actual = trie.copiar_version(stoi(palabra));
        }
        else if(type_op==3){
            v_actual = trie.copiar_version(v_actual);
            cout << trie.num_palabras_with_prefix(v_actual, palabra) << "\n";
        }
    }         



}
