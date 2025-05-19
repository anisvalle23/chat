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
    int contador;

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
        contador++;
    }

    bool desencolar(T& valor) {
        if (estaVacia()) return false;
        NodoCola<T>* temp = frente;
        valor = temp->dato;
        frente = frente->siguiente;
        if (!frente) final = nullptr;
        delete temp;
        contador--;
        return true;
    }

    bool estaVacia() const {
        return frente == nullptr;
    }

    void limpiar() {
        T temp;
        while (desencolar(temp)) {}
        contador = 0;
    }

    bool frenteElemento(T& valor) const {
        if (estaVacia()) return false;
        valor = frente->dato;
        return true;
    }

    int tamano() const {
        return contador;
    }
};

#endif
