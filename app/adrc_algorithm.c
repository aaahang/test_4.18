#include <stdio.h>


// 定义ADRC控制器的参数结构体
typedef struct {
    float b0;  // 系统增益估计
    float beta1;  // 扩展状态观测器的参数
    float beta2;
    float beta3;
    float z1;  // 状态观测值
    float z2;
    float z3;
    float u;  // 控制输入
    float e;  // 误差
    float r;  // 设定值
    float y;  // 系统输出
    float h;  // 采样周期
} ADRC_Controller;
void ADRC_Init(ADRC_Controller *adrc, float b0, float beta1, float beta2, float beta3, float h);
void ADRC_Update(ADRC_Controller *adrc, float setpoint, float measurement);
ADRC_Controller * adrc_algorithm(int tem ,int hum)
{

    static  ADRC_Controller adrc;
    static int first_call = 0;
    float b0 = 1.0f;
    float beta1 = 1.0f;
    float beta2 = 0.1f;
    float beta3 = 0.01f;
    float h = 0.01f;
    if (first_call ==0)
    {
        ADRC_Init(&adrc, b0, beta1, beta2, beta3, h);
        first_call  =1 ;
    }

    ADRC_Update(&adrc, 20, tem);
    return &adrc;
}




// 初始化ADRC控制器
void ADRC_Init(ADRC_Controller *adrc, float b0, float beta1, float beta2, float beta3, float h) {
    adrc->b0 = b0;
    adrc->beta1 = beta1;
    adrc->beta2 = beta2;
    adrc->beta3 = beta3;
    adrc->z1 = 0.0f;
    adrc->z2 = 0.0f;
    adrc->z3 = 0.0f;
    adrc->u = 0.0f;
    adrc->e = 0.0f;
    adrc->r = 0.0f;
    adrc->y = 0.0f;
    adrc->h = h;
}

// 扩展状态观测器（ESO）更新
void ESO_Update(ADRC_Controller *adrc) {
    float e1 = adrc->z1 - adrc->y;
    adrc->z1 += adrc->h * (adrc->z2 - adrc->beta1 * e1);
    adrc->z2 += adrc->h * (adrc->z3 - adrc->beta2 * e1 + adrc->u);
    adrc->z3 += adrc->h * (-adrc->beta3 * e1);
}

// 计算控制输入
void ADRC_Control(ADRC_Controller *adrc) {
    adrc->e = adrc->r - adrc->y;
    float u0 = adrc->beta1 * adrc->e - adrc->z2;
    adrc->u = u0 / adrc->b0;
}

// 更新ADRC控制器
void ADRC_Update(ADRC_Controller *adrc, float setpoint, float measurement) {
    adrc->r = setpoint;
    adrc->y = measurement;

    // 更新ESO
    ESO_Update(adrc);

    // 计算控制输入
    ADRC_Control(adrc);
}

