#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "../gui/driver/disp_driver.h"
#include "../gui/porting/lv_port_disp.h"
using namespace std;

void *printHello(void *thread){
    long tid;
    tid = (long)thread;
    cout << "Hello World! It's me, thread #" << tid << endl;
    pthread_exit(NULL);
}

void lv_app_init(void);
void lv_example_chart_1(void);
void *thread_lv_task_entry(void*);
void *thread_lv_tick_entry(void*);

int main(void){
    // pthread_t thread;
    // int ret;
    // long t = 666;
    // ret = pthread_create(&thread, NULL, printHello, (void *)t);
    // disp_driver_init();
    // lv_port_disp_init();

    // cout << "hello oecg!" << endl;
    // return 0;



    pthread_t id,id_tick,id_task;
    int ret;

    // LVGL 图形库初始化
    lv_init();
    // 显示接口初始化
    lv_port_disp_init();

    // test_set_px(0,480,0,360,0x000);
    // test_set_px(40,60,70,300,0xF800);
    // test_set_px(50,300,10,70,0x07E0);
    // test_set_px(60,350,80,360,0x001F);

    // 图形应用初始化
    // lv_app_init();
    lv_example_chart_1();

    // 图形库任务线程，用于图形库处理任务和事件
    ret = pthread_create(&id_task,NULL,thread_lv_task_entry,NULL);
    if(ret != 0)
    {
        printf("Create lv_task pthread error\n");
        exit(1);
    }

    // 图形库时基线程，用于为图形库提供时基
    ret = pthread_create(&id_tick,NULL,thread_lv_tick_entry,NULL);
    if(ret != 0)
    {
        printf("Create lv_tick pthread error\n");
        exit(1);
    }

    pthread_join(id_task,NULL);
    pthread_join(id_tick,NULL);

    exit(0);

}

void lv_example_chart_1(void)
{
    /*Create a chart*/
    lv_obj_t * chart;
    chart = lv_chart_create(lv_scr_act());
    lv_obj_set_size(chart, 200, 150);
    lv_obj_center(chart);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);   /*Show lines and points too*/

    /*Add two data series*/
    lv_chart_series_t * ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_series_t * ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_SECONDARY_Y);

    /*Set the next points on 'ser1'*/
    lv_chart_set_next_value(chart, ser1, 10);
    lv_chart_set_next_value(chart, ser1, 10);
    lv_chart_set_next_value(chart, ser1, 10);
    lv_chart_set_next_value(chart, ser1, 10);
    lv_chart_set_next_value(chart, ser1, 10);
    lv_chart_set_next_value(chart, ser1, 10);
    lv_chart_set_next_value(chart, ser1, 10);
    lv_chart_set_next_value(chart, ser1, 30);
    lv_chart_set_next_value(chart, ser1, 70);
    lv_chart_set_next_value(chart, ser1, 90);

    /*Directly set points on 'ser2'*/
    ser2->y_points[0] = 90;
    ser2->y_points[1] = 70;
    ser2->y_points[2] = 65;
    ser2->y_points[3] = 65;
    ser2->y_points[4] = 65;
    ser2->y_points[5] = 65;
    ser2->y_points[6] = 65;
    ser2->y_points[7] = 65;
    ser2->y_points[8] = 65;
    ser2->y_points[9] = 65;

    lv_chart_refresh(chart); /*Required after direct set*/
}

void lv_app_init(void)
{
    /* use a pretty small demo for monochrome displays */
    /* Get the current screen  */
    lv_obj_t * scr = lv_disp_get_scr_act(NULL);

    /*Create a Label on the currently active screen*/
    lv_obj_t * label1 =  lv_label_create(scr);

    /*Modify the Label's text*/
    lv_label_set_text(label1, "Hello\nworld");

    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
}

// 时基线程
void *thread_lv_tick_entry(void *arg)
{
    while(1) 
    {
        lv_tick_inc(1); 
        usleep(1000);     /* takes microseconds */
    }
    return 0;
}

// 任务线程
void *thread_lv_task_entry(void *arg)
{
    while(1) 
    {
        lv_task_handler();
        usleep(5*1000);     /* takes microseconds */
    }
    return 0;
}