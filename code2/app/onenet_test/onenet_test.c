#include <stdio.h>
#include <unistd.h>
#include "MQTTClient.h"
#include "onenet.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"



//此处为添加的头文件

#include "dht11.h"
#include "hi_time.h"
// #define	DHT11_DQ_IN  	Gpio_Input_Action(); //读取GPIO输入的状态
IotGpioValue DHT11_DQ_IN;
IotGpioValue levelold;//IO输出状况


#include "hi_io.h"



#define ONENET_INFO_DEVID "955854197"
#define ONENET_INFO_AUTH "hi3861"
#define ONENET_INFO_APIKEY "gryM5amudrw0JUnim5SljOa16rY="
#define ONENET_INFO_PROID "525664"
#define ONENET_MASTER_APIKEY "2JLl16aIkSSkLuRW9sv0=6MkWgI="


#define LED_TEST_GPIO 9

extern int rand(void);

//添加的读取温湿度数据的代码

//得到输入信息
// u8 GPIOGETINPUT(unsigned int id,IotGpioValue *val)
// {
//     IoTGpioGetInputVal(id,val);
//     return *val;
// }

/****************************************
设置端口为输出
*****************************************/
void DHT11_IO_OUT(void)
{
    //设置GPIO_11为输出模式
    
    IoTGpioSetDir(DHT11_GPIO, IOT_GPIO_DIR_OUT);
}

/****************************************
设置端口为输入
*****************************************/
void DHT11_IO_IN(void)
{
    //设置为输入模式
    IoTGpioSetDir(DHT11_GPIO, IOT_GPIO_DIR_IN);
    //设置为浮空输入
    hi_io_set_pull(DHT11_GPIO, HI_IO_PULL_NONE);
}


//复位DHT11
void DHT11_Rst(void)	   
{                
    //设置端口为输出模式
	DHT11_IO_OUT(); 	//SET OUTPUT
    //设置为低电平
    DHT11_DQ_OUT_Low; 	//拉低DQ
    // GpioGetOutputVal(DHT11_GPIO,&levelold);
    // printf("out:%d\r\n",levelold);
    hi_udelay(20000);//拉低至少18ms

    //设置为高电平
    DHT11_DQ_OUT_High; 	//DQ=1 
	hi_udelay(35);     	//主机拉高20~40us
  //  GpioGetOutputVal(DHT11_GPIO,&levelold);
  //   printf("out:%d\r\n",levelold);
    printf("DHT11 Rest Successful\r\n");
}


//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在
u8 DHT11_Check(void) 	   
{   
	u8 retry=0;
	DHT11_IO_IN();//SET INPUT	 
    while (IoTGpioGetInputVal(DHT11_GPIO,&DHT11_DQ_IN)&&retry<100)//DHT11会拉低40~80us
	{
		retry++;
		hi_udelay(1);
	};	 
	if(retry>=100)return 1;
	else retry=0;
    
    while ((!IoTGpioGetInputVal(DHT11_GPIO,&DHT11_DQ_IN))&&retry<100)//DHT11拉低后会再次拉高40~80us
	{
		retry++;
		hi_udelay(1);
	};
	if(retry>=100)return 1;	   
	return 0;
}
//从DHT11读取一个位
//返回值：1/0
u8 DHT11_Read_Bit(void) 			 
{
 	int retry=0;
    while(IoTGpioGetInputVal(DHT11_GPIO,&DHT11_DQ_IN)&&retry<100){//等待变为低电平
        retry++;
        hi_udelay(1);
    }
    retry=0;
    while((!IoTGpioGetInputVal(DHT11_GPIO,&DHT11_DQ_IN))&&retry<100){//等待变高电平
        retry++;
        hi_udelay(1);
    }
    hi_udelay(40);//等待40us	//用于判断高低电平，即数据1或0
    if(IoTGpioGetInputVal(DHT11_GPIO,&DHT11_DQ_IN))return 1; else return 0;
}
//从DHT11读取一个字节
//返回值：读到的数据
u8 DHT11_Read_Byte(void)    
{        
    u8 i,dat;
    dat=0;
	for (i=0;i<8;i++) 
	{
   		dat<<=1; 
	    dat|=DHT11_Read_Bit();
    }			
    return dat;
}
//从DHT11读取一次数据
//temp:温度值(范围:0~50°)
//humi:湿度值(范围:20%~90%)
//返回值：0,正常;1,读取失败
u8 DHT11_Read_Data(u8 *temp,u8 *humi)    
{        
 	u8 buf[5]={ 0 };
	u8 i;
	DHT11_Rst();
	if(DHT11_Check()==0)
	{
		for(i=0;i<5;i++)//读取40位数据
		{
			buf[i]=DHT11_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])//数据校验
		{
			*humi=buf[0];
			*temp=buf[2];
            printf("校验成功！");
		}
	}else return 1;
	return 0;	    
}
//初始化DHT11的IO口 DQ 同时检测DHT11的存在
//返回1:不存在
//返回0:存在    	 
u8 DHT11_Init(void)
{	 
	//初始化GPIO
    IoTGpioInit(DHT11_GPIO);
    printf("INit successful");
    //设置GPIO_2的复用功能为普通GPIO,cunyi
    //hi_io_set_func(HI_IO_NAME_GPIO_2, HI_IO_FUNC_GPIO_2_GPIO);
    //设置GPIO_2为输出模式
    //IoTGpioSetDir(2, IOT_GPIO_DIR_OUT);
	//设置GPIO_2输出高电平点亮LED灯  IOT_GPIO_IN
    //IoTGpioSetOutputVal(2, 1);
    //设置GPIO_9的复用功能为普通GPIO
	hi_io_set_func(DHT11_GPIO, HI_IO_FUNC_GPIO_11_GPIO);
    printf("复用设置完毕");
    //设置GPIO_9为输出模式
    IoTGpioSetDir(DHT11_GPIO, IOT_GPIO_DIR_OUT);
	printf("输出模式设置完毕");
         //设置GPIO_9输出高电平
    IoTGpioSetOutputVal(DHT11_GPIO, 1);
    printf("输出高电平");	    
	DHT11_Rst();  //复位DHT11
    printf("复位完毕！");
    printf("DHT11_Check存在吗,0存在");
	return DHT11_Check();//等待DHT11的回应
} 


static void DHT11_Task(void)
{
    u8  ledflag=0;
    u8 temperature=0;  	    
	u8 humidity=0;  
    
    DHT11_Init();
    // while(DHT11_Init())	//DHT11初始化	
	// {
	// 	printf("DHT11 Init Error!!\r\n");
    //     printf("")
    //     hi_udelay(1);
 	// 	//usleep(100000);
	// }		
    printf("DHT11 Init Successful!!");


    while (1)
    {
       if(DHT11_Read_Data(&temperature,&humidity)==0)	//读取温湿度值
        {   
            if((temperature!= 0)||(humidity!=0))
            {
                ledflag++;
                printf("Temperature = %d\r\n",temperature);
                printf("Humidity = %d\r\n",humidity);
                //将数据上传至onenet
                if (onenet_mqtt_upload_digit("temperature", temperature) < 0 || onenet_mqtt_upload_digit("humidity",humidity)<0)
                {
                    printf("upload has an error, stop uploading");
                    //break;
                }
            }
        }
        //延时100ms
        //IoTGpioSetOutputVal(DHT11_GPIO, ledflag%2);
        usleep(500000);
        //hi_udelay(1);
    }
}

static void DHT11ExampleEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "DHT11_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = 25;

    if (osThreadNew((osThreadFunc_t)DHT11_Task, NULL, &attr) == NULL)
    {
        printf("Falied to create DHT11_Task!\n");
    }
}




SYS_RUN(DHT11ExampleEntry);



//添加代码结束



void onenet_cmd_rsp_cb(uint8_t *recv_data, size_t recv_size, uint8_t **resp_data, size_t *resp_size)
{
    printf("recv data is %.*s\n", recv_size, recv_data);

    IoTGpioInit(LED_TEST_GPIO);
    IoTGpioSetDir(LED_TEST_GPIO, IOT_GPIO_DIR_OUT);

    cJSON *recvjson;
    recvjson = cJSON_Parse((const char*)recv_data);
    char *command = cJSON_GetObjectItem(recvjson,"ledSwitch")->valuestring;
    printf("%s\n",command);
    
    if (strcmp("ON",command)==0)//检测到接收数据中有对应字符串就执行
    {
        printf("打开ON\r\n");
        IoTGpioSetDir(LED_TEST_GPIO, 1);
    }
    else printf("无ON\r\n");
    if (strcmp("OFF",command)==0)
    {
        printf("关闭OFF\r\n");
        IoTGpioSetDir(LED_TEST_GPIO, 0);
        }
    else printf("无OFF\r\n");

    // if(strcmp("ON",(char*)(recv_data))==0)  //检测到接收数据中有对应的字符串就执行
    // {
    //     printf("ON\r\n");
    //     IoTGpioSetDir(LED_TEST_GPIO, 1);
    // }

    // if(strcmp("OFF",(char*)(recv_data))==0)
    // {
    //     printf("OFF\r\n");
    //     IoTGpioSetDir(LED_TEST_GPIO, 0); //设置GPIO_2输出低电平关闭灯
    //     printf("close ok");
    // }
   
    *resp_data = NULL;
    *resp_size = 0;
}



int onenet_test(void)
{
	
    device_info_init(ONENET_INFO_DEVID, ONENET_INFO_PROID, ONENET_INFO_AUTH, ONENET_INFO_APIKEY, ONENET_MASTER_APIKEY);
    onenet_mqtt_init();

    onenet_set_cmd_rsp_cb(onenet_cmd_rsp_cb);


    u8  ledflag=0;
    u8 temperature=0;  	    
	u8 humidity=0;  
    
    DHT11_Init();
    while (1)
    {
       if(DHT11_Read_Data(&temperature,&humidity)==0)	//读取温湿度值
        {   
            //将数据上传至onenet
            if (onenet_mqtt_upload_digit("temperature", temperature) < 0 || onenet_mqtt_upload_digit("humidity",humidity)<0)
            {
                printf("upload has an error, stop uploading");
                //break;
            }
        }
        //延时100ms
        //IoTGpioSetOutputVal(DHT11_GPIO, ledflag%2);
        usleep(500000);
        //hi_udelay(1);
    }

	// while (1)
    // {
    //     //用随机值进行温度传递测试
	// 	int value = 0;
    //     value = rand() % 100;

    //     if (onenet_mqtt_upload_digit("temperature", value) < 0)
    //     {
    //         printf("upload has an error, stop uploading");
    //         //break;
    //     }
    //     else
    //     {
    //         printf("buffer : {\"temperature\":%d} \r\n", value);
    //     }
    //     sleep(1);
    // }
	return 0;
}


//原始的onenet功能代码
// void onenet_cmd_rsp_cb(uint8_t *recv_data, size_t recv_size, uint8_t **resp_data, size_t *resp_size)
// {
//     printf("recv data is %.*s\n", recv_size, recv_data);

//     IoTGpioInit(LED_TEST_GPIO);
//     IoTGpioSetDir(LED_TEST_GPIO, IOT_GPIO_DIR_OUT);


//     if(strcmp("ON",(char*)(recv_data))==0)  //检测到接收数据中有对应的字符串就执行
//     {
//         printf("ON\r\n");
//         IoTGpioSetDir(LED_TEST_GPIO, 1);
//     }

//     if(strcmp("OFF",(char*)(recv_data))==0)
//     {
//         printf("OFF\r\n");
//         IoTGpioSetDir(LED_TEST_GPIO, 0); //设置GPIO_2输出低电平关闭灯
//         printf("close ok");
//     }
   
//     *resp_data = NULL;
//     *resp_size = 0;
// }


// int onenet_test(void)
// {
	
//     device_info_init(ONENET_INFO_DEVID, ONENET_INFO_PROID, ONENET_INFO_AUTH, ONENET_INFO_APIKEY, ONENET_MASTER_APIKEY);
//     onenet_mqtt_init();

//     onenet_set_cmd_rsp_cb(onenet_cmd_rsp_cb);



	// while (1)
    // {
    //     //用随机值进行温度传递测试
	// 	int value = 0;
    //     value = rand() % 100;

    //     if (onenet_mqtt_upload_digit("temperature", value) < 0)
    //     {
    //         printf("upload has an error, stop uploading");
    //         //break;
    //     }
    //     else
    //     {
    //         printf("buffer : {\"temperature\":%d} \r\n", value);
    //     }
    //     sleep(1);
    // }
// 	return 0;
// }