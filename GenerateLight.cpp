#include"svpng.inc"
#include<math.h>
#include<iostream>
#include<fstream>
#include"basic.h"
#include"time.h"
#include<stdlib.h>
#include"Scene.h"
using std::initializer_list;
#define W 1024
#define H 1024

unsigned char img[W*H*3];
//用于截断画布外的线段，使p1,p2均落在画布内???
void validate(Point&p1,Point&p2){
    //if(!p1.IsValid()||!p2.IsValid()) return;
    if(p1.x<0.f)
        p1={0.f,(p1.y*p2.x-p2.y*p1.x)/(p2.x-p1.x)};
    if(p1.x>1.f)
        p1={1.f,(p1.y-p2.y+p1.x*p2.y-p2.x*p1.y)/(p1.x-p2.x)};
    if(p1.y<0.f)
        p1={(p1.x*p2.y-p2.x*p1.y)/(p2.y-p1.y),0.f};
    if(p1.y>1.f)
        p1={(p1.x-p2.x+p1.y*p2.x-p2.y*p1.x)/(p1.y-p2.y),1.f};
    if(!p2.IsValid())
        validate(p2,p1);
}
//画线函数  用于dubug
void drawLine(Point p1,Point p2){
    validate(p1,p2);
    //端点像素坐标
    int x0=p1.x*W;
    int y0=p1.y*H;
    int x1=p2.x*W;
    int y1=p2.y*H;
    int dx=abs(x1-x0);
    int sx=x0<x1 ? 1:-1;
    int dy=abs(y1-y0);
    int sy=y0<y1 ? 1:-1;
    int err=(dx>dy ? dx:dy)/2;
    //p1点处画红色
    while (*(img+(y0*W+x0)*3)=255,x0 != x1 || y0 != y1){
        int e2=err;
        if(e2 >-dx){err-=dy;x0+=sx;}
        if(e2 < dy){err+=dx;y0+=sy;}
    }
}
//生成多边形
Shape*GeneratePolygon(initializer_list<Point>points){
    Point sum={0.f,0.f};
    for(auto p:points){
        sum.x+=p.x;
        sum.y+=p.y;
    }
    Point center={sum.x/points.size(),sum.y/points.size()};//多边形坐标数值中心
    Shape*si=NULL;
    for(auto i=points.begin();i+1!=points.end();i++){
        Point p1= *i;
        Point p2=*(i+1);
        Line*l=new Line(p1,p2,center);
        if(si)
            si=new ShapeIntersect(si,l);
        else
            si=l;
    }
    si=new ShapeIntersect(si,new Line(*(points.begin()),*(points.end()-1),center));
    return si;
}

Scene*GenerateScene(){
    // Shape*circle=new Circle({0.5f,0.35f},0.1f);
    // Shape*triangle=GeneratePolygon({{0.9f,0.6f},{0.7f,0.6f},{0.7f,0.4f}});
    // Shape*quad1=GeneratePolygon({{0.3f,0.4f},{0.2f,0.4f},{0.2f,0.3f},{0.3f,0.3f}});
    // Shape*quad2=GeneratePolygon({{0.4f,0.6f},{0.3f,0.75f},{0.55f,0.75f},{0.55f,0.6f}});
    // Entity*e1=new Entity(circle,{1.1f,0.9f,0.0f},0.f);
    // Entity*e2=new Entity(triangle,{0.1f,0.9f,1.1f});
    // Entity*e3=new Entity(quad1,{0.05f,0.05f,0.9f},0.8f);
    // Entity*e4=new Entity(quad2,{0.05f,1.1f,0.05f},0.5f);
    // return new Scene({e1,e2,e3,e4});
    Shape*circle1=new Circle({0.35f,0.65f},0.15f);
    Shape*circle2=new Circle({0.7f,0.3f},0.11f);
    Shape*circle3=new Circle({0.66f,0.60f},0.08f);
    Entity*e1=new Entity(circle1,{0.9f,0.8f,0.1f},0.1f);
    Entity*e2=new Entity(circle2,{0.1f,0.2f,0.9f},0.9f);
    Entity*e3=new Entity(circle3,{0.6f,1.2f,0.6f},0.5f);
    return new Scene({e1,e2,e3});
}

int main(void){
    time_t a=time(NULL);
    int start_num=1;
    Scene* s = GenerateScene();
    unsigned char*p=img;
    for(int y=0;y<H;y++)
        for(int x=0;x<W;x++,p+=3){
            Color color=s->Sample({(float)x/W,(float)y/H});
            p[0]=(int)fminf(color.r*255.0f,255.0f);
            p[1]=(int)fminf(color.g*255.0f,255.0f);
            p[2]=(int)fminf(color.b*255.0f,255.0f);
        }
    //s->Sample({0.76f,0.16f});
    svpng(fopen("reflect.png","wb"),W,H,img,0);
    system("pause");
    time_t b=time(NULL);
}