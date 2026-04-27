#pragma once

/*
 * StringCrypt.h - Compile-time XOR String Encryption
 *
 * Provides macros for encrypting strings at compile time and decrypting at runtime.
 * This helps avoid static string detection by AV/EDR solutions.
 */

#include "utils.h"

// XOR Key for encryption (can be randomized per build)
#define XOR_KEY_1 0x5A
#define XOR_KEY_2 0x3C
#define XOR_KEY_3 0x7E

// Compile-time string length
#define CRYPT_STRLEN(s) (sizeof(s) - 1)

// Single character XOR encryption
#define CRYPT_CHAR(c, k) ((c) ^ (k))

// Compile-time encrypted string structure
template<size_t N>
struct CryptString {
    char data[N];
    size_t length;
    BYTE key;

    __forceinline void Decrypt(char* output) {
        for (size_t i = 0; i < length; i++) {
            output[i] = data[i] ^ key;
        }
        output[length] = '\0';
    }

    __forceinline void DecryptW(wchar_t* output) {
        for (size_t i = 0; i < length; i++) {
            output[i] = (wchar_t)(data[i] ^ key);
        }
        output[length] = L'\0';
    }
};

// Runtime XOR encryption/decryption
__forceinline void XorEncryptDecrypt(PBYTE data, SIZE_T size, BYTE key) {
    for (SIZE_T i = 0; i < size; i++) {
        data[i] ^= key;
    }
}

// Multi-key XOR encryption for better obfuscation
__forceinline void XorEncryptDecryptMulti(PBYTE data, SIZE_T size, PBYTE keys, SIZE_T keyCount) {
    for (SIZE_T i = 0; i < size; i++) {
        data[i] ^= keys[i % keyCount];
    }
}

// Runtime string encryption with stack allocation
#define STACK_CRYPT_STR(str, key) \
    []() -> char* { \
        static char encrypted[] = str; \
        constexpr size_t len = sizeof(str) - 1; \
        static bool decrypted = false; \
        if (!decrypted) { \
            for (size_t i = 0; i < len; i++) encrypted[i] ^= key; \
            decrypted = true; \
        } \
        return encrypted; \
    }()

// Macro for encrypting wide strings at runtime
#define STACK_CRYPT_WSTR(str, key) \
    []() -> wchar_t* { \
        static wchar_t encrypted[] = L##str; \
        constexpr size_t len = (sizeof(L##str) / sizeof(wchar_t)) - 1; \
        static bool decrypted = false; \
        if (!decrypted) { \
            for (size_t i = 0; i < len; i++) encrypted[i] ^= (wchar_t)key; \
            decrypted = true; \
        } \
        return encrypted; \
    }()

// Compile-time XOR encryption helper
constexpr char CryptChar(char c, BYTE key) {
    return c ^ key;
}

constexpr wchar_t CryptWChar(wchar_t c, BYTE key) {
    return c ^ key;
}

// Compile-time string encryption for constant strings
template<size_t N>
class EncryptedString {
private:
    char encrypted[N];
    BYTE key;

public:
    constexpr EncryptedString(const char(&str)[N], BYTE k) : encrypted{}, key(k) {
        for (size_t i = 0; i < N; i++) {
            encrypted[i] = str[i] ^ key;
        }
    }

    void Decrypt(char* output) const {
        for (size_t i = 0; i < N; i++) {
            output[i] = encrypted[i] ^ key;
        }
    }

    constexpr size_t Length() const { return N - 1; }
};

// Wide string version
template<size_t N>
class EncryptedWString {
private:
    wchar_t encrypted[N];
    BYTE key;

public:
    constexpr EncryptedWString(const wchar_t(&str)[N], BYTE k) : encrypted{}, key(k) {
        for (size_t i = 0; i < N; i++) {
            encrypted[i] = str[i] ^ key;
        }
    }

    void Decrypt(wchar_t* output) const {
        for (size_t i = 0; i < N; i++) {
            output[i] = encrypted[i] ^ key;
        }
    }

    constexpr size_t Length() const { return N - 1; }
};

// Convenient macros for use in code
#define CRYPT_STR(str) EncryptedString(str, XOR_KEY_1)
#define CRYPT_WSTR(str) EncryptedWString(L##str, XOR_KEY_1)

// Stack-based string decryption (auto-wipes when scope ends)
class ScopedDecryptedString {
private:
    char* buffer;
    size_t size;

public:
    ScopedDecryptedString(const char* encrypted, size_t len, BYTE key) : size(len) {
        buffer = (char*)MemAllocLocal(len + 1);
        for (size_t i = 0; i < len; i++) {
            buffer[i] = encrypted[i] ^ key;
        }
        buffer[len] = '\0';
    }

    ~ScopedDecryptedString() {
        if (buffer) {
            memset(buffer, 0, size);
            MemFreeLocal((LPVOID*)&buffer, size + 1);
        }
    }

    const char* Get() const { return buffer; }
    operator const char* () const { return buffer; }
};

// Wide string version
class ScopedDecryptedWString {
private:
    wchar_t* buffer;
    size_t size;

public:
    ScopedDecryptedWString(const wchar_t* encrypted, size_t len, BYTE key) : size(len) {
        buffer = (wchar_t*)MemAllocLocal((len + 1) * sizeof(wchar_t));
        for (size_t i = 0; i < len; i++) {
            buffer[i] = encrypted[i] ^ key;
        }
        buffer[len] = L'\0';
    }

    ~ScopedDecryptedWString() {
        if (buffer) {
            memset(buffer, 0, size * sizeof(wchar_t));
            MemFreeLocal((LPVOID*)&buffer, (size + 1) * sizeof(wchar_t));
        }
    }

    const wchar_t* Get() const { return buffer; }
    operator const wchar_t* () const { return buffer; }
};

// Generate randomized XOR key at compile time
#define RANDOM_KEY() (XOR_KEY_1 ^ XOR_KEY_2 ^ XOR_KEY_3)

// Helper for string obfuscation in memory
__forceinline void ObfuscateString(char* str, size_t len) {
    XorEncryptDecrypt((PBYTE)str, len, RANDOM_KEY());
}

__forceinline void DeobfuscateString(char* str, size_t len) {
    XorEncryptDecrypt((PBYTE)str, len, RANDOM_KEY());
}

// String hashing for API names (alternative to storing strings)
constexpr ULONG HashStringCompileTime(const char* str, BYTE key) {
    ULONG hash = 1572;
    while (*str) {
        char c = (*str) ^ key;
        if (c >= 'A' && c <= 'Z') c += 0x20;
        hash = ((hash << 5) + hash) + c;
        str++;
    }
    return hash;
}
