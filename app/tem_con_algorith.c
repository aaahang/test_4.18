#include "stdio.h"
#include "stdlib.h"
#include <math.h>
// 常量定义
#define A 17.27
#define B 237.7

// 流体属性 (假设水)
#define RHO 1000       // 密度 kg/m^3
#define MU 0.001       // 动力粘度 Pa·s
#define CP 4186        // 比热容 J/(kg·K)
#define K 0.6          // 导热系数 W/(m·K)
 
// 管道属性
#define D 0.05         // 管道直径 m
#define DP 0.002       // 管道壁厚 m
#define KP 401         // 铜导热系数 W/(m·K)
#define HO 10          // 管道外表面传热系数 W/(m²·K)

double calculate_dew_point(double Ta, double RH);
double calculate_surface_temperature(double Tl, double Ta, double v);
double tem_con_algorith(double Ta, double RH,double Tl, double v)
{ 
    return calculate_dew_point(Ta,RH) -calculate_surface_temperature(Tl,Ta,v);  
}

double calculate_dew_point(double Ta, double RH) {
    double gamma = (A * Ta) / (B + Ta) + log(RH / 100.0);
    double Td = (B * gamma) / (A - gamma);
    return Td;
}

double calculate_surface_temperature(double Tl, double Ta, double v) {
    double Re = (RHO * v * D) / MU;
    double Pr = (CP * MU) / K;
    double Nu = 0.023 * pow(Re, 0.8) * pow(Pr, 0.4);
    double hi = (Nu * K) / D;

    // 计算传热量 Q
    double Q = hi * M_PI * D * (Tl - Ta);

    // 通过铜管壁传热
    double Ts = Tl - Q * (DP / KP);

    return Ts;
}