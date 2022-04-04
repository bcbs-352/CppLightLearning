#pragma once
#include <stdlib.h>
#include "Shape.h"
#include <list>
#include <iostream>
using namespace std;
#define MAX_DEPTH 8
#define N 32
#define REFRACT 3
#define TWO_PI 6.28318530718f
#define BIAS 1e-4f //步进距离
#define USE_QUADTREE false
#define DEBUG false
#define SAFE_DELETE(p) \
    do                 \
    {                  \
        delete (p);    \
        (p) = NULL;    \
    } while (false)
void drawLine(Point p1, Point p2);

class Entity
{
protected:
    Shape *m_shape;
    Color m_emissive;
    float m_reflectivity;
    float m_refractivity;                        //折射率+反射率<=1
    float *m_refract_index = new float[REFRACT]; //每种颜色都有不同的折射率
public:
    Entity(Shape *s, Color e, float re = 0.f, float ra = 0.f, float *ri = NULL) : m_shape(s), m_emissive(e), m_reflectivity(re), m_refractivity(ra)
    {
        if (ri)
            for (int i = 0; i < 3; i++)
                m_refract_index[i] = ri[i];
    }
    Entity(Shape *s, Color e, float re, float ra, float ri_min, float ri_max) : m_shape(s), m_emissive(e), m_reflectivity(re), m_refractivity(ra)
    {
        for (int i = 0; i < REFRACT; i++)
            m_refract_index[i] = (ri_max - ri_min) / (REFRACT - 1) * i + ri_min; //折射率[min,max]间分为3份
    }
    ~Entity()
    {
        SAFE_DELETE(m_shape);
    }
    Shape *GetShape() { return m_shape; }
    //自身颜色
    Color GetEmissive() { return m_emissive; }
    //反射率
    float GetReflectivity() { return m_reflectivity; }
    //折射率
    float GetRefractivity() { return m_refractivity; }
    //判断相交并计算交点inter
    virtual bool Intersect(Point p, Vector d, Point &inter)
    {
        return m_shape->Intersect(p, d, inter);
    }
    float GetRefractIndex(int index) { return m_refract_index[index]; }
    //在包围盒里
    bool Contained(float left, float right, float up, float down)
    {
        return m_shape->Contained(left, right, up, down);
    }
};

//聚光灯
class SpotLight : public Entity
{
protected:
    Vector m_dir; //聚光灯主方向
    float m_cosa; //聚光灯角度范围的cos
public:
    SpotLight(Shape *s, Color e, float re = 0.f, float ra = 0.f, float *ri = NULL,
              Vector dir = {0.f, 1.f}, float a = 0.03f) : Entity(s, e, re, ra, ri), m_dir(dir)
    {
        m_cosa = cos(a);
    }
    bool Intersect(Point p, Vector d, Point &inter)
    {
        if (d * (-m_dir) < m_cosa) //预过滤角度方向在照射范围外的光线
            return false;
        else
            return m_shape->Intersect(p, d, inter);
    }
};

class Scene
{
protected:
    Color rainbow[7] = {
        {1.f, 0.f, 0.f}, {1.f, 0.65f, 0.f}, {1.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.5f, 1.f}, {0.f, 0.f, 1.f}, {0.55f, 0.f, 1.f}};
    list<Entity *> m_entities;

public:
    Scene(list<Entity *> entities) : m_entities(entities)
    {
        // m_entityTree = new QuadTree<Entity>(entities);
    }
    ~Scene()
    {
        for (auto ent : m_entities)
            SAFE_DELETE(ent);
    }
    list<Entity *> GetEntities() { return m_entities; }

    Color GetRefractColor(int index)
    {
        float idxf = index * 6.f / (REFRACT - 1);
        float idx1 = floor(idxf);
        float idx2 = ceil(idxf);
        float gap1 = idxf - idx1;
        float gap2 = idxf - idx1;
        Color color;
        if (idx1 == idx2)
        {
            color = rainbow[(int)idx2];
        }
        else
        {
            color = {
                rainbow[(int)idx1].r * gap2 + rainbow[(int)idx2].r * gap1,
                rainbow[(int)idx1].g * gap2 + rainbow[(int)idx2].g * gap1,
                rainbow[(int)idx1].b * gap2 + rainbow[(int)idx2].b * gap1};
        }
        return color;
    }

    //反射点inter从方向reflect获取的光照颜色
    Color Reflect(Entity *ent, Point inter, Vector d, Vector normal, int color_index, int depth)
    {
        if (depth > MAX_DEPTH || ent->GetReflectivity() == 0.f)
            return {0.f, 0.f, 0.f};
        Vector reflect = d.reflect(normal);
        return GetColor(inter + reflect * BIAS, reflect, color_index, depth) * ent->GetReflectivity();
    }

    Color Refract(Entity *ent, Point inter, Vector d, Vector normal, int color_index, int depth)
    {
        if (depth > MAX_DEPTH || ent->GetRefractivity() == 0.f)
            return {0.f, 0.f, 0.f};
        float idotn = d * normal;
        float ri = ent->GetRefractIndex(color_index);
        float k, a;
        if (idotn > 0.f)
        {
            k = 1.f - ri * ri * (1.f - idotn * idotn);
            if (k < 0.f) //说明全反射，不再采样
                return {0.f, 0.f, 0.f};
            a = ri * idotn - sqrtf(k);
        }
        else
        {
            ri = 1.f / ri;
            k = 1.f - ri * ri * (1.f - idotn * idotn);
            a = ri * idotn + sqrtf(k);
        }
        Vector refract = d * ri - normal * a;
        return GetColor(inter + refract * BIAS, refract, color_index, depth) * ent->GetRefractivity();
    }

    //获取p点从d方向收到的emissive
    Color GetColor(Point p, Vector d, int color_index, int depth = 0)
    {
        Color trace_emissive{0.f, 0.f, 0.f};
        float distance = 10.f;
        Entity *ent_near = NULL;
        Point inter;

        for (auto ent : m_entities)
        {
            Point tmp_inter;
            if (ent->Intersect(p, d, tmp_inter))
            {
                float new_distance = (tmp_inter - p).len(); //剩余光程
                if (distance > new_distance)
                {
                    ent_near = ent;
                    distance = new_distance;
                    inter = tmp_inter;
                }
            }
        }

        if (ent_near)
        {
            if (DEBUG) // debug
                drawLine(p, inter);
            Vector normal = ent_near->GetShape()->GetNormal(inter);
            Color reflect = Reflect(ent_near, inter, d, normal, color_index, depth + 1);
            Color refract = {0.f, 0.f, 0.f};
            if (color_index >= REFRACT)
            {
                for (int i = 0; i < REFRACT; i++)
                    refract = refract + Refract(ent_near, inter, d, normal, i, depth + 1) * GetRefractColor(i);
                refract = refract * 2.f / REFRACT;
            }
            else
                refract = Refract(ent_near, inter, d, normal, color_index, depth + 1);
            return ent_near->GetEmissive() + reflect + refract;
        }
        else
            return {0.0f, 0.0f, 0.0f};
    }

    Color Sample(Point p)
    {
        Color sum{0.0f, 0.0f, 0.0f};
        Color tmp[N];

#pragma omp parallel for
        for (int i = 0; i < N; i++)
        {
            float a = TWO_PI * (i + (float)rand() / RAND_MAX) / N; //采样角度
            tmp[i] = GetColor(p, {cosf(a), sinf(a)}, REFRACT);
        }
        for (int i = 0; i < N; i++)
        {
            sum = sum + tmp[i];
        }
        return sum / N;
    }
    Color GetBaseColor(Point p)
    {
        for (auto ent : m_entities)
        {
            if (ent->GetShape()->IsInside(p))
                // cout<<ent->GetEmissive().r<<ent->GetEmissive().g<<endl;
                return ent->GetEmissive();
        }
        return {0.f, 0.f, 0.f};
    }
};