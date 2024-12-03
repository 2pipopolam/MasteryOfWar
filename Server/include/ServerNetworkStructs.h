#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>

struct Vector3 {
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    float length() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    Vector3 normalize() const {
        float len = length();
        if (len > 0) {
            return Vector3(x/len, y/len, z/len);
        }
        return *this;
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    float dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
};

enum class EGameMapType {
    Pistol_Map,
    Rifle_Map,
    Sniper_Map,
    Grenade_Map
};

struct GameSessionInfo {
    int32_t sessionId;
    EGameMapType mapType;
    int32_t currentPlayers;
    bool hasPassword;
    std::string sessionName;
};

struct PlayerState {
    int32_t playerId;
    Vector3 position;
    Vector3 rotation;
    bool isCrouching;
    bool isWalking;
    struct {
        bool isFiring;
        bool isReloading;
        int32_t currentAmmo;
    } weapon;
};

struct ShotInfo {
    int32_t shooterId;
    Vector3 startLocation;
    Vector3 direction;
    float speed;
    int32_t damage;
    float spread;
    int32_t bulletId;
};

struct HitInfo {
    int32_t bulletId;
    Vector3 hitLocation;
    Vector3 hitNormal;
    float damageTaken;
};



struct GrenadeInfo {
    int32_t throwerId;
    Vector3 location;
    Vector3 velocity;
    Vector3 rotation;
};


namespace VectorUtils {
    inline float Distance(const Vector3& a, const Vector3& b) {
        Vector3 diff = a - b;
        return diff.length();
    }

    inline Vector3 Cross(const Vector3& a, const Vector3& b) {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    inline float Angle(const Vector3& a, const Vector3& b) {
        float dot = a.dot(b);
        float lengths = a.length() * b.length();
        if (lengths > 0) {
            return std::acos(dot / lengths);
        }
        return 0;
    }
}
