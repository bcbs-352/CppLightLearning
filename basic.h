#pragma once
//此头文件用于定义基本结构及其运算
//向量、颜色、点的坐标
#include<math.h>//fabsf(),fminf(),fmaxf(),sinf(),cosf().sqrtf()

struct Vector
{
    float x,y;
    Vector operator+(Vector v){
        return {x+v.x,y+v.y};
    }
    Vector operator-(Vector v){
        return {x-v.x,y-v.y};
    }
    Vector operator-(void){//负数
        return {-x,-y};
    }
    Vector operator*(float f){
        return {x*f,y*f};
    }
    float operator*(Vector v){
        return x*v.x + y*v.y;
    }
    Vector operator/(float f){
        return {x/f,y/f};
    }
    float len(){
        return sqrtf(x*x+y*y);
    }
    //向量单位化
    Vector normalize(){
        float length=len();
        if(length>0){
            return {x/length,y/length};
        }
        return {0.0f,0.0f};
    }
    //反射:根据单位法线向量，返回反射后的向量
    Vector reflect(const Vector normal){
        float idotn2=(normal.x*x+normal.y*y)*-2.f;//=即Vector(this)XVector(normal)
        return {x+idotn2*normal.x,y+idotn2*normal.y};
    }
};

struct Point{
    float x,y;
    Point operator+(Vector v){
        return {x+v.x,y+v.y};
    }
    Point operator-(Vector v){
        return {x-v.x,y-v.y};
    }
    Vector operator-(Point p){
        return {x-p.x,y-p.y};
    }
    bool IsValid(){
        return (x>=0.f && x<=1.f && y>0.f && y<1.f);
    }
};

struct Color{
    float r,g,b;
    Color operator+(Color c){
        return {r+c.r,g+c.g,b+c.b}; 
    }
    Color operator*(float f){
        return {r*f,g*f,b*f};
    }
    Color operator*(Color c){
        return {r*c.r,g*c.g,b*c.b};
    }
    Color operator/(float f){
        return {r/f,g/f,b/f};
    }
    bool operator>(Color c){
        return (r+g+b>c.r+c.g+c.b);
    }
    bool operator<(Color c){
        return (r+g+b<c.r+c.g+c.b);
    }
};