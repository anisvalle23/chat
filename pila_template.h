#ifndef PILA_TEMPLATE_H
#define PILA_TEMPLATE_H

template <typename T>
class NodoPila {
public:
    T dato;
    NodoPila* siguiente;

    NodoPila(T d) : dato(d), siguiente(nullptr) {}
};

template <typename T>
class Pila {
private:
    NodoPila<T>* cima;

public:
    Pila() : cima(nullptr) {}

    ~Pila() {
        while (!estaVacia())
            desapilar();
    }

    void apilar(T valor) {
        NodoPila<T>* nuevo = new NodoPila<T>(valor);
        nuevo->siguiente = cima;
        cima = nuevo;
    }

    T desapilar() {
        if (estaVacia()) return T();  // Retorna objeto por defecto si está vacía
        NodoPila<T>* temp = cima;
        T dato = temp->dato;
        cima = cima->siguiente;
        delete temp;
        return dato;
    }

    bool estaVacia() const {
        return cima == nullptr;
    }
};

#endif // PILA_TEMPLATE_H
