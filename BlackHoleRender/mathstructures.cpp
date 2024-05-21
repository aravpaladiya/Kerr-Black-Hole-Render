//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//#include <cmath>
//#include <vector>
//#include "mathstructures.h"
//#include <initializer_list>
//#include <iostream>
//#include <glm/glm.hpp>
//
//
//
//Vec4::Vec4(float xx, float yy, float zz, float ww) {
//    x = xx;
//    y = yy;
//    z = zz;
//    w = ww;
//}
//
//Vec4::Vec4() {
//    x = 0;
//    y = 0;
//    z = 0;
//    w = 0;
//}
//
//float Vec3::getx() {
//    return x;
//}
//float Vec3::gety() {
//    return y;
//}
//float Vec3::getz() {
//    return z;
//}
//
//float Vec4::getx() {
//    return x;
//}
//float Vec4::gety() {
//    return y;
//}
//float Vec4::getz() {
//    return z;
//}
//float Vec4::getw() {
//    return w;
//}
//
//Vec3::Vec3() {
//    x = 0;
//    y = 0;
//    z = 0;
//}
//
//Vec3::Vec3(float xx, float yy, float zz) {
//    x = xx;
//    y = yy;
//    z = zz;
//}
//
//Vec4 Mat4::operator*(Vec4 v) const {
//    Vec4 result = Vec4();
//    result.x = elements[0][0] * (v.getx()) + elements[0][1] * (v.gety()) + elements[0][2] * (v.getz()) + elements[0][3] * (v.getw());
//    result.y = elements[1][0] * (v.getx()) + elements[1][1] * (v.gety()) + elements[1][2] * (v.getz()) + elements[1][3] * (v.getw());
//    result.z = elements[2][0] * (v.getx()) + elements[2][1] * (v.gety()) + elements[2][2] * (v.getz()) + elements[2][3] * (v.getw());
//    result.w = elements[3][0] * (v.getx()) + elements[3][1] * (v.gety()) + elements[3][2] * (v.getz()) + elements[3][3] * (v.getw());
//
//    return result;
//}
//
//Mat4::Mat4(float m[4][4])
//{
//    for (int i = 0; i < 4; ++i) {
//        for (int j = 0; j < 4; ++j) {
//            Mat4::elements[i][j] = m[i][j];
//        }
//    }
//}
//
//Mat4::Mat4(std::initializer_list<std::initializer_list<float>> list) {
//    int i = 0, j = 0;
//    for (auto& row : list) {
//        for (float val : row) {
//            elements[i][j] = val;
//            j++;
//        }
//        j = 0;
//        i++;
//    }
//}
//std::vector<float> Mat4::to1DArray() {
//    std::vector<float> a;
//    for (int i = 0; i < 4; i++) {
//        for (int j = 0; j < 4; j++) {
//            a.push_back(elements[i][j]);
//        }
//    }
//    return a;
//}
//
//Mat4 Mat4::generateProjection()
//{
//    return Mat4();
//}
//
//Mat4::Mat4() {
//    for (int i = 0; i < 4; ++i) {
//        for (int j = 0; j < 4; ++j) {
//            elements[i][j] = 0.0f;
//        }
//    }
//}
//
//Mat4 Mat4::operator+(const Mat4 m) const
//{
//    Mat4 result;
//    for (int i = 0; i < 4; i++) {
//        for (int j = 0; j < 4; j++) {
//            result.elements[i][j] = elements[i][j] + m.elements[i][j];
//        }
//    }
//    return result;
//}
//
//Mat4 Mat4::operator-(const Mat4 m) const
//{
//    Mat4 result;
//    for (int i = 0; i < 4; i++) {
//        for (int j = 0; j < 4; j++) {
//            result.elements[i][j] = elements[i][j] - m.elements[i][j];
//        }
//    }
//    return result;
//}
//
//Mat4 Mat4::operator*(const Mat4 m) const
//{
//    Mat4 result;
//    for (int i = 0; i < 4; ++i) {
//        for (int j = 0; j < 4; ++j) {
//            for (int k = 0; k < 4; ++k) {
//                result.elements[i][j] += elements[i][k] * m.elements[k][j];
//            }
//        }
//    }
//
//    return result;
//}
//
//
//const Mat4 Mat4::identity({
//   {1, 0, 0, 0},
//   {0, 1, 0, 0},
//   {0, 0, 1, 0},
//   {0, 0, 0, 1}
//});
//
//
//
//
//Mat4 Vec3::generateTranslate() {
//    return Mat4{ {
//       {1, 0, 0, x},
//       {0, 1, 0, y},
//       {0, 0, 1, z},
//       {0, 0, 0, 1}
// } };
//}
//
//
//Mat4 Vec4::generateTranslate() {
//    return Vec3(x, y, z).generateTranslate();
//}
//
//
//
//Quaternion::Quaternion(float ww, float xx, float yy, float zz) {
//    w = ww;
//    x = xx;
//    y = yy;
//    z = zz;
//}
//
//Quaternion Quaternion::generateFromAxis(float theta, Vec3 v) {
//    float s = sin(theta / 2);
//
//    return Quaternion((float)cos(theta / 2), s * v.x, s * v.y, s * v.z);
//}
//Quaternion Quaternion::generateFromAxis(float theta, float x, float y, float z) {
//    float s = sin(theta / 2);
//
//    return Quaternion((float)cos(theta / 2), s * x, s * y, s * z);
//}
//
//Quaternion Quaternion::conjugate() {
//    return Quaternion(w, -x, -y, -z);
//}
//
//Mat4 Quaternion::toMatrix() {
//    float xx = 2 * x * x;
//    float yy = 2 * y * y;
//    float zz = 2 * z * z;
//    float xy = 2 * x * y;
//    float xz = 2 * x * z;
//    float xw = 2 * x * w;
//    float yz = 2 * y * z;
//    float yw = 2 * y * w;
//    float zw = 2 * z * w;
//    return Mat4{ {
//       {1 - yy - zz, xy - zw, xz + yw, 0},
//       {xy + zw, 1 - xx - zz, yz - xw, 0},
//       {xz - yw, yz + xw, 1 - xx - yy, 0},
//       {0, 0, 0, 1}
//    } };
//
//};
//
////
////int main() {
////    // Vec4 v = Vec4(1, 2, 3, 1);
////
////    // Mat4 m = v.generateTranslate();
////
////    Vec4 s = Vec4(1, 1, 1, 1);
////
////    Mat4 r = Quaternion::generateFromAxis(atan(1), 0, 1, 0).toMatrix();
////
////    Vec4 a = r * s;
////
////
////    std::cout << a.x << "\n" << a.y << "\n" << a.z << std::endl;
////
////
////    return 0;
////}
//
//
//
//
//
