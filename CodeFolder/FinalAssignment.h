template<typename T>
class ResizableArray {
public:
    ResizableArray() : data(nullptr), cap(0), len(0) {}

    // Copy constructor
    ResizableArray(const ResizableArray& other) : data(nullptr), cap(other.cap), len(other.len) {
        if (cap > 0) {
            data = new T[cap];
            for (size_t i = 0; i < len; ++i) {
                data[i] = other.data[i];
            }
        }
    }

    // Move constructor
    ResizableArray(ResizableArray&& other) noexcept : data(other.data), cap(other.cap), len(other.len) {
        other.data = nullptr;
        other.cap = 0;
        other.len = 0;
    }

    // Copy assignment operator
    ResizableArray& operator=(const ResizableArray& other) {
        if (this != &other) {
            delete[] data;
            cap = other.cap;
            len = other.len;
            data = (cap > 0) ? new T[cap] : nullptr;
            for (size_t i = 0; i < len; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    // Move assignment operator
    ResizableArray& operator=(ResizableArray&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            cap = other.cap;
            len = other.len;
            other.data = nullptr;
            other.cap = 0;
            other.len = 0;
        }
        return *this;
    }

    // Destructor
    ~ResizableArray() { delete[] data; }

    void push_back(const T& val) {
        if (len == cap) resize();
        data[len++] = val;
    }

    T& operator[](size_t idx) { assert(idx < len); return data[idx]; }
    const T& operator[](size_t idx) const { assert(idx < len); return data[idx]; }
    size_t size() const { return len; }

private:
    void resize() {
        cap = (cap == 0 ? 10 : cap * 2);
        T* newdata = new T[cap];
        for (size_t i = 0; i < len; ++i) newdata[i] = data[i];
        delete[] data;
        data = newdata;
    }

    T* data;
    size_t cap;
    size_t len;
};
