#pragma once

// 3D Projective Geometric Algebra
// Written by a generator written by enki.
#include <common.hpp>

static const char* basis[] = { "1","e1","e2","e3","e12","e13","e23","e123" };

class R300 {
  public:
    R300 ()  { std::fill( mvec, mvec + sizeof( mvec )/4, 0.0f ); }
    R300 (float f, int idx=0) { std::fill( mvec, mvec + sizeof( mvec )/4, 0.0f ); mvec[idx] = f; }
    float& operator [] (size_t idx) { return mvec[idx]; }
    const float& operator [] (size_t idx) const { return mvec[idx]; }
    R300 log () { int n=0; for (int i=0,j=0;i<8;i++) if (mvec[i]!=0.0f) { n++; printf("%s%0.7g%s",(j>0)?" + ":"",mvec[i],(i==0)?"":basis[i]); j++; };if (n==0) printf("0");  printf("\n"); return *this; }
    R300 Conjugate(); 
    R300 Involute();
    float norm();
    float inorm();
    R300 normalized();
  private:  
    float mvec[8];
};


//***********************
// R300.Reverse : res = ~a
// Reverse the order of the basis blades.
//***********************
inline R300 operator ~ (const R300 &a) {
  R300 res;
  res[0]=a[0];
  res[1]=a[1];
  res[2]=a[2];
  res[3]=a[3];
  res[4]=-a[4];
  res[5]=-a[5];
  res[6]=-a[6];
  res[7]=-a[7];
  return res;
};

//***********************
// R300.Dual : res = !a
// Poincare duality operator.
//***********************
inline R300 operator ! (const R300 &a) {
  R300 res;
  res[0]=-a[7];
  res[1]=-a[6];
  res[2]=a[5];
  res[3]=-a[4];
  res[4]=a[3];
  res[5]=-a[2];
  res[6]=a[1];
  res[7]=a[0];
  return res;
};

//***********************
// R300.Conjugate : res = a.Conjugate()
// Clifford Conjugation
//***********************
inline R300 R300::Conjugate () {
  R300 res;
  res[0]=this->mvec[0];
  res[1]=-this->mvec[1];
  res[2]=-this->mvec[2];
  res[3]=-this->mvec[3];
  res[4]=-this->mvec[4];
  res[5]=-this->mvec[5];
  res[6]=-this->mvec[6];
  res[7]=this->mvec[7];
  return res;
};

//***********************
// R300.Involute : res = a.Involute()
// Main involution
//***********************
inline R300 R300::Involute () {
  R300 res;
  res[0]=this->mvec[0];
  res[1]=-this->mvec[1];
  res[2]=-this->mvec[2];
  res[3]=-this->mvec[3];
  res[4]=this->mvec[4];
  res[5]=this->mvec[5];
  res[6]=this->mvec[6];
  res[7]=-this->mvec[7];
  return res;
};

//***********************
// R300.Mul : res = a * b 
// The geometric product.
//***********************
inline R300 operator * (const R300 &a, const R300 &b) {
  R300 res;
  res[0]=b[0]*a[0]+b[1]*a[1]+b[2]*a[2]+b[3]*a[3]-b[4]*a[4]-b[5]*a[5]-b[6]*a[6]-b[7]*a[7];
  res[1]=b[1]*a[0]+b[0]*a[1]-b[4]*a[2]-b[5]*a[3]+b[2]*a[4]+b[3]*a[5]-b[7]*a[6]-b[6]*a[7];
  res[2]=b[2]*a[0]+b[4]*a[1]+b[0]*a[2]-b[6]*a[3]-b[1]*a[4]+b[7]*a[5]+b[3]*a[6]+b[5]*a[7];
  res[3]=b[3]*a[0]+b[5]*a[1]+b[6]*a[2]+b[0]*a[3]-b[7]*a[4]-b[1]*a[5]-b[2]*a[6]-b[4]*a[7];
  res[4]=b[4]*a[0]+b[2]*a[1]-b[1]*a[2]+b[7]*a[3]+b[0]*a[4]-b[6]*a[5]+b[5]*a[6]+b[3]*a[7];
  res[5]=b[5]*a[0]+b[3]*a[1]-b[7]*a[2]-b[1]*a[3]+b[6]*a[4]+b[0]*a[5]-b[4]*a[6]-b[2]*a[7];
  res[6]=b[6]*a[0]+b[7]*a[1]+b[3]*a[2]-b[2]*a[3]-b[5]*a[4]+b[4]*a[5]+b[0]*a[6]+b[1]*a[7];
  res[7]=b[7]*a[0]+b[6]*a[1]-b[5]*a[2]+b[4]*a[3]+b[3]*a[4]-b[2]*a[5]+b[1]*a[6]+b[0]*a[7];
  return res;
};

//***********************
// R300.Wedge : res = a ^ b 
// The outer product. (MEET)
//***********************
inline R300 operator ^ (const R300 &a, const R300 &b) {
  R300 res;
  res[0]=b[0]*a[0];
  res[1]=b[1]*a[0]+b[0]*a[1];
  res[2]=b[2]*a[0]+b[0]*a[2];
  res[3]=b[3]*a[0]+b[0]*a[3];
  res[4]=b[4]*a[0]+b[2]*a[1]-b[1]*a[2]+b[0]*a[4];
  res[5]=b[5]*a[0]+b[3]*a[1]-b[1]*a[3]+b[0]*a[5];
  res[6]=b[6]*a[0]+b[3]*a[2]-b[2]*a[3]+b[0]*a[6];
  res[7]=b[7]*a[0]+b[6]*a[1]-b[5]*a[2]+b[4]*a[3]+b[3]*a[4]-b[2]*a[5]+b[1]*a[6]+b[0]*a[7];
  return res;
};

//***********************
// R300.Vee : res = a & b 
// The regressive product. (JOIN)
//***********************
inline R300 operator & (const R300 &a, const R300 &b) {
  R300 res;
  res[7]=1*(a[7]*b[7]);
  res[6]=1*(a[6]*b[7]+a[7]*b[6]);
  res[5]=-1*(a[5]*-1*b[7]+a[7]*b[5]*-1);
  res[4]=1*(a[4]*b[7]+a[7]*b[4]);
  res[3]=1*(a[3]*b[7]+a[5]*-1*b[6]-a[6]*b[5]*-1+a[7]*b[3]);
  res[2]=-1*(a[2]*-1*b[7]+a[4]*b[6]-a[6]*b[4]+a[7]*b[2]*-1);
  res[1]=1*(a[1]*b[7]+a[4]*b[5]*-1-a[5]*-1*b[4]+a[7]*b[1]);
  res[0]=1*(a[0]*b[7]+a[1]*b[6]-a[2]*-1*b[5]*-1+a[3]*b[4]+a[4]*b[3]-a[5]*-1*b[2]*-1+a[6]*b[1]+a[7]*b[0]);
  return res;
};

//***********************
// R300.Dot : res = a | b 
// The inner product.
//***********************
inline R300 operator | (const R300 &a, const R300 &b) {
  R300 res;
  res[0]=b[0]*a[0]+b[1]*a[1]+b[2]*a[2]+b[3]*a[3]-b[4]*a[4]-b[5]*a[5]-b[6]*a[6]-b[7]*a[7];
  res[1]=b[1]*a[0]+b[0]*a[1]-b[4]*a[2]-b[5]*a[3]+b[2]*a[4]+b[3]*a[5]-b[7]*a[6]-b[6]*a[7];
  res[2]=b[2]*a[0]+b[4]*a[1]+b[0]*a[2]-b[6]*a[3]-b[1]*a[4]+b[7]*a[5]+b[3]*a[6]+b[5]*a[7];
  res[3]=b[3]*a[0]+b[5]*a[1]+b[6]*a[2]+b[0]*a[3]-b[7]*a[4]-b[1]*a[5]-b[2]*a[6]-b[4]*a[7];
  res[4]=b[4]*a[0]+b[7]*a[3]+b[0]*a[4]+b[3]*a[7];
  res[5]=b[5]*a[0]-b[7]*a[2]+b[0]*a[5]-b[2]*a[7];
  res[6]=b[6]*a[0]+b[7]*a[1]+b[0]*a[6]+b[1]*a[7];
  res[7]=b[7]*a[0]+b[0]*a[7];
  return res;
};

//***********************
// R300.Add : res = a + b 
// Multivector addition
//***********************
inline R300 operator + (const R300 &a, const R300 &b) {
  R300 res;
      res[0] = a[0]+b[0];
    res[1] = a[1]+b[1];
    res[2] = a[2]+b[2];
    res[3] = a[3]+b[3];
    res[4] = a[4]+b[4];
    res[5] = a[5]+b[5];
    res[6] = a[6]+b[6];
    res[7] = a[7]+b[7];
  return res;
};

//***********************
// R300.Sub : res = a - b 
// Multivector subtraction
//***********************
inline R300 operator - (const R300 &a, const R300 &b) {
  R300 res;
      res[0] = a[0]-b[0];
    res[1] = a[1]-b[1];
    res[2] = a[2]-b[2];
    res[3] = a[3]-b[3];
    res[4] = a[4]-b[4];
    res[5] = a[5]-b[5];
    res[6] = a[6]-b[6];
    res[7] = a[7]-b[7];
  return res;
};

//***********************
// R300.smul : res = a * b 
// scalar/multivector multiplication
//***********************
inline R300 operator * (const float &a, const R300 &b) {
  R300 res;
      res[0] = a*b[0];
    res[1] = a*b[1];
    res[2] = a*b[2];
    res[3] = a*b[3];
    res[4] = a*b[4];
    res[5] = a*b[5];
    res[6] = a*b[6];
    res[7] = a*b[7];
  return res;
};

//***********************
// R300.muls : res = a * b 
// multivector/scalar multiplication
//***********************
inline R300 operator * (const R300 &a, const float &b) {
  R300 res;
      res[0] = a[0]*b;
    res[1] = a[1]*b;
    res[2] = a[2]*b;
    res[3] = a[3]*b;
    res[4] = a[4]*b;
    res[5] = a[5]*b;
    res[6] = a[6]*b;
    res[7] = a[7]*b;
  return res;
};

//***********************
// R300.sadd : res = a + b 
// scalar/multivector addition
//***********************
inline R300 operator + (const float &a, const R300 &b) {
  R300 res;
    res[0] = a+b[0];
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
// R300.adds : res = a + b 
// multivector/scalar addition
//***********************
inline R300 operator + (const R300 &a, const float &b) {
  R300 res;
    res[0] = a[0]+b;
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
// R300.ssub : res = a - b 
// scalar/multivector subtraction
//***********************
inline R300 operator - (const float &a, const R300 &b) {
  R300 res;
    res[0] = a-b[0];
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
// R300.subs : res = a - b 
// multivector/scalar subtraction
//***********************
inline R300 operator - (const R300 &a, const float &b) {
  R300 res;
    res[0] = a[0]-b;
      res[1] = a[1];
    res[2] = a[2];
    res[3] = a[3];
    res[4] = a[4];
    res[5] = a[5];
    res[6] = a[6];
    res[7] = a[7];
  return res;
};


inline float R300::norm() { return sqrt(std::abs(((*this)*Conjugate()).mvec[0])); }
inline float R300::inorm() { return (!(*this)).norm(); }
inline R300 R300::normalized() { return (*this) * (1/norm()); }


static R300 e1(1.0f,1);
static R300 e2(1.0f,2);
static R300 e3(1.0f,3);
static R300 e12(1.0f,4);
static R300 e13(1.0f,5);
static R300 e23(1.0f,6);
static R300 e123(1.0f,7);


int main (int argc, char **argv) {
  
  printf("e1*e1         : "); (e1*e1).log();
  printf("pss           : "); e123.log();
  printf("pss*pss       : "); (e123*e123).log();

  return 0;
}