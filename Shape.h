#pragma once
#include"basic.h"

#define EPSILON 1e-6f

class Shape{
public:
    Shape(){};
    ~Shape(){};
    //判断是否相交
    virtual bool Intersect(Point p,Vector v){
        return false;
    }
    //判断是否相交并求交点
    virtual bool Intersect(Point p,Vector d,Point &inter){
        return false;
    }
    //判断是否在Shape内部
    virtual bool IsInside(Point p){
        return false;
    }
    //获取边界上的点所在位置的法向
    virtual Vector GetNormal(Point p){
        return {0.f,0.f};
    }
    //是否在边界上
    virtual bool IsOnBoundary(Point p){
        return false;
    }
    //判断是否在包围盒内部
    virtual bool Contained(float left,float reght,float up,float down){
        return false;
    }
};

//半平面:m_a*x+m_b*y+m_c>0,法线方向m_normal
class Line : public Shape{
protected:
    float m_a,m_b,m_c;
    Vector m_normal;
    //计算点p出发，d方向的直线与Line的交点inter
    void CalcIntersect(Point p,Vector d,Point &inter){
        inter.x=(m_b*d.y*p.x-m_b*d.x*p.y-m_c*d.x)/(m_a*d.x+m_b*d.y);
        if(d.x) inter.y=(inter.x-p.x)*d.y/d.x+p.y;
        else inter.y=(-m_c-m_a*inter.x)/m_b;
    }
public:
    Line(float a,float b,float c):m_a(a),m_b(b),m_c(c){
        m_normal={-a,-b};
        m_normal=m_normal.normalize();
    }
    //根据p1、p2确定一条直线，确定点inter是否在线的上方
    //即对于inter(x,y)有Ax+By+c>0
    Line(Point p1,Point p2,Point inter){
        m_a=p2.y-p1.y;
        m_b=p1.x-p2.x;
        m_c=p2.x*p1.y-p1.x*p2.y;
        if(!IsInside(inter)){
            m_a=-m_a;
            m_b=-m_b;
            m_c=-m_c;
        }
        m_normal={-m_a,-m_b};
        m_normal=m_normal.normalize();
    }
    //点在半平面内
    bool IsInside(Point p){
        return m_a*p.x+m_b*p.y+m_c >= 0;
    }
    
    Vector GetNormal(Point p){
        return m_normal;
    }
    //点在半平面的边界上,即距离<epsilon
    bool IsOnBoundary(Point p){
        return fabs(m_a*p.x+m_b*p.y+m_c)<=EPSILON;
    }
    //求点p出发的d方向的直线与Line的交点
    bool Intersect(Point p,Vector d){
        if(IsInside(p)) return true;
        if(d*m_normal<0){
            Point inter;
            CalcIntersect(p,d,inter);
            //if(inter.IsValid())
            return true;
        }
        return false;
    }

    bool Intersect(Point p,Vector d,Point&inter){
        if(IsInside(p)){
            CalcIntersect(p,d,inter);
            if((inter-p)*d<0)
                inter ={-1.f,-1.f};
            return true;
        }
        if(d*m_normal<0){
            CalcIntersect(p,d,inter);
            return true;
        }
        return false;
    }
};

class Circle:public Shape{
protected:
    Point m_o;//central point
    float m_r;//radius
public:
    //圆心坐标p，半径r
    Circle(Point p,float r):m_o(p),m_r(r){};
    Point GetCenter(){return m_o;}
    float GetRadius(){return m_r;}
    bool IsInside(Point p){
        return (m_o-p).len() <= m_r;
    }
    bool IsBoundary(Point p){
        return fabs((m_o-p).len()-m_r) <= EPSILON;
    }
    //圆心指向点p的单位向量
    Vector GetNormal(Point p){
        return (p-m_o).normalize();
    }
    //点p出发的方向d的直线与circle是否相交
    bool Intersect(Point p,Vector d){
        if(IsInside(p)) return true;    //在圆内必然相交
        float proj = (m_o-p).len();     //向量po在射线上垂足的距离
        if(proj <= 0) return false;     //反向射线
        Point foot = p+d*proj;          //垂足位置
        float distance=(m_o-foot).len();//圆心到垂足的距离
        return distance < m_r;
    }
    bool Intersect(Point p,Vector d,Point&inter){
        if(IsInside(p)){
            float proj=(m_o-p)*d;           
            Point foot=p+d*proj;            
            float dis1=(m_o-foot).len();
            float dis2=sqrt(m_r*m_r-dis1*dis1);
            inter = foot+d*dis2;
            //inter=p ??
            return true;
        }
        float proj=(m_o-p)*d;       //向量po在射线上垂足的距离
        if(proj<=0) return false;   //反向射线
        Point foot=p+d*proj;        //垂足位置
        float dis1=(m_o-foot).len();//圆心到垂足距离
        if(dis1>m_r) return false;
        float dis2=sqrt(m_r*m_r-dis1*dis1);
        inter = foot-d*dis2;
        return true;
    }
    //是否在包围盒内部
    bool Contained(float left,float right,float up,float down){
        return (m_o.x-m_r > left) && (m_o.x+m_r < right) 
            && (m_o.y-m_r > up) && (m_o.y+m_r < down);
    }
};
//形状的并集
class ShapeUnion : public Shape{
private:
    Shape*m_shape1;
    Shape*m_shape2;
public:
    ShapeUnion(Shape*shape1,Shape*shape2){
        m_shape1=shape1;
        m_shape2=shape2;
    }
    ~ShapeUnion(){
        delete m_shape1;
        delete m_shape2;
    }
    bool IsInside(Point p){
        return m_shape1->IsInside(p) || m_shape2->IsInside(p);
    }
    bool IsOnBoundary(Point p){
        return m_shape1->IsOnBoundary(p) || m_shape2->IsOnBoundary(p);
    }
    Vector GetNormal(Point p){
        if(m_shape1->IsOnBoundary(p) && m_shape2->IsOnBoundary(p))
            return (m_shape1->GetNormal(p)+m_shape2->GetNormal(p))/2.f;
        if(m_shape1->IsOnBoundary(p))
            return m_shape1->GetNormal(p);
        if(m_shape2->IsOnBoundary(p))
            return m_shape2->GetNormal(p);
        else
            return {0.f,1.f};//???
    }
    bool Intersect(Point p,Vector d){
        Point inter1,inter2;
        if(!(m_shape1->Intersect(p,d,inter1) || m_shape2->Intersect(p,d,inter2)))
            return false;
        else 
            return true;
    }
    bool Intersect(Point p,Vector d,Point&inter){
        Point inter1,inter2;
        bool res1=m_shape1->Intersect(p,d,inter1);
        bool res2=m_shape2->Intersect(p,d,inter2);
        if(!(res1 || res2))
            return false;
        if(!res1)
            inter=inter2;
        else if(!res2)
            inter=inter1;
        else //选取距离点p最近的交点
            inter=(inter1-p).len() > (inter2-p).len() ? inter2 : inter1;
        return true;
    }
};
//形状的交集
class ShapeIntersect:public Shape{
private:
    Shape*m_shape1;
    Shape*m_shape2;
public:
    ShapeIntersect(Shape*shape1,Shape*shape2){
        m_shape1=shape1;m_shape2=shape2;
    }
    ~ShapeIntersect(){
        delete m_shape1;
        delete m_shape2;
    }
    bool IsInside(Point p){
        return m_shape1->IsInside(p) && m_shape2->IsInside(p);
    }
    bool IsOnboundary(Point p){
        return m_shape1->IsOnBoundary(p) || m_shape2->IsOnBoundary(p);
    }
    Vector GetNormal(Point p){
        if(m_shape1->IsOnBoundary(p)&&m_shape2->IsOnBoundary(p))
            return (m_shape1->GetNormal(p)+m_shape2->GetNormal(p))/2.f;
        if(m_shape1->IsOnBoundary(p))
            return m_shape1->GetNormal(p);
        if(m_shape2->IsOnBoundary(p))
            return m_shape2->GetNormal(p);
        else
            return {0.f,1.f};
    }
    bool Intersect(Point p,Vector d){
        Point inter1,inter2;
        if(!(m_shape1->Intersect(p,d,inter1)&&m_shape2->Intersect(p,d,inter2)))
            return false;
        else    //
            return m_shape2->IsInside(inter1) || m_shape1->IsInside(inter2);
    }
    bool Intersect(Point p,Vector d,Point&inter){
        Point inter1,inter2;
        if(!(m_shape1->Intersect(p,d,inter1)&&m_shape2->Intersect(p,d,inter2)))
            return false;
        if(m_shape2->IsInside(inter1)&&m_shape1->IsInside(inter2))
            inter=(inter1-p).len() > (inter2-p).len() ? inter2 : inter1;
        else if(m_shape2->IsInside(inter1))
            inter = inter1;
        else if(m_shape1->IsInside(inter2))
            inter = inter2;
        else
            return false;
        return true;
    }
};