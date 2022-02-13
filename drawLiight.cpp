#include <math.h>
#include "svpng.inc"
#include<stdlib.h> //rand() RAND_MAX

#define TWO_PI 6.28318530718f
#define MAX_STEP 10//光线步进采样次数
#define MAX_DISTANCE 2.0f//计算光走的距离
#define EPSILON 1e-6f//10^-6像素点距离形状的
#define N 64    //每个像素的采样次数
#define W 512   //图像长宽像素
#define H 512
unsigned char img[W*H*3];

/*
SDF:signed distance field 有符号距离场
圆心为c(cx,cy)，半径为r的圆形SDF定义为Fcircle(x)=|*x-*c|-r;
有正负号，用于表示坐标x相对于场景形状的位置，>0说明在场景形状之外，距离为F(x)
同理，F(x)<0说明坐标x位于场景形状之内，=0时说明x刚好在形状边界上
*/
float circleSDF(float x,float y,float cx,float cy,float r){
    float ux=x-cx,uy=y-cy;
    return sqrtf(ux*ux+uy*uy)-r;
}

/*
光线追踪ray tracing用于求出光线与场景的最近点,然而需要为各种几何形状编写与光线相交的函数，过于复杂(可能还要提供相交点的发现normal vector)
光线步进ray marching/sphere tracing，场景只需要以SDF表示
*/
float trace(float x,float y,float dx,float dy){
    float t=0.0f;
    for(int i=0;i<MAX_STEP && t<MAX_DISTANCE;i++){
        float sd=circleSDF(x+dx*t,y+dy*t,0.5f,0.5f,0.08f);
        if(sd<EPSILON)
            return 2.0f;
        t+=sd;
    }
    return 0.0f;
}

//sample 每个像素向N个方向采样
//均匀采样、分层采样、分层抖动采样
float sample(float x,float y){
    float sum=0.0f;
    for(int i=0;i<N;i++){
        //rand()/RAND_MAX将随机数单位化0<randnum<1
        //float a=TWO_PI*rand()/RAND_MAX;//均匀采样：过于随机，噪点太多
        //float a=TWO_PI*i/N;
        float a=TWO_PI/N*(i+(float)rand()/RAND_MAX);
        sum += trace(x,y,cosf(a),sinf(a));
    }
    return sum/N;
}



int main(void){
    unsigned char*p=img;
    for(int height=0;height<H;height++){
        for(int width=0;width<W;width++,p+=3){
            p[0]=p[1]=p[2]=(int)(fminf(sample(
                (float)width/W,(float)height/H)*255.0f,255.0f));
        }
    }
    svpng(fopen("picture_basic.png","wb"),W,H,img,0);
    system("pause");
    return 0;
}