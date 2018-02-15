#ifndef MATRIX3F10_HPP
#define MATRIX3F10_HPP

#include "cfe.h"
#include "Vector10F.hpp"
#include "Vector3F.hpp"
#include "Matrix3F3.hpp"
#include "Matrix10F3.hpp"
#include "Matrix10F10.hpp"

#define M_DEG_TO_RAD_F		(0.01745329251994329576f)

namespace math
{

/* Forward declares */
class Matrix10F3;
class Matrix10F10;

class Matrix3F10
{
public:


private:
	static const int ROWS = 3;
	static const int COLS = 10;
	Vector10F data[ROWS];
	Vector10F nan;

public:
	Matrix3F10(Vector10F m0, Vector10F m1, Vector10F m2);
	~Matrix3F10();
	Vector10F& operator [] (uint32 i);
	Vector10F operator [] (uint32 i) const;
	Matrix3F10();
	Matrix10F3 Transpose(void);
	static Matrix3F10 Identity(void);
	Vector10F operator*(const Vector10F &vecIn);
	Matrix3F10 operator*(const Matrix3F10 &matIn);
	Matrix3F10 operator*(const Matrix3F3 &matIn);
	Matrix3F10 operator*(const Matrix10F10 &matIn);
    Matrix10F10 operator*(const Matrix10F3 &matIn);
	Matrix3F10 operator*(const float &scalar);
	void Zero(void);
    Matrix3F10 operator+(const Matrix3F10 &matIn) const;

protected:

};

}

#endif
