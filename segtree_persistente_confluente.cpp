// Segment tree persistente con path copying (persistencia TOTAL y CONFLUENTE).
//
// Cada versión representa un MULTICONJUNTO de enteros en el dominio [0, DOMINIO).
// El árbol es dinámico: solo existen los nodos de los valores insertados; el nodo 0 es
// el "nulo" compartido (conteo 0, hijos apuntando a sí mismo), así no se reserva nada al inicio.
//
// Operaciones (t = versión de origen; todas las que modifican devuelven una versión nueva):
//   insert(t, v)             -> t con una copia más de v                       O(log D) tiempo y nodos
//   erase(t, v)              -> t con una copia menos de v (delete)            O(log D)
//   query(t, l, r)           -> cuántos elementos de t están en [l, r]         O(log D)
//   copy_versions(t)         -> versión nueva idéntica a t (comparte la raíz)  O(1), 0 nodos
//   merge_versions(a, b)     -> versión nueva = a ∪ b (suma de multiconjuntos) O(nodos comunes de a y b)
// ("delete" es palabra reservada en C++, por eso la operación se llama erase.)
//
// merge_versions es lo que hace la estructura CONFLUENTE: la versión nueva tiene dos versiones
// padre. Funciona porque los nodos son inmutables: donde solo un árbol tiene nodo, se reutiliza
// ese subárbol tal cual (se comparte); solo se crean nodos donde ambos árboles tienen contenido.

#include <bits/stdc++.h>
using namespace std;

struct Nodo {
    int izq, der;        // índices de los hijos en el pool (0 = nulo)
    long long conteo;    // cuántos elementos del multiconjunto caen en el rango del nodo
};

class SegTreePersistente {
    const int DOMINIO;            // valores válidos: 0 .. DOMINIO-1
    vector<Nodo> nodos;           // pool; un nodo nunca se modifica después de crearse
    vector<int> raices;           // raices[t] = raíz de la versión t
    vector<pair<int,int>> padres; // versión(es) de las que salió cada versión (-1 = ninguna)

    int nuevo_nodo(int izq, int der, long long conteo) {
        nodos.push_back({izq, der, conteo});
        return (int)nodos.size() - 1;
    }

    // Devuelve la raíz de la copia del subárbol 'actual' con conteo[v] += delta.
    // Solo se copian los nodos del camino hacia la hoja v; el hermano se comparte.
    int actualizar(int actual, int lo, int hi, int v, long long delta) {
        if (lo == hi)
            return nuevo_nodo(0, 0, nodos[actual].conteo + delta);
        int medio = lo + (hi - lo) / 2;
        int izq = nodos[actual].izq, der = nodos[actual].der;
        if (v <= medio) izq = actualizar(izq, lo, medio, v, delta);
        else            der = actualizar(der, medio + 1, hi, v, delta);
        return nuevo_nodo(izq, der, nodos[izq].conteo + nodos[der].conteo);
    }

    long long consultar(int actual, int lo, int hi, int l, int r) const {
        if (actual == 0 || r < lo || hi < l) return 0;       // nulo o fuera de rango
        if (l <= lo && hi <= r) return nodos[actual].conteo;  // rango cubierto
        int medio = lo + (hi - lo) / 2;
        return consultar(nodos[actual].izq, lo, medio, l, r)
             + consultar(nodos[actual].der, medio + 1, hi, l, r);
    }

    // Unión no destructiva de dos árboles: nunca modifica a ni b.
    int fusionar(int a, int b, int lo, int hi) {
        if (a == 0) return b;                 // solo b tiene algo aquí: se comparte su subárbol
        if (b == 0) return a;                 // solo a tiene algo aquí: se comparte su subárbol
        if (lo == hi)
            return nuevo_nodo(0, 0, nodos[a].conteo + nodos[b].conteo);
        int medio = lo + (hi - lo) / 2;
        int izq = fusionar(nodos[a].izq, nodos[b].izq, lo, medio);
        int der = fusionar(nodos[a].der, nodos[b].der, medio + 1, hi);
        return nuevo_nodo(izq, der, nodos[izq].conteo + nodos[der].conteo);
    }

    long long conteo_de(int t, int v) const { return consultar(raices[t], 0, DOMINIO - 1, v, v); }

    int nueva_version(int raiz, int padre1, int padre2 = -1) {
        raices.push_back(raiz);
        padres.push_back({padre1, padre2});
        return (int)raices.size() - 1;
    }

    void validar_version(int t) const {
        if (t < 0 || t >= (int)raices.size()) throw out_of_range("version inexistente: " + to_string(t));
    }
    void validar_valor(int v) const {
        if (v < 0 || v >= DOMINIO) throw out_of_range("valor fuera del dominio: " + to_string(v));
    }

public:
    explicit SegTreePersistente(int dominio) : DOMINIO(dominio) {
        if (dominio <= 0) throw invalid_argument("el dominio debe ser positivo");
        nodos.push_back({0, 0, 0});   // nodo 0 = nulo compartido
        nueva_version(0, -1);         // versión 0: multiconjunto vacío
    }

    int insert(int t, int v) {
        validar_version(t); validar_valor(v);
        return nueva_version(actualizar(raices[t], 0, DOMINIO - 1, v, +1), t);
    }

    // Borra UNA aparición de v. Si v no está, la versión nueva es idéntica a t (0 nodos nuevos).
    int erase(int t, int v) {
        validar_version(t); validar_valor(v);
        if (conteo_de(t, v) == 0) return nueva_version(raices[t], t);
        return nueva_version(actualizar(raices[t], 0, DOMINIO - 1, v, -1), t);
    }

    long long query(int t, int l, int r) const {
        validar_version(t); validar_valor(l); validar_valor(r);
        if (l > r) swap(l, r);
        return consultar(raices[t], 0, DOMINIO - 1, l, r);   // la versión se elige en la raíz
    }

    // Con nodos inmutables, copiar una versión completa es copiar un puntero.
    int copy_versions(int t) {
        validar_version(t);
        return nueva_version(raices[t], t);
    }

    int merge_versions(int a, int b) {
        validar_version(a); validar_version(b);
        return nueva_version(fusionar(raices[a], raices[b], 0, DOMINIO - 1), a, b);
    }

    int num_versiones() const { return (int)raices.size(); }
    int num_nodos() const { return (int)nodos.size(); }
    int raiz(int t) const { validar_version(t); return raices[t]; }
    pair<int,int> padres_de(int t) const { validar_version(t); return padres[t]; }

    // Lista los elementos de la versión t (con repeticiones), para depurar.
    vector<int> elementos(int t) const {
        validar_version(t);
        vector<int> salida;
        function<void(int,int,int)> recorrer = [&](int actual, int lo, int hi) {
            if (actual == 0 || nodos[actual].conteo == 0) return;
            if (lo == hi) { for (long long i = 0; i < nodos[actual].conteo; i++) salida.push_back(lo); return; }
            int medio = lo + (hi - lo) / 2;
            recorrer(nodos[actual].izq, lo, medio);
            recorrer(nodos[actual].der, medio + 1, hi);
        };
        recorrer(raices[t], 0, DOMINIO - 1);
        return salida;
    }
};

int main() {
    SegTreePersistente st(16);   // valores en [0, 15]; altura = log2(16) = 4
    auto mostrar = [&](const string& nombre, int t) {
        cout << nombre << " = v" << t << " {";
        auto e = st.elementos(t);
        for (size_t i = 0; i < e.size(); i++) cout << (i ? ", " : "") << e[i];
        auto p = st.padres_de(t);
        cout << "}  raiz=#" << st.raiz(t) << "  padres=(" << p.first << ", " << p.second << ")\n";
    };

    int v0 = 0;
    int v1 = st.insert(v0, 3);
    int v2 = st.insert(v1, 7);
    int v3 = st.insert(v2, 3);                 // duplicado
    int antes = st.num_nodos();
    int v4 = st.erase(v3, 7);
    cout << "nodos creados por erase: " << st.num_nodos() - antes << " (= log2(16) + 1)\n";

    int w1 = st.insert(v0, 10);                // otra rama desde v0 (persistencia total)
    int w2 = st.insert(w1, 12);

    antes = st.num_nodos();
    int c = st.copy_versions(v3);
    cout << "nodos creados por copy_versions: " << st.num_nodos() - antes << "\n";

    antes = st.num_nodos();
    int m = st.merge_versions(v3, w2);         // confluencia: dos padres
    cout << "nodos creados por merge_versions: " << st.num_nodos() - antes << "\n";

    int mm = st.merge_versions(m, m);          // fusionar una versión consigo misma
    int e1 = st.erase(v4, 9);                  // 9 no existe: versión idéntica

    cout << "\n";
    for (auto [nombre, t] : vector<pair<string,int>>{{"v0",v0},{"v1",v1},{"v2",v2},{"v3",v3},{"v4",v4},
                                                     {"w1",w1},{"w2",w2},{"copia(v3)",c},
                                                     {"merge(v3,w2)",m},{"merge(m,m)",mm},{"erase(v4,9)",e1}})
        mostrar(nombre, t);

    cout << "\nquery(v3, 0, 5)  = " << st.query(v3, 0, 5)  << "\n";   // 2 (los dos 3)
    cout << "query(v4, 0, 15) = " << st.query(v4, 0, 15) << "\n";     // 2
    cout << "query(m, 3, 10)  = " << st.query(m, 3, 10)  << "\n";     // 3,3,7,10 -> 4
    cout << "query(mm, 0, 15) = " << st.query(mm, 0, 15) << "\n";     // 10
    cout << "query(w2, 0, 5)  = " << st.query(w2, 0, 5)  << "\n";     // 0: w2 no se afectó
    cout << "\nversiones=" << st.num_versiones() << " nodos totales=" << st.num_nodos() << "\n";
}
