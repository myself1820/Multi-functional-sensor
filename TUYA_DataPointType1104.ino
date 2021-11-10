/*
 * @FileName: DataPointType.ino
 * @Author: Tuya
 * @Email: 
 * @LastEditors: Tuya
 * @Date: 2021-04-19 14:31:52
 * @LastEditTime: 2021-04-28 19:47:36
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Description: 
 * @Github:https://github.com/tuya/tuya-wifi-mcu-sdk-arduino-library
 */

#include <TuyaWifi.h>
#include <SoftwareSerial.h>
//#include "DHT11.h"
#include "DHT.h"

TuyaWifi my_device;
SoftwareSerial DebugSerial(8,9);

#define DHTPIN 5  //温湿度传感器接口
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
/* Current LED status */
unsigned char led_state = 0;

#define battery_voltage_pin A0       //电压检测接口
#define light_sensor_pin A1       //光敏传感器接口
#define smoke_sensor_pin A2       //烟雾传感器模拟接口
#define gas_sensor_pin A3       //天然气传感器模拟接口
//SDA（A4）、 SCL{A5）， 接OLED 或者 红蓝色报警灯
#define red_alarm_LED A4       //红色报警灯
#define blue_alarm_LED A5      //蓝色报警灯

#define ringtone_key_pin 2       //报警铃声选择按键
#define muffling_key_pin 3       //消音开关
#define human_sensor_pin 4   //人体传感器接口
#define buzzer_pin 6         //蜂鸣器接口
#define wifi_key_pin 7       //无线配网开关 Connect network button pin 
#define alarm_key_pin 10     //报警开关按键
#define human_LED 11         //人体感应夜灯
#define alarm_key_LED 12      //报警开关指示灯

//#define wifi_led_pin 13     //配网指示灯

/* Data point define */
#define DPID_battery_state   2
#define DPID_battery_percentage  3
#define DPID_temp_current   8
#define DPID_humidity_value 9
#define DPID_pir_state    12
#define DPID_smoke_sensor_state  13
#define DPID_smoke_sensor_value  14
#define DPID_alarm_volume   15
#define DPID_alarm_ringtone  16
#define DPID_alarm_time   17
#define DPID_preheat 20
#define DPID_alarm_switch    23
#define DPID_muffling  24
#define DPID_gas_sensor_state  25
#define DPID_gas_sensor_value  26
#define DPID_alarm_state  29
#define DPID_illuminance_value  43

/* Current device DP values */
unsigned char battery_state=1;    //电池电量状态low,middle,high(0,1,2)
unsigned char battery_percentage=0;  //电池电量0-100
unsigned char smoke_sensor_state=0; //烟雾检测状态alarm, normal
int smoke_sensor_value=0;  //烟雾检测值0-1000
unsigned char gas_sensor_state=0; //燃气检测状态alarm, normal
int gas_sensor_value=0; //燃气检测值0-1000
int light_value =0;  //光照度
unsigned char pir_state=0;  //人体感应状态0,1
int temp=0;   //温度-400-2000,间距: 1
int humidity=0;  //湿度0-100
unsigned char preheat_flag=0; //预热标志0,1
unsigned char alarm_state=2;  //报警器状态 alarm_sound, alarm_light, alarm_sound_light, normal
unsigned char alarm_volume=1;  //报警音量0,1,2,3(low,middle,high,mute)
unsigned char alarm_ringtone=1; //报警铃声0,1,2,3,4
unsigned char alarm_time=5; //报警时长0-60，间距：1，单位：s
unsigned char alarm_switch= 1;  //报警开关  0关闭，1打开 
unsigned char muffling=0; //0正常,1消音

unsigned char ringtone_key_Count = 1;   //报警铃声按键计数
unsigned long alarm_T0 = 0;    //警报打开 计时开始时间
unsigned long alarm_T1 = 0;    //警报暂停 计时开始时间
//unsigned long alarm_period = 500;  //警报1持续时间,3秒
bool alarm_open_flag = 1;     //警报打开标志,0关闭,1打开
bool alarm_light_flag=0;      //报警灯打开标志,0关闭,1打开

int count =0;
int battery_vol=0; 
int VOL_sun=0;
int average_VOL=0;

/* Stores all DPs and their types. PS: array[][0]:dpid, array[][1]:dp type. 
 *                                     dp type(TuyaDefs.h) : DP_TYPE_RAW, DP_TYPE_BOOL, DP_TYPE_VALUE, DP_TYPE_STRING, DP_TYPE_ENUM, DP_TYPE_BITMAP
*/
unsigned char dp_array[][2] = {
    {DPID_battery_state, DP_TYPE_ENUM},
    {DPID_battery_percentage, DP_TYPE_VALUE},
    {DPID_temp_current, DP_TYPE_VALUE},
    {DPID_humidity_value, DP_TYPE_VALUE},
    {DPID_pir_state, DP_TYPE_ENUM},
    {DPID_smoke_sensor_state, DP_TYPE_ENUM},
    {DPID_smoke_sensor_value, DP_TYPE_VALUE},
    {DPID_alarm_volume, DP_TYPE_ENUM},
    {DPID_alarm_ringtone, DP_TYPE_ENUM},
    {DPID_alarm_time, DP_TYPE_VALUE},
    {DPID_preheat, DP_TYPE_BOOL},
    {DPID_alarm_switch, DP_TYPE_BOOL},
    {DPID_muffling, DP_TYPE_BOOL},
    {DPID_gas_sensor_state, DP_TYPE_ENUM},
    {DPID_gas_sensor_value, DP_TYPE_VALUE},
    {DPID_alarm_state, DP_TYPE_ENUM},
    {DPID_illuminance_value, DP_TYPE_VALUE},
};

unsigned char pid[] = {"g031akxctzdbzuzo"};
unsigned char mcu_ver[] = {"1.0.0"};

unsigned long update_T0 = 0;    //上传数据计时
unsigned long period = 1000;  //数据上传间隔,5秒
/* last time */
unsigned long last_time = 0;

void setup()
{
    Serial.begin(9600);
    DebugSerial.begin(9600);

    //Initialize led port, turn off led.
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    pinMode(red_alarm_LED,OUTPUT);
    pinMode(blue_alarm_LED,OUTPUT);
    pinMode(ringtone_key_pin,INPUT_PULLUP); 
    pinMode(muffling_key_pin,INPUT_PULLUP); 
    pinMode(human_sensor_pin,INPUT); 
    pinMode(buzzer_pin,OUTPUT); 
    pinMode(wifi_key_pin, INPUT_PULLUP);    //Initialize networking keys.
    pinMode(alarm_key_pin,INPUT_PULLUP);
    pinMode(human_LED,OUTPUT); 
    pinMode(alarm_key_LED,OUTPUT); 
    
    digitalWrite(red_alarm_LED,LOW);
    digitalWrite(blue_alarm_LED,LOW);
    digitalWrite(human_LED,LOW);
    digitalWrite(alarm_key_LED,LOW);
    
    dht.begin();
    //Enter the PID and MCU software version
    my_device.init(pid, mcu_ver);
    //incoming all DPs and their types array, DP numbers
    my_device.set_dp_cmd_total(dp_array, 17);
    //register DP download processing callback function
    my_device.dp_process_func_register(dp_process);
    //register upload all DP callback function
    my_device.dp_update_all_func_register(dp_update_all);

    alarm_T0 = millis();    
    alarm_T1 = millis(); 
    update_T0 = millis();
    last_time = millis();
}

void loop()
{
    my_device.uart_service();

    //Enter the connection network mode when Pin7 is pressed.
    if (digitalRead(wifi_key_pin) == LOW) {
        delay(80);
        if (digitalRead(wifi_key_pin) == LOW) {
            my_device.mcu_set_wifi_mode(SMART_CONFIG);
        }
    }

    /* LED blinks when network is being connected */
    if ((my_device.mcu_get_wifi_work_state() != WIFI_LOW_POWER) && (my_device.mcu_get_wifi_work_state() != WIFI_CONN_CLOUD) && (my_device.mcu_get_wifi_work_state() != WIFI_SATE_UNKNOW)) {
        if (millis() - last_time >= 500) {
            last_time = millis();
            /* Toggle current LED status */
            if (led_state == LOW) {
                led_state = HIGH;
            } else {
                led_state = LOW;
            }
            digitalWrite(LED_BUILTIN, led_state);
        }
    }

    if( digitalRead(alarm_key_pin) == LOW )            //报警开关按键扫描
    {
        delay(80);          //延时去抖
        if( digitalRead(alarm_key_pin) == LOW )
        {
            alarm_switch=!(alarm_switch);
            DebugSerial.print("alarm_switch:");
            DebugSerial.println(alarm_switch);
        }
    }
    

    if( digitalRead(muffling_key_pin) == LOW )            //消音开关
    {
        delay(80);          //延时去抖
        if( digitalRead(muffling_key_pin) == LOW )
        {
            muffling=!(muffling);
            if(muffling==1)
            {
                tone(buzzer_pin, 900); 
                delay(20);   
                noTone(buzzer_pin);   
            }
            DebugSerial.print("muffling:");
            DebugSerial.println(muffling);
        }
    }
    if( digitalRead(ringtone_key_pin) == LOW )            //报警铃声选择
    {
        delay(80);          //延时去抖
        if( digitalRead(ringtone_key_pin) == LOW )
        {
            ringtone_key_Count++; 
            if (ringtone_key_Count==5)
            {
                ringtone_key_Count=0;
            }
            //tone(buzzer_pin, 556,20);
            alarm_ringtone=ringtone_key_Count; 
            DebugSerial.print("alarm_ringtone:");
            DebugSerial.println(alarm_ringtone);       
        }
    }

    //电量采集
    battery_vol=analogRead(battery_voltage_pin); 
    VOL_sun+=battery_vol;
    count ++;
    if (count>=20)
    {
      count=0;
      average_VOL=VOL_sun/20;
      VOL_sun=0;
    }
    
    //DebugSerial.print("battery_vol:");
    //DebugSerial.println(battery_vol);  
    if (average_VOL>520)
    {
      average_VOL=520;
    }
    else if(average_VOL<450)
    {
      average_VOL=450;
    }
    battery_percentage= map(average_VOL, 450, 520, 0, 100);
    //DebugSerial.print("battery_percentage:");
    //DebugSerial.println(battery_percentage);    
    if(battery_percentage>=80)
    {
      battery_state=2;
    }
    else if(battery_percentage>=30 && battery_percentage<80)
    {
      battery_state=1;
    }
    else if(battery_percentage<30)
    {
      battery_state=0;
    }

      //人体感应状态0,1
    pir_state=!(digitalRead(human_sensor_pin));  //0有人,1无人
    //DebugSerial.print("pir_state:");
    //DebugSerial.println(pir_state);  

    

  
    //光照度
    light_value =1023-analogRead(light_sensor_pin); 
    //DebugSerial.print("light_value:");
    //DebugSerial.println(light_value); 
    if(pir_state==0 && light_value<200)
    {
          digitalWrite(human_LED,HIGH);
    }
    else if(pir_state==1)
    {
          digitalWrite(human_LED,LOW);
    }

    if ((millis()- update_T0) >= period) {
        update_T0= millis();

         //温湿度
        humidity =int(dht.readHumidity());
        // Read temperature as Celsius (the default)
        temp = int(dht.readTemperature()*10);

        //烟雾检测
        smoke_sensor_value=analogRead(smoke_sensor_pin);
        //DebugSerial.print("smoke_sensor_value:");
        //DebugSerial.println(smoke_sensor_value);  
        if(smoke_sensor_value>60)
        {
          smoke_sensor_state=0; //烟雾检测状态alarm
        }
        else
        {
          smoke_sensor_state=1; //烟雾检测状态normal
        }
        
        //燃气检测
        gas_sensor_value=analogRead(gas_sensor_pin); 
        //DebugSerial.print("gas_sensor_value:");
        //DebugSerial.println(gas_sensor_value); 
        //DebugSerial.println(""); 
        if(gas_sensor_value>(10+alarm_volume*10))
        {
          gas_sensor_state=1; //燃气检测状态alarm
        }
        else
        {
          gas_sensor_state=0; //燃气检测状态normal
        }
        
        my_device.mcu_dp_update(DPID_battery_state, battery_state, 1);
        my_device.mcu_dp_update(DPID_battery_percentage, battery_percentage, 1);
        my_device.mcu_dp_update(DPID_temp_current, temp, 1);
        my_device.mcu_dp_update(DPID_humidity_value, humidity, 1);
        my_device.mcu_dp_update(DPID_pir_state, pir_state, 1);
        my_device.mcu_dp_update(DPID_smoke_sensor_state, smoke_sensor_state, 1);
        my_device.mcu_dp_update(DPID_smoke_sensor_value, smoke_sensor_value, 1);
        my_device.mcu_dp_update(DPID_alarm_volume, alarm_volume, 1);
        my_device.mcu_dp_update(DPID_alarm_ringtone, alarm_ringtone, 1);
        my_device.mcu_dp_update(DPID_alarm_time, alarm_time, 1);
        my_device.mcu_dp_update(DPID_preheat, preheat_flag, 1);
        my_device.mcu_dp_update(DPID_alarm_switch, alarm_switch, 1); 
        my_device.mcu_dp_update(DPID_muffling,  muffling, 1);
        my_device.mcu_dp_update(DPID_gas_sensor_state, gas_sensor_state, 1);
        my_device.mcu_dp_update(DPID_gas_sensor_value, gas_sensor_value, 1);
        my_device.mcu_dp_update(DPID_alarm_state,alarm_state, 1);
        my_device.mcu_dp_update(DPID_illuminance_value, light_value, 1);

        if (alarm_light_flag==1) 
        {
            alarm_light();
        }
        else if (alarm_light_flag == 0)
        {
            digitalWrite(red_alarm_LED,LOW);
            digitalWrite(blue_alarm_LED,LOW);
        }
    }

    if (alarm_switch==1) 
    {
        digitalWrite(alarm_key_LED,HIGH);
        if ((smoke_sensor_state==0 || gas_sensor_state==1 || temp >=400) ) 
        {
            alarm_light_flag=1;
            if ((millis()-alarm_T0)>=(alarm_time*1000))
            {
                alarm_T0=millis(); 
                //digitalWrite(alarm_LED,LOW);
                alarm_open_flag = !alarm_open_flag;
            }
            if (alarm_open_flag == 1)
            {
                //DebugSerial.println("Alarm_ON");
                Alarm_ON(); 
            }

        }
        else{
            alarm_light_flag=0;
        }
    }
    else if(alarm_switch==0) 
    {
        alarm_light_flag=0;
        digitalWrite(alarm_key_LED,LOW);
        digitalWrite(red_alarm_LED,LOW);
        digitalWrite(blue_alarm_LED,LOW);
    }
}

/**
 * @description: DP download callback function.
 * @param {unsigned char} dpid
 * @param {const unsigned char} value
 * @param {unsigned short} length
 * @return {unsigned char}
 */
unsigned char dp_process(unsigned char dpid, const unsigned char value[], unsigned short length)
{
    DebugSerial.print("dpid");
    DebugSerial.println(dpid);
    switch (dpid) {
        case DPID_alarm_volume:  //alarm_volume 报警音量
            DebugSerial.print("DPID_alarm_volume:");
            alarm_volume = my_device.mcu_get_dp_download_data(dpid, value, length);
            DebugSerial.println(alarm_volume);
            /* After processing the download DP command, the current status should be reported. */
            my_device.mcu_dp_update(DPID_alarm_volume, alarm_volume, 1);
        break;

        case DPID_alarm_ringtone:  //alarm_ringtone 报警铃声
            DebugSerial.print("DPID_alarm_ringtone:");
            alarm_ringtone = my_device.mcu_get_dp_download_data(dpid, value, length);
            DebugSerial.println(alarm_ringtone);
            /* After processing the download DP command, the current status should be reported. */
            my_device.mcu_dp_update(DPID_alarm_ringtone, alarm_ringtone, 1);
        break;

        case DPID_alarm_time:  //alarm_time 报警时长
            DebugSerial.print("DPID_alarm_time:");
            alarm_time = my_device.mcu_get_dp_download_data(dpid, value, length);
            DebugSerial.println(alarm_time);
            /* After processing the download DP command, the current status should be reported. */
            my_device.mcu_dp_update(DPID_alarm_time, alarm_time, 1);
        break;

        case DPID_alarm_switch:   //alarm_switch  报警开关
            DebugSerial.print("DPID_alarm_switch:");
            alarm_switch = my_device.mcu_get_dp_download_data(dpid, value, length);
            DebugSerial.println(alarm_switch);
            /* After processing the download DP command, the current status should be reported. */
            my_device.mcu_dp_update(DPID_alarm_switch, alarm_switch,1);
        break;

        case DPID_muffling:  //muffling 消音
            DebugSerial.println("DPID_muffling:");
            muffling = my_device.mcu_get_dp_download_data(dpid, value, length);
            if(muffling==1)
            {
                alarm_state=1;
            }
            else
            {
                alarm_state=2;
            }
            DebugSerial.println(muffling);
            /* After processing the download DP command, the current status should be reported. */
            my_device.mcu_dp_update(DPID_muffling, muffling, 1);
        break;
        

        default:
            break;
    }
    
    return SUCCESS;
}

/**
 * @description: Upload all DP status of the current device.
 * @param {*}
 * @return {*}
 */
void dp_update_all(void)
{
    my_device.mcu_dp_update(DPID_battery_state, battery_state, 1);
    my_device.mcu_dp_update(DPID_battery_percentage, battery_percentage, 1);
    my_device.mcu_dp_update(DPID_temp_current, temp, 1);
    my_device.mcu_dp_update(DPID_humidity_value, humidity, 1);
    my_device.mcu_dp_update(DPID_pir_state, pir_state, 1);
    my_device.mcu_dp_update(DPID_smoke_sensor_state, smoke_sensor_state, 1);
    my_device.mcu_dp_update(DPID_smoke_sensor_value, smoke_sensor_value, 1);
    my_device.mcu_dp_update(DPID_alarm_volume, alarm_volume, 1);
    my_device.mcu_dp_update(DPID_alarm_ringtone, alarm_ringtone, 1);
    my_device.mcu_dp_update(DPID_alarm_time, alarm_time, 1);
    my_device.mcu_dp_update(DPID_preheat, preheat_flag, 1);
    my_device.mcu_dp_update(DPID_alarm_switch, alarm_switch, 1); 
    my_device.mcu_dp_update(DPID_muffling,  muffling, 1);
    my_device.mcu_dp_update(DPID_gas_sensor_state, gas_sensor_state, 1);
    my_device.mcu_dp_update(DPID_gas_sensor_value, gas_sensor_value, 1);
    my_device.mcu_dp_update(DPID_alarm_state,alarm_state, 1);
    my_device.mcu_dp_update(DPID_illuminance_value, light_value, 1);
}

void Alarm_ON() 
{
    if(muffling==1)
    {
        //alarm_light();
        alarm_state=1;
    }
    else
    {
        //alarm_light();
        alarm_sound();
        alarm_state=2;
    }
}



void alarm_sound()
{
    switch(alarm_ringtone)
    {
       case 0:
         alarm_sound_1();
         break;
         
       case 1:
         alarm_sound_2();
         break;
       
       case 2:
         alarm_sound_3();
         break; 
       
       case 3:
         alarm_sound_4();
         break;
       
       case 4:
         alarm_sound_5();
         break;
       
       default:
         break;
     }
}


void alarm_sound_1()
{
    for (int x = 0; x<180; x++) 
    {
      //当使用sin函数时，角度转换成弧度，用sin函数值产生声音频率
      int toneVal = 1000+ (int((sin(x*(3.1412 / 180))) * 1000));
      tone(buzzer_pin, toneVal);
      delay(1);
    }
    noTone(buzzer_pin);
}

void alarm_sound_2()
{
    for (int x = 0; x<90; x++) 
    {
      int toneVal = 1000+ (int((sin(x*(3.1412 / 180))) * 1000));
      tone(buzzer_pin, toneVal);
      delay(1);
    }
    noTone(buzzer_pin);
}

void alarm_sound_3()
{
    for (int x = 0; x<90; x++) 
    {
      int toneVal = 800+ (int((sin(x*(3.1412 / 180))) * 1000));
      tone(buzzer_pin, toneVal);
      delay(1);
    }
    noTone(buzzer_pin);
}

void alarm_sound_4()
{
    for (int x = 0; x<90; x++) 
    {
      int toneVal = 600+ (int((sin(x*(3.1412 / 180))) * 1000));
      tone(buzzer_pin, toneVal);
      delay(1);
    }
    noTone(buzzer_pin);
}

void alarm_sound_5()
{
    digitalWrite(buzzer_pin,HIGH);
    delay(50);
    digitalWrite(buzzer_pin,LOW);
/* 
    tone(buzzer_pin, 294);
    delay(100);
    noTone(buzzer_pin);
    tone(buzzer_pin, 556);
    delay(10);
    noTone(buzzer_pin);
    */
}

void alarm_light()
{
    digitalWrite(red_alarm_LED,!(digitalRead(red_alarm_LED)));
    digitalWrite(blue_alarm_LED,!(digitalRead(red_alarm_LED)));
}
