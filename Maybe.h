#ifndef GLAEBHOERL_MAYBE_H
#define GLAEBHOERL_MAYBE_H

namespace glaebhoerl
{

template<typename T>
struct Maybe
{
    Maybe(): m_value(0) { }
    Maybe(const T& value): m_value(new T(value)) { }
    Maybe(const Maybe<T>& other): m_value(other ? new T(*other) : 0) { }
    ~Maybe() { clear(); }

    void operator=(const T& value)
    {
        if (m_value) {
            *m_value = value;
        } else {
            m_value = new T(value);
        }
    }

    void operator=(const Maybe<T>& other)
    {
        if (other) {
            *this = *other;
        } else {
            clear();
        }
    }

    void clear()
    {
        delete m_value;
        m_value = 0;
    }

    bool hasValue() const { return m_value; }

    bool isEmpty() const { return !hasValue(); }

    explicit operator bool() const { return hasValue(); }

          T& operator  *()       { return *m_value; }
    const T& operator  *() const { return *m_value; }
          T* operator ->()       { return  m_value; }
    const T* operator ->() const { return  m_value; }

    T orOtherwise(T other) const { return m_value ? *m_value : other; }
    T otherwiseDefault() const { return (*this).orOtherwise(T()); }

    bool operator==(const Maybe<T>& other) const
    {
        if (m_value && other) {
            return *m_value == *other;
        } else {
            return !m_value && !other;
        }
    }

private:
    T* m_value;
};

template<typename T>
Maybe<T> empty() { return Maybe<T>(); }

template<typename T>
Maybe<T> value(T val) { return Maybe<T>(val); }

}

#endif
