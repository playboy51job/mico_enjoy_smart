#include "mico.h"
#include "fogcloud_up_thread.h"
#include "fog_v2_include.h"
#include "json_c/json_object.h"
#include "json_c/json.h"
#include "micokit_ext.h"
#include "sensor/DHT11/DHT11.h"


#define fogcloud_up_log(M, ...) custom_log("FOG_UP_DATA", M, ##__VA_ARGS__)
//����ԭ��
//�����������ݶ�ȡ����ת��Ϊjson�ַ���
char *buildSensorJson();
//���������߳�
void user_upstream_thread(mico_thread_arg_t args);


//�����ʼ��
static OSStatus ext_module_init(void)
{
    OSStatus err = kNoErr;

    //��ʼ���ⲿ������
    err = DHT11_Init();             //��ʪ�ȴ�������ʼ��
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: Unable to Init DHT11") );

    //�����ʼ��
    err = infrared_reflective_init();
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: Unable to Init infrared refective") );

    /*init Light sensor*/
    err = light_sensor_init();
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: Unable to Init light sensor") );

    //��ʼ�����
    err = dc_motor_init();
    require_noerr_action( err, exit, fogcloud_up_log("ERROR: Unable to Init DC motor") );
    dc_motor_set(0);

    //��ʼ��rgb_led
    hsb2rgb_led_init();

    fogcloud_up_log("module init success");

 exit:
    return err;
}



void user_upstream_thread(mico_thread_arg_t args)
{
	OSStatus err = kUnknownErr;

	fogcloud_up_log("------------upstream_thread start------------");

    err = ext_module_init();      //�����ʼ��
    require_noerr(err, exit);

	while(1)
	{
	    //ÿ��һ��ʱ��Ͷ�ȡ�����ϴ����ƶˣ��ߵ���httpsЭ��
        mico_thread_msleep(3000);
        if(fog_v2_is_have_superuser() == true)
        {
            char *upload_data = NULL;

            upload_data = buildSensorJson();
            if(upload_data ==NULL) continue;

            //��json�ַ����ϴ�
            fog_v2_device_send_event(upload_data, FOG_V2_SEND_EVENT_RULES_PUBLISH | FOG_V2_SEND_EVENT_RULES_DATEBASE);
            fogcloud_up_log("data upload...%s",upload_data);
            free(upload_data);
            upload_data = NULL;
        }
	}

 exit:
	mico_rtos_delete_thread(NULL);  // delete current thread

	return;
}

//���������ݣ��û������޸Ĵ˴��Ĵ��룬�����Լ��Ĵ��������õ���json��ʽ���ݽṹ
char *buildSensorJson()
{
  OSStatus    err = kNoErr;
  uint8_t     temperature;     //�¶�
  uint8_t     humidity;       //ʪ��
  uint16_t    infrared_reflective;  //����
  uint16_t    light_sensor;   //����ǿ��
  char oled_show_line[OLED_DISPLAY_MAX_CHAR_PER_ROW+1]={0};//��ʾOLED��
  /*control info*/
  char *p= (char*)malloc(1024);

  /*1:construct json object*/
  struct json_object *recv_json_object=NULL;
  recv_json_object=json_object_new_object();

  struct json_object *device_object=NULL;
  device_object=json_object_new_object();
  //first,read sensor data
  //��ȡ��ʪ��
  err = DHT11_Read_Data(&temperature, &humidity);
  require_noerr_action( err, exit, fogcloud_up_log("ERROR: Can't Read Data") );
  //fogcloud_up_log("DHT11  T: %3.1fC  H: %3.1f%%", (float)ext_sensor_data.temperature, (float)ext_sensor_data.humidity);

  err = infrared_reflective_read(&infrared_reflective);
  require_noerr_action( err, exit, fogcloud_up_log("ERROR: Can't infrared refectiver read data") );
  //fogcloud_up_log("infrared reflective date: %d", ext_sensor_data.infrared_reflective);

  err = light_sensor_read(&light_sensor);
  require_noerr_action( err, exit, fogcloud_up_log("ERROR: Can't light sensor read data") );
  //fogcloud_up_log("light date: %d", ext_sensor_data.light_sensor);


  json_object_object_add(device_object, "temperature", json_object_new_int(temperature));
  json_object_object_add(device_object, "humidity", json_object_new_int(humidity));
  json_object_object_add(device_object, "infrared_reflective", json_object_new_int(infrared_reflective));
  json_object_object_add(device_object, "light_sensor", json_object_new_int(light_sensor));

  json_object_object_add(recv_json_object,"device_info",device_object);/*one pair K-V*/
  strcpy(p,json_object_to_json_string(recv_json_object));
  /*3:parse finished,free memory*/
  json_object_put(recv_json_object);/*free memory*/
  recv_json_object=NULL;



  //��������ʾ��OLED��Ļ��ȥ
  memset(oled_show_line, 0, OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  sprintf(oled_show_line,"%s","Fog V2          ");
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_1, (char *)oled_show_line);

  //������ʾ�������ѿ��ƣ�����ս����ٸ�ֵ����ʪ��
  memset(oled_show_line, 0, OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  sprintf(oled_show_line,"%s","                ");
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_2, (char *)oled_show_line);
  memset(oled_show_line, 0, OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  sprintf(oled_show_line,"Humi:%d%%Temp:%dC",humidity,temperature);
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_2, (char *)oled_show_line);

  //��ǿ����λflux
  memset(oled_show_line, 0, OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  sprintf(oled_show_line,"%s","                ");
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_3, (char *)oled_show_line);
  memset(oled_show_line, 0, OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  sprintf(oled_show_line,"Light:%dflux", light_sensor);
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_3, (char *)oled_show_line);

  //������룬��λmm
  memset(oled_show_line, 0, OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  sprintf(oled_show_line,"%s","                ");
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, (char *)oled_show_line);
  memset(oled_show_line, 0, OLED_DISPLAY_MAX_CHAR_PER_ROW+1);
  sprintf(oled_show_line,"IR:%dmm", infrared_reflective);
  OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, (char *)oled_show_line);
  return p;
exit:
  return NULL;
}

