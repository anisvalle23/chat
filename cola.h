#ifndef COLA_H
#define COLA_H

template <typename T>
class NodoCola {
public:
    T dato;
    NodoCola* siguiente;

    NodoCola(const T& valor) : dato(valor), siguiente(nullptr) {}
};

template <typename T>
class Cola {
private:
    NodoCola<T>* frente;
    NodoCola<T>* final;
    int contador;  // ✅ Contador interno para tamaño

public:
    Cola() : frente(nullptr), final(nullptr), contador(0) {}

    ~Cola() {
        limpiar();
    }

    void encolar(const T& valor) {
        NodoCola<T>* nuevo = new NodoCola<T>(valor);
        if (estaVacia()) {
            frente = final = nuevo;
        } else {
            final->siguiente = nuevo;
            final = nuevo;
        }
        contador++;  // ✅ Incrementar tamaño
    }

    bool desencolar(T& valor) {
        if (estaVacia()) return false;
        NodoCola<T>* temp = frente;
        valor = temp->dato;
        frente = frente->siguiente;
        if (!frente) final = nullptr;
        delete temp;
        contador--;  // ✅ Decrementar tamaño
        return true;
    }

    bool estaVacia() const {
        return frente == nullptr;
    }

    void limpiar() {
        T temp;
        while (desencolar(temp)) {}
        contador = 0;  // ✅ Resetear contador
    }

    bool frenteElemento(T& valor) const {
        if (estaVacia()) return false;
        valor = frente->dato;
        return true;
    }

    int tamano() const {  // ✅ Método para obtener número de elementos
        return contador;
    }
};

#endif // COLA_H
