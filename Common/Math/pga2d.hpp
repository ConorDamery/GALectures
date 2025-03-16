#pragma once

// 3D Projective Geometric Algebra
// Written by a generator written by enki.
#include <common.hpp>

class PGA2D {
public:
    static const char* basis[] = { "1","e0","e1","e2","e01","e20","e12","e012" };

    PGA2D() { std::fill(mvec, mvec + sizeof(mvec) / 4, 0.0f); }
    PGA2D(float f, int idx = 0) { std::fill(mvec, mvec + sizeof(mvec) / 4, 0.0f); mvec[idx] = f; }
    float& operator [] (size_t idx) { return mvec[idx]; }
    const float& operator [] (size_t idx) const { return mvec[idx]; }
    PGA2D log() { int n = 0; for (int i = 0, j = 0; i < 8; i++) if (mvec[i] != 0.0f) { n++; printf("%s%0.7g%s", (j > 0) ? " + " : "", mvec[i], (i == 0) ? "" : basis[i]); j++; }; if (n == 0) printf("0");  printf("\n"); return *this; }
    PGA2D Conjugate();
    PGA2D Involute();
    float norm();
    float inorm();
    PGA2D normalized();
private:
    float mvec[8];
};


//***********************
// PGA2D.Reverse : res = ~a
// Reverse the order of the basis blades.
//***********************
inline PGA2D operator ~ (const PGA2D& a) {
    PGA2D res;
    res[0] = a[0];
    res[1] = a[1];
    res[2] = a[2];
    res[3] = a[3];
    res[4] = -a[4];
    res[5] = -a[5];
    res[6] = -a[6];
    res[7] = -a[7];
    return res;
};

//***********************
// PGA2D.Dual : res = !a
// Poincare duality operator.
//***********************
inline PGA2D operator ! (const PGA2D& a) {
    PGA2D res;
    res[0] = a[7];
    res[1] = a[6];
    res[2] = a[5];
    res[3] = a[4];
    res[4] = a[3];
    res[5] = a[2];
    res[6] = a[1];
    res[7] = a[0];
    return res;
};

//***********************
// PGA2D.Conjugate : res = a.Conjugate()
// Clifford Conjugation
//***********************
inline PGA2D PGA2D::Conjugate() {
    PGA2D res;
    res[0] = this->mvec[0];
    res[1] = -this->mvec[1];
    res[2] = -this->mvec[2];
    res[3] = -this->mvec[3];
    res[4] = -this->mvec[4];
    res[5] = -this->mvec[5];
    res[6] = -this->mvec[6];
    res[7] = this->mvec[7];
    return res;
};

//***********************
// PGA2D.Involute : res = a.Involute()
// Main involution
//***********************
inline PGA2D PGA2D::Involute() {
    PGA2D res;
    res[0] = this->mvec[0];
    res[1] = -this->mvec[1];
    res[2] = -this->mvec[2];
    res[3] = -this->mvec[3];
    res[4] = this->mvec[4];
    res[5] = this->mvec[5];
    res[6] = this->mvec[6];
    res[7] = -this->mvec[7];
    return res;
};

//***********************
// PGA2D.Mul : res = a * b 
// The geometric product.
//***********************
inline PGA2D operator * (const PGA2D& a, const PGA2D& b) {
    PGA2D res;
    res[0] = b[0] * a[0] + b[2] * a[2] + b[3] * a[3] - b[6] * a[6];
    res[1] = b[1] * a[0] + b[0] * a[1] - b[4] * a[2] + b[5] * a[3] + b[2] * a[4] - b[3] * a[5] - b[7] * a[6] - b[6] * a[7];
    res[2] = b[2] * a[0] + b[0] * a[2] - b[6] * a[3] + b[3] * a[6];
    res[3] = b[3] * a[0] + b[6] * a[2] + b[0] * a[3] - b[2] * a[6];
    res[4] = b[4] * a[0] + b[2] * a[1] - b[1] * a[2] + b[7] * a[3] + b[0] * a[4] + b[6] * a[5] - b[5] * a[6] + b[3] * a[7];
    res[5] = b[5] * a[0] - b[3] * a[1] + b[7] * a[2] + b[1] * a[3] - b[6] * a[4] + b[0] * a[5] + b[4] * a[6] + b[2] * a[7];
    res[6] = b[6] * a[0] + b[3] * a[2] - b[2] * a[3] + b[0] * a[6];
    res[7] = b[7] * a[0] + b[6] * a[1] + b[5] * a[2] + b[4] * a[3] + b[3] * a[4] + b[2] * a[5] + b[1] * a[6] + b[0] * a[7];
    return res;
};

//***********************
// PGA2D.Wedge : res = a ^ b 
// The outer product. (MEET)
//***********************
inline PGA2D operator ^ (const PGA2D& a, const PGA2D& b) {
    PGA2D res;
    res[0] = b[0] * a[0];
    res[1] = b[1] * a[0] + b[0] * a[1];
    res[2] = b[2] * a[0] + b[0] * a[2];
    res[3] = b[3] * a[0] + b[0] * a[3];
    res[4] = b[4] * a[0] + b[2] * a[1] - b[1] * a[2] + b[0] * a[4];
    res[5] = b[5] * a[0] - b[3] * a[1] + b[1] * a[3] + b[0] * a[5];
    res[6] = b[6] * a[0] + b[3] * a[2] - b[2] * a[3] + b[0] * a[6];
    res[7] = b[7] * a[0] + b[6] * a[1] + b[5] * a[2] + b[4] * a[3] + b[3] * a[4] + b[2] * a[5] + b[1] * a[6] + b[0] * a[7];
    return res;
};

//***********************
// PGA2D.Vee : res = a & b 
// The regressive product. (JOIN)
//***********************
inline PGA2D operator & (const PGA2D& a, const PGA2D& b) {
    PGA2D res;
    res[7] = 1 * (a[7] * b[7]);
    res[6] = 1 * (a[6] * b[7] + a[7] * b[6]);
    res[5] = 1 * (a[5] * b[7] + a[7] * b[5]);
    res[4] = 1 * (a[4] * b[7] + a[7] * b[4]);
    res[3] = 1 * (a[3] * b[7] + a[5] * b[6] - a[6] * b[5] + a[7] * b[3]);
    res[2] = 1 * (a[2] * b[7] - a[4] * b[6] + a[6] * b[4] + a[7] * b[2]);
    res[1] = 1 * (a[1] * b[7] + a[4] * b[5] - a[5] * b[4] + a[7] * b[1]);
    res[0] = 1 * (a[0] * b[7] + a[1] * b[6] + a[2] * b[5] + a[3] * b[4] + a[4] * b[3] + a[5] * b[2] + a[6] * b[1] + a[7] * b[0]);
    return res;
};

//***********************
// PGA2D.Dot : res = a | b 
// The inner product.
//***********************
inline PGA2D operator | (const PGA2D& a, const PGA2D& b) {
    PGA2D res;
    res[0] = b[0] * a[0] + b[2] * a[2] + b[3] * a[3] - b[6] * a[6];
    res[1] = b[1] * a[0] + b[0] * a[1] - b[4] * a[2] + b[5] * a[3] + b[2] * a[4] - b[3] * a[5] - b[7] * a[6] - b[6] * a[7];
    res[2] = b[2] * a[0] + b[0] * a[2] - b[6] * a[3] + b[3] * a[6];
    res[3] = b[3] * a[0] + b[6] * a[2] + b[0] * a[3] - b[2] * a[6];
    res[4] = b[4] * a[0] + b[7] * a[3] + b[0] * a[4] + b[3] * a[7];
    res[5] = b[5] * a[0] + b[7] * a[2] + b[0] * a[5] + b[2] * a[7];
    res[6] = b[6] * a[0] + b[0] * a[6];
    res[7] = b[7] * a[0] + b[0] * a[7];
    return res;
};

//***********************
// PGA2D.Add : res = a + b 
// Multivector addition
//***********************
inline PGA2D operator + (const PGA2D& a, const PGA2D& b) {
    PGA2D res;
    res[0] = a[0] + b[0];
    res[1] = a[1] + b[1];
    res[2] = a[2] + b[2];
    res[3] = a[3] + b[3];
    res[4] = a[4] + b[4];
    res[5] = a[5] + b[5];
    res[6] = a[6] + b[6];
    res[7] = a[7] + b[7];
    return res;
};

//***********************
// PGA2D.Sub : res = a - b 
// Multivector subtraction
//***********************
inline PGA2D operator - (const PGA2D& a, const PGA2D& b) {
    PGA2D res;
    res[0] = a[0] - b[0];
    res[1] = a[1] - b[1];
    res[2] = a[2] - b[2];
    res[3] = a[3] - b[3];
    res[4] = a[4] - b[4];
    res[5] = a[5] - b[5];
    res[6] = a[6] - b[6];
    res[7] = a[7] - b[7];
    return res;
};

//***********************
// PGA2D.smul : res = a * b 
// scalar/multivector multiplication
//***********************
inline PGA2D operator * (const float& a, const PGA2D& b) {
    PGA2D res;
    res[0] = a * b[0];
    res[1] = a * b[1];
    res[2] = a * b[2];
    res[3] = a * b[3];
    res[4] = a * b[4];
    res[5] = a * b[5];
    res[6] = a * b[6];
    res[7] = a * b[7];
    return res;
};

//***********************
// PGA2D.muls : res = a * b 
// multivector/scalar multiplication
//***********************
inline PGA2D operator * (const PGA2D& a, const float& b) {
    PGA2D res;
    res[0] = a[0] * b;
    res[1] = a[1] * b;
    res[2] = a[2] * b;
    res[3] = a[3] * b;
    res[4] = a[4] * b;
    res[5] = a[5] * b;
    res[6] = a[6] * b;
    res[7] = a[7] * b;
    return res;
};

//***********************
// PGA2D.sadd : res = a + b 
// scalar/multivector addition
//***********************
inline PGA2D operator + (const float& a, const PGA2D& b) {
    PGA2D res;
    res[0] = a + b[0];
    res[1] = b[1];
    res[2] = b[2];
    res[3] = b[3];
    res[4] = b[4];
    res[5] = b[5];
    res[6] = b[6];
    res[7] = b[7];
    return res;
};

//***********************
// PGA2D.adds : res = a + b 
// multivector/scalar addition
//***********************
inline PGA2D operator + (const PGA2D& a, const float& b) {
    PGA2D res;
    res[0] = a[0] + b;
    res[1] = a[1];
    res[2] = a[2];
    res[3] = a[3];
    res[4] = a[4];
    res[5] = a[5];
    res[6] = a[6];
    res[7] = a[7];
    return res;
};

//***********************
// PGA2D.ssub : res = a - b 
// scalar/multivector subtraction
//***********************
inline PGA2D operator - (const float& a, const PGA2D& b) {
    PGA2D res;
    res[0] = a - b[0];
    res[1] = -b[1];
    res[2] = -b[2];
    res[3] = -b[3];
    res[4] = -b[4];
    res[5] = -b[5];
    res[6] = -b[6];
    res[7] = -b[7];
    return res;
};

//***********************
// PGA2D.subs : res = a - b 
// multivector/scalar subtraction
//***********************
inline PGA2D operator - (const PGA2D& a, const float& b) {
    PGA2D res;
    res[0] = a[0] - b;
    res[1] = a[1];
    res[2] = a[2];
    res[3] = a[3];
    res[4] = a[4];
    res[5] = a[5];
    res[6] = a[6];
    res[7] = a[7];
    return res;
};


inline float PGA2D::norm() { return sqrt(std::abs(((*this) * Conjugate()).mvec[0])); }
inline float PGA2D::inorm() { return (!(*this)).norm(); }
inline PGA2D PGA2D::normalized() { return (*this) * (1 / norm()); }


static PGA2D e0(1.0f, 1);
static PGA2D e1(1.0f, 2);
static PGA2D e2(1.0f, 3);
static PGA2D e01(1.0f, 4);
static PGA2D e20(1.0f, 5);
static PGA2D e12(1.0f, 6);
static PGA2D e012(1.0f, 7);


int main(int argc, char** argv) {

    printf("e0*e0         : "); (e0 * e0).log();
    printf("pss           : "); e012.log();
    printf("pss*pss       : "); (e012 * e012).log();

    return 0;
}