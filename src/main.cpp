#include <raylib-cpp.hpp>
#include "Ray2d.h"
#include <vector>

namespace windowsos
{
    #define WIN32_LEAN_AND_MEAN
    #define NOUSER
    #define NOSERVICE
    #define NOIME
    #include <windows.h>
}

bool Debug = false;

Color ColorLerp(Color aa, Color bb, float t)
{
    float r = Clamp(Lerp(aa.r, bb.r, t), 0.0f, 255.0f);
    float g = Clamp(Lerp(aa.g, bb.g, t), 0.0f, 255.0f);
    float b = Clamp(Lerp(aa.b, bb.b, t), 0.0f, 255.0f);
    float a = Clamp(Lerp(aa.a, bb.a, t), 0.0f, 255.0f);
    return (Color){(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a};
}

float NormalizeRange(float min, float max, float t)
{
    return (t - min) / (max - min);
}

Color WavelengthToColor(float wavelength)
{
    Color result = {0,0,0,0};

    auto SubRangeLerp = [&](float min, float max, Color a, Color b)
    {
        if(wavelength >= min && wavelength < max)
        {
            result = ColorLerp(a, b, NormalizeRange(min, max, wavelength));
        }
    };

    SubRangeLerp(380.0f, 440.0f, {255, 0, 255, 255}, {0, 0, 255, 255});
    SubRangeLerp(440.0f, 490.0f, {0, 0, 255, 255}, {0, 255, 255, 255});
    SubRangeLerp(490.0f, 510.0f, {0, 255, 255, 255}, {0, 255, 0, 255});
    SubRangeLerp(510.0f, 580.0f, {0, 255, 0, 255}, {255, 255, 0, 255});
    SubRangeLerp(580.0f, 645.0f, {255, 255, 0, 255}, {255, 0, 0, 255});
    SubRangeLerp(645.0f, 780.0f, {255, 0, 0, 255}, {255, 0, 0, 255});

    if(wavelength < 420.0f)result = Fade(result, Lerp(0.0f, 1.0f, NormalizeRange(380.0f, 420.0f, wavelength)));
    if(wavelength >= 420.0f && wavelength <= 700.0f)result.a = 255;
    if(wavelength > 700.0f)result = Fade(result, Lerp(1.0f, 0.0f, NormalizeRange(700.0f, 780.0f, wavelength)));

    return result;
}

struct Lens
{
    Vector2 position;
    float rotation;
    float scale;
    float refractive_index;
    float albedo;
    Vector2 GetP1()
    {
        return Vector2Add(position, Vector2Scale({cosf(rotation*DEG2RAD), sinf(rotation*DEG2RAD)}, scale));
    }
    Vector2 GetP2()
    {
        return Vector2Add(position, Vector2Scale({cosf((-120.0f+rotation)*DEG2RAD), sinf((-120.0f+rotation)*DEG2RAD)}, scale));
    }
    Vector2 GetP3()
    {
        return Vector2Add(position, Vector2Scale({cosf((-240.0f+rotation)*DEG2RAD), sinf((-240.0f+rotation)*DEG2RAD)}, scale));
    }
    void Draw()
    {
        DrawTriangle(GetP1(), GetP2(), GetP3(), {200, 220, 255, 90});
    }
    Ray2DCollision GetRayCollision(Ray2D r)
    {
        Ray2DCollision c1 = RayLineSegmentIntersection(r, GetP1(), GetP2());
        Ray2DCollision c2 = RayLineSegmentIntersection(r, GetP2(), GetP3());
        Ray2DCollision c3 = RayLineSegmentIntersection(r, GetP3(), GetP1());

        //if(!c1.hit && !c2.hit && !c3.hit)return NO_RAY_COLLISION;

        if(c1.distance <= c2.distance && c1.distance <= c3.distance)return c1;
        if(c2.distance <= c1.distance && c2.distance <= c3.distance)return c2;
        if(c3.distance <= c1.distance && c3.distance <= c2.distance)return c3;

        return NO_RAY_COLLISION;
    }
    bool MouseCollision()
    {
        return CheckCollisionPointTriangle(GetMousePosition(), GetP1(), GetP2(), GetP3());
    }
};

struct Light_Beam
{
    float Wavelength;//In nanometers
    float Brightness;
    Ray2D Ray;
    int num_collisions = 0;
    void Draw(std::vector<Lens> &lenses)
    {
        float current_bright = Brightness;
        Ray2D current_ray = Ray;
        num_collisions = 0;

        while (current_bright > 0.01f)
        {
            Ray2DCollision c = NO_RAY_COLLISION;
            int lens_index = 0;
            for (unsigned i = 0; i < lenses.size(); i++)
            {
                Ray2DCollision cc = lenses.at(i).GetRayCollision(current_ray);
                if(c.distance > cc.distance)
                {
                    c = cc;
                    lens_index = i;
                }
            }
            if(!c.hit)
            {
                DrawLineEx(current_ray.origin, Vector2Add(current_ray.origin, Vector2Scale(current_ray.direction, 2000.0f)), 4.0f, Fade(WavelengthToColor(Wavelength), current_bright));
                break;
            }
            else
            {
                BeginBlendMode(BLEND_ADDITIVE);
                DrawLineEx(current_ray.origin, c.point, 4.0f, Fade(WavelengthToColor(Wavelength), current_bright));
                EndBlendMode();
                if(Debug)
                {
                    DrawLineEx(c.point, Vector2Add(c.point, Vector2Scale(c.normal, 100.0f)), 1.2f, RED);
                    DrawCircleV(c.point, 6.0f, BLUE);
                }
                float effective_refractive_index = lenses.at(lens_index).refractive_index*(Wavelength/400.0f);
                if(Vector2DotProduct(c.normal, current_ray.direction) < 0.0f)effective_refractive_index = 1.0f/effective_refractive_index;
                current_ray.Refract(c, lenses.at(lens_index).refractive_index*(Wavelength/400.0f));
                current_bright *= lenses.at(lens_index).albedo;
                num_collisions++;
            }
        }
    }
};

int WINAPI WinMain(windowsos::HINSTANCE hInstance, windowsos::HINSTANCE hPrevInstance, windowsos::LPSTR lpCmdLine, int nCmdShow) {

    raylib::Window w(1200, 675, "Optic sim");
    w.SetState(FLAG_WINDOW_RESIZABLE);
    w.SetMinSize(800, 450);
    
    SetTargetFPS(60);

    std::vector<Light_Beam> spectrum;
    unsigned int spectrum_reso = 24;
    for (unsigned int i = 0; i < spectrum_reso; i++)
    {
        spectrum.push_back({Lerp(380.0f, 780.0f, (float)i / (float)spectrum_reso), 0.75f, {200.0f, GetScreenHeight()/2.0f}});
    }

    std::vector<Lens> lenses;

    while (!w.ShouldClose())
    {
        if(IsKeyPressed(KEY_F3))Debug = !Debug;

        if(IsKeyDown(KEY_LEFT_SHIFT))
        {
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                lenses.push_back({GetMousePosition(), 0.0f, 100.0f, 1.5f, 0.8f});
            }
        }
        else
        {
            bool hovered_over_lens = false;
            unsigned int lens_index = 0;
            for (unsigned int i = 0; i < lenses.size(); i++)
            {
                if(lenses.at(i).MouseCollision())
                {
                    hovered_over_lens = true;
                    lens_index = i;
                }
            }

            if(!hovered_over_lens)
            {
                for (unsigned int i = 0; i < spectrum.size(); i++)
                {
                    if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT))spectrum.at(i).Ray.direction = Vector2Normalize(Vector2Subtract(GetMousePosition(), spectrum.at(i).Ray.origin));
                    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))spectrum.at(i).Ray.origin = GetMousePosition();
                }
            }
            else
            {
                if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))lenses.at(lens_index).position = GetMousePosition();
                if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
                {
                    Vector2 mouse_vector = Vector2Normalize(Vector2Subtract(GetMousePosition(), lenses.at(lens_index).position));
                    lenses.at(lens_index).rotation = -Vector2Angle(mouse_vector, {0.0f, -1.0f})*RAD2DEG-90.0f;
                }
                if(IsKeyDown(KEY_LEFT_CONTROL)){lenses.at(lens_index).refractive_index += GetMouseWheelMove() * 0.1f;}
                else{lenses.at(lens_index).scale += GetMouseWheelMove() * 2.0f;}
                if(IsKeyPressed(KEY_DELETE))
                {
                    lenses.erase(lenses.begin()+lens_index);
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for (unsigned int i = 0; i < lenses.size(); i++)
        {
            lenses.at(i).Draw();
        }

        for (unsigned int i = 0; i < spectrum.size(); i++)
        {
            spectrum.at(i).Draw(lenses);
        }

        if(Debug)
        {
            DrawText(TextFormat("Lenses = %i", lenses.size()), 20, 20, 20, WHITE);
        }

        EndDrawing();
    }
    return 0;
}