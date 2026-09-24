#pragma once

template<typename T>
inline MyHash<T>::MyHash() {}

template<typename T>
inline unsigned int MyHash<T>::operator()(T value) {
    switch (_hashType) {
    case HashType::None:
        return ConvertToInt(value);
    default:
        return ConvertToInt(value);
    }
}

template<typename T>
inline unsigned int MyHash<T>::ConvertToInt(T value) {
    return (unsigned int)value;
}

template<>
inline unsigned int MyHash<std::string>::ConvertToInt(std::string value) {
    unsigned int hash = 0;
    for (char c : value) hash = hash * 131 + c;
    return hash;
}
